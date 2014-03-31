/*
 *  File:				main.cpp
 *  Created by:			Luyuan Lin
 *  Created:			March 2014
 *  Last Modified:		March 2014
 *  Last Modified by:	Luyuan Lin
 * 
 *  Defined the main function for the whole EHwA system.
 */

#include <iostream>
#include <string>
#include <utility>
#include "bayesact_client.hpp"
#include "EPACalculator/epa_calculator.hpp"
#include "frame_analyzer.hpp"
#include "prompt_selecter.hpp"

using std::cout;
using std::endl;
using std::string;
using std::make_pair;

using namespace EHwA;

void StartServer(string addr) {
    string cmd = "gnome-terminal -e 'python ./bayesact_server_stub.py " + addr + "'";
    system(cmd.c_str());
}
	
void StartClient(string addr, string output_mapping_filename) {
    // Connect the BayesActClient client & server
    BayesactClient client(addr);
    // define and initianlize pipeline variables
    // KinGrabber kinGrabber;
    FrameAnalyzer frame_analyzer;
    EPACalculator epa_calculator;
    PromptSelecter prompt_selecter(output_mapping_filename);
    vector<pair<Position, Position> > hand_positions;
    while (true) {
	  //------------- Grab an image & pack a Frame for process ----------------
      // Frame frame = kinGrabber.Capture();
      //------------- Analyze the frame & update corresponding variables --------
      frame_analyzer.Analyze();
      Position left_hand_pos = frame_analyzer.get_left_hand_position();
      Position right_hand_pos = frame_analyzer.get_right_hand_position();
      hand_positions.push_back(
        make_pair<Position, Position> (left_hand_pos, right_hand_pos));
      //---------------- Calculate EPA values -------------------------
      epa_calculator.Calculate(hand_positions);
      //-------- Send currentEPA & handAction to server -------
      client.Send(epa_calculator.get_current_epa(),
                  frame_analyzer.get_hand_action());
      //----------- Get response from server --------------
      client.Receive();
      vector<double> response_epa = client.get_response_epa();
      int response_prompt = client.get_response_prompt();
      //----------- Select proper prompt ---------------------
      int id = prompt_selecter.Select(response_epa, response_prompt);
      cout << "Proper prompt is #" << id << endl;
      //----------- Play prompt with PromptPlayer (a plug-in)
     }
 }
	


int main() {
  cout << "Hello World for Emotional Handwashing Assistant (EHwA)!"
       << endl;
  // Constant values used in the program
  string serverAddr = "tcp://*:5555";
  string clientAddr = "tcp://localhost:5555";
  string outputMappingFilename = "";

  StartServer(serverAddr);
  StartClient(clientAddr, outputMappingFilename);

  return 0;
}
