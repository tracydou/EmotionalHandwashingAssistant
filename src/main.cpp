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
#include "BayesActClient.hpp"
#include "EPACalculator/EPACalculator.hpp"
#include "FrameAnalyzer.hpp"
#include "PromptSelecter.hpp"

using std::cout;
using std::endl;
using std::string;
using std::make_pair;

using namespace EHwA;

int main() {
  cout << "Hello World for Emotional Handwashing Assistant (EHwA)!"
       << endl;
  // Constant values used in the program
  string addr = "tcp://*:5555"; // shared by server & client
  string outputMappingFilename = "";

  int pid=fork();
  if (pid < 0 ) { // failed to fork
    cout << "Failed to fork child process!" << endl;  
    return -1;
  } else if (pid != 0) { // Child process
    // change directory and start the BayesactEngine server
    // TODO: implement server stub using zmq
    chdir("../lib/BayesactEngine");
    system("python ./bayesactinteractive.py");
  } else { // Parent process
    // Connect the BayesActClient client & server
    BayesActClient client(addr);
    // define and initianlize pipeline variables
    // KinGrabber kinGrabber;
    FrameAnalyzer frameAnalyzer;
    EPACalculator epaCalculator;
    PromptSelecter promptSelecter(outputMappingFilename);
    vector<pair<Position, Position> > handPositions;
    while (true) {
	  //------------- Grab an image & pack a Frame for process ----------------
      // Frame frame = kinGrabber.Capture();
      //------------- Analyze the frame & update corresponding variables --------
      frameAnalyzer.Analyze();
      Position leftHandPos = frameAnalyzer.getLeftHandPos();
      Position rightHandPos = frameAnalyzer.getRightHandPos();
      handPositions.push_back(
        make_pair<Position, Position> (leftHandPos, rightHandPos));
      //---------------- Calculate EPA values -------------------------
      epaCalculator.Calculate(handPositions);
      //-------- Send currentEPA & handAction to server -------
      client.Send(epaCalculator.getCurrentEPA(),
                  frameAnalyzer.getHandAction());
      //----------- Get response from server --------------
      client.Receive();
      vector<double> respondedEPA = client.getRespondedEPA();
      int respondedPrompt = client.getRespondedPrompt();
      //----------- Select proper prompt ---------------------
      int id = promptSelecter.Select(respondedEPA, respondedPrompt);
      cout << "Proper prompt is #" << id << endl;
      //----------- Play prompt with PromptPlayer (a plug-in)
     }
  }
  return 0;
}
