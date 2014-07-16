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
#include <thread> // multi-threading for prompt-player

#include "bayesact_client.hpp"
#include "buffer.hpp"
#include "defines.hpp"
#include "prompt_player.hpp"
#include "prompt_selecter.hpp"
#include "tracker_client.hpp"

// to get current time
#include <time.h>

// write to file
#include <iostream>
#include <fstream>
using std::ofstream;
using std::ios;

using std::cout;
using std::endl;
using std::list;
using std::string;

using std::thread; // multi-threading for prompt-player

using namespace EHwA;

void StartBayesactServer(string addr) {
    string cmd = "gnome-terminal -e 'python ./bayesact_server_stub.py " + addr + "'";
    system(cmd.c_str());
}

void StartHandtrackerServer(string addr, string vidpath) {
    string cmd = "gnome-terminal -e \
      '../lib/hand_tracker/build/HAND_TRACKER -projectpath ../lib/hand_tracker -vidpath " +vidpath + " "+ addr + "'";
    system(cmd.c_str());
}
	
void StartClient(string bayesact_addr, string hand_tracker_addr,
                 string output_mapping_filename,string prompt_foldername, 
                 string default_prompt_filename) {
                     
    // Used to collect latency of system
    //ofstream outfile;
    //outfile.open("runspeed.txt", ios::out|ios::app);
  
    // Start BayesActClient & TrackerClient clients
    BayesactClient bayesact_client(bayesact_addr);
    TrackerClient tracker_client(hand_tracker_addr);
    cout << "=============== Clients have been set up! =============" << endl << endl;
    // Define and initialize pipeline variables
    vector<double> default_epa(3);
    default_epa[0] = 0.0;// E stays as 0 throughout the whole process 
    default_epa[1] = 0;// initialize PA=[1.2, -1.2] for "elder" or [0,-2] for "lonesome elder"
    default_epa[2] = -2; // i.e. 
    Buffer buffer(300,1,default_epa);
    PromptSelecter prompt_selecter(output_mapping_filename, default_prompt_filename);
    PromptPlayer prompt_player;
    bool is_done = false;
    cout << "=============== Initialization ready ! =============" << endl << endl;
    while (not is_done) {
    
        //struct timespec start, middle1, middle2, end;
        //clock_gettime(CLOCK_MONOTONIC, &start);	/* mark start time */
          
        cout << endl << "=============== Start of a new iteration: =============" << endl;
	    //------------- Get hand-pos info from HandTracker -------
        Position left_hand_pos, right_hand_pos;
        int current_action = BAYESACT_NOTHING;
        if (!tracker_client.GetHandPositionAndAction(
            left_hand_pos, right_hand_pos, current_action)) {
          continue;
        }
      
    
        //clock_gettime(CLOCK_MONOTONIC, &middle1);/* mark middle time */

        //---------------- Updater buffer state -------------------------
        buffer.Update(current_action, left_hand_pos, right_hand_pos);
        if(buffer.get_current_state() != Buffer::STATE_C) {
          continue;
        }
        
        //clock_gettime(CLOCK_MONOTONIC, &middle2);/* mark middle time */
        
        //-------- Send currentEPA & handAction to server -------
        current_action = buffer.get_current_user_behaviour();
        bayesact_client.Send(buffer.get_current_epa(), current_action);
        buffer.ChangeToState(Buffer::STATE_A, current_action);
        //----------- Get response from server --------------
        bayesact_client.Receive();
        vector<double> response_epa = bayesact_client.get_response_epa();
        int response_prompt = bayesact_client.get_response_prompt();
        is_done = bayesact_client.is_done();
        //----------- Select proper prompt & Play prompt with PromptPlayer (a plug-in) ---------------------
        string prompt_filename = prompt_foldername + prompt_selecter.Select(response_epa, response_prompt);
        if (prompt_filename == "") {
          cout << "No Prompt is needed at this time;" << endl;
        } else {		  
          cout << "Proper prompt_filename is " << prompt_filename << endl;
          if (prompt_player.TryLock()) {
			thread thread_play_video(&PromptPlayer::Play, &prompt_player, prompt_filename);
			thread_play_video.detach();
		  }
        }

        //clock_gettime(CLOCK_MONOTONIC, &end);/* mark end time */
        //uint64_t diff1, diff2, diff3;
        //uint64_t BILLION = 1000000000;
        //diff1 = BILLION * (middle1.tv_sec - start.tv_sec) + middle1.tv_nsec - start.tv_nsec;
        //diff2 = BILLION * (middle2.tv_sec - middle1.tv_sec) + middle2.tv_nsec - middle1.tv_nsec;
        //diff3 = BILLION * (end.tv_sec - middle2.tv_sec) + end.tv_nsec - middle2.tv_nsec;
        //outfile << "elapsed time = nanoseconds. diff1 = "<<diff1<<", diff2 = "<<diff2<<", diff3 = "<<diff3<<endl;
        }
    //outfile.close();
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
  string default_vid_path = "/home/l39lin/TracyThesis/EmotionalHandwashingAssistant" \
                        "/lib/hand_tracker/videos/slow2.oni";
                        
  StartBayesactServer(bayesactServerAddr);
  StartHandtrackerServer(trackerServerAddr,default_vid_path);
  StartClient(bayesactClientAddr, trackerClientAddr, output_mapping_filename,
              prompt_foldername, default_prompt_filename);
              

  return 0;
}
