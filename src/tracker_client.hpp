/*
 *  File:				tracker_client.hpp
 *  Created by:			Luyuan Lin
 *  
 */

#ifndef TRACKER_CLIENT_
#define TRACKER_CLIENT_

#include <string>
#include <vector>
#include <../lib/cppzmq/zmq.hpp>
#include "../lib/hand_tracker/hand_tracker_message.pb.h"
#include "defines.hpp"

using std::string;
using std::vector;

namespace EHwA {

class TrackerClient {
  public:
    TrackerClient(string handtracker_addr);
    ~TrackerClient();
  
    // Request for hand-positions and current action. Call HandTracker funcs.
    bool GetHandPositionAndAction(Position& left_hand_pos, 
                                  Position& right_hand_pos, int& action);
    
  protected:
    bool SendHandTrackerRequest(const HandTrackerRequest& request);
    bool ReceiveHandTrackerResponse(HandTrackerResponse& response);
    
    zmq::context_t handtracker_context_;
    zmq::socket_t handtracker_socket_;
};

}  // namespace EHwA

#endif  // TRACKER_CLIENT_
