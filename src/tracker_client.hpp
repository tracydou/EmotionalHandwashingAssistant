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
  
    // request for left-hand-pos if left_hand == true; otherwise,
    // request for right-hand-pos. Call HandTracker funcs.
    bool GetHandPosition(bool left_hand, Position& pos);
    // request for current hand action, call HandTracker funcs.
    bool GetHandAction(int& action);
 
  protected:
    bool SendHandTrackerRequest(const HandTrackerRequest& request);
    bool ReceiveHandPos(HandTrackerResponseHandPos& response);
    bool ReceiveAction(HandTrackerResponseAction& response);
    
    zmq::context_t handtracker_context_;
    zmq::socket_t handtracker_socket_;
};

}  // namespace EHwA

#endif  // TRACKER_CLIENT_
