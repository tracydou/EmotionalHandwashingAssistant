/*
 *  File:				main.cpp
 *  Created by:			Luyuan Lin
 * 
 *  Defined the main function for the whole EHwA system.
 */

#include <iostream>
#include <string>
#include <utility>
#include "bayesact_client.hpp"
#include "defines.hpp"
#include "EPACalculator/epa_calculator.hpp"
#include "prompt_player.hpp"
#include "prompt_selecter.hpp"
#include "tracker_client.hpp"

using std::cout;
using std::endl;
using std::string;
using std::make_pair;

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
                 string output_mapping_filename) {
    // Start BayesActClient & TrackerClient clients
    BayesactClient bayesact_client(bayesact_addr);
    TrackerClient tracker_client(hand_tracker_addr);
    // Define and initianlize pipeline variables
    EPACalculator epa_calculator;
    PromptSelecter prompt_selecter(output_mapping_filename);
    vector<pair<Position, Position> > hand_positions;
    int current_action = UNKNOWN_ACTION;
    PromptPlayer prompt_player;
    while (true) {
	  //------------- Get hand-pos info from HandTracker -------
      Position left_hand_pos, right_hand_pos;
      if (tracker_client.GetHandPositionAndAction(
            left_hand_pos, right_hand_pos, current_action)) {
          hand_positions.push_back(
            make_pair<Position, Position> (left_hand_pos, right_hand_pos));
      } else {
          continue;
      }
      //---------------- Calculate EPA values -------------------------
      epa_calculator.Calculate(hand_positions);
      //-------- Send currentEPA & handAction to server -------
      bayesact_client.Send(epa_calculator.get_current_epa(), current_action);
      //----------- Get response from server --------------
      bayesact_client.Receive();
      vector<double> response_epa = bayesact_client.get_response_epa();
      int response_prompt = bayesact_client.get_response_prompt();
      //----------- Select proper prompt ---------------------
      string prompt_filename = "../data/" + prompt_selecter.Select(response_epa, response_prompt);
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
  string outputMappingFilename = "";

  StartBayesactServer(bayesactServerAddr);
  StartHandtrackerServer(trackerServerAddr);
  StartClient(bayesactClientAddr, trackerClientAddr, outputMappingFilename);

  return 0;
}
