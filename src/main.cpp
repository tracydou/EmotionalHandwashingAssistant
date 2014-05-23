/*
 *  File:				main.cpp
 *  Created by:			Luyuan Lin
 * 
 *  Defined the main function for the whole EHwA system.
 */

#include <iostream>
#include <string>
#include <utility>
#include <list>

#include "bayesact_client.hpp"
#include "defines.hpp"
#include "EPACalculator/epa_calculator.hpp"
#include "prompt_player.hpp"
#include "prompt_selecter.hpp"
#include "tracker_client.hpp"

using std::cout;
using std::endl;
using std::list;
using std::string;

using namespace EHwA;

void StartBayesactServer(string addr) {
    string cmd = "gnome-terminal -e 'python ./bayesact_server_stub.py " + addr + "'";
    system(cmd.c_str());
}

void StartHandtrackerServer(string addr) {
    string cmd = "gnome-terminal -e \
      '../lib/hand_tracker/build/HAND_TRACKER -projectpath ../lib/hand_tracker " + addr + "'";
    system(cmd.c_str());
}
	
void StartClient(string bayesact_addr, string hand_tracker_addr,
                 string output_mapping_filename,string prompt_foldername, 
                 string default_prompt_filename) {
    // Start BayesActClient & TrackerClient clients
    BayesactClient bayesact_client(bayesact_addr);
    TrackerClient tracker_client(hand_tracker_addr);
    cout << "=============== Clients have been set up! =============" << endl << endl;
    // Define and initialize pipeline variables
    EPACalculator epa_calculator;
    PromptSelecter prompt_selecter(output_mapping_filename, default_prompt_filename);
    list<pair<Position, Position> > hand_positions;
    int current_action = UNKNOWN_ACTION;
    PromptPlayer prompt_player;
    bool is_done = false;
    cout << "=============== Initialization ready ! =============" << endl << endl;
    while (not is_done) {
      cout << endl << "=============== Start of a new iteration: =============" << endl;
	    //------------- Get hand-pos info from HandTracker -------
      Position left_hand_pos, right_hand_pos;
      if (tracker_client.GetHandPositionAndAction(
            left_hand_pos, right_hand_pos, current_action)) {
          hand_positions.push_front(
            pair<Position, Position> (left_hand_pos, right_hand_pos));
      } else {
          continue;
      }
      //---------------- Calculate EPA values -------------------------
      // NUM_POSITIONS_NEEDED, defined in defines.hpp, is the number of 
      // handpositions needed by epa-calc to compute epa values
      epa_calculator.Calculate(hand_positions);
      if (hand_positions.size() > NUM_POSITIONS_NEEDED) {
          hand_positions.pop_back();
      }
      //-------- Send currentEPA & handAction to server -------
      bayesact_client.Send(epa_calculator.get_current_epa(), current_action);
      //----------- Get response from server --------------
      bayesact_client.Receive();
      vector<double> response_epa = bayesact_client.get_response_epa();
      int response_prompt = bayesact_client.get_response_prompt();
      is_done = bayesact_client.is_done();
      //----------- Select proper prompt ---------------------
      string prompt_filename = prompt_foldername + prompt_selecter.Select(response_epa, response_prompt);
      cout << "Proper prompt_filename is " << prompt_filename << endl;
      //----------- Play prompt with PromptPlayer (a plug-in)
      prompt_player.Play(prompt_filename);
     }
 }
	
int main() {
  cout << "Hello World for Emotional Handwashing Assistant (EHwA)!"
       << endl;
  // Constant values used in the program
  string bayesactServerAddr = "tcp://*:5555";
  string bayesactClientAddr = "tcp://localhost:5555";
  string trackerServerAddr = "tcp://*:5556";
  string trackerClientAddr = "tcp://localhost:5556";
  string output_mapping_filename = "../data/OutputMappingResult.csv";
  string prompt_foldername = "../data/video_prompts/";
  string default_prompt_filename = "default_prompt.mp4";

  StartBayesactServer(bayesactServerAddr);
  StartHandtrackerServer(trackerServerAddr);
  StartClient(bayesactClientAddr, trackerClientAddr, output_mapping_filename,
              prompt_foldername, default_prompt_filename);

  return 0;
}
