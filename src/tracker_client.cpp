/*
 *  File:				tracker_client.cpp
 *  Created by:			Luyuan Lin
 *  
 */

#include "tracker_client.hpp"
#include <iostream>
using std::cout;
using std::endl;

namespace EHwA {
	
TrackerClient::TrackerClient(string handtracker_addr):
  handtracker_context_(1),
  handtracker_socket_(handtracker_context_, ZMQ_REQ) {
  // set up connection to server
  cout << "Connecting to handtracker server..." << endl;
  handtracker_socket_.connect(handtracker_addr.c_str());
}

TrackerClient::~TrackerClient() {
  // Optional:  Delete all global objects allocated by libprotobuf.
  google::protobuf::ShutdownProtobufLibrary();
}

bool TrackerClient::GetHandPositionAndAction(Position& left_hand_pos,
  Position& right_hand_pos, int& action) {
  // Construct request message
  HandTrackerRequest request;
  // Send out request and wait for response
  SendHandTrackerRequest(request);
  // Decode received message
  HandTrackerResponse response;
  if (!ReceiveHandTrackerResponse(response)) {
	  cout << "Failed in getting hand positions and current action!" << endl;
	  return false;
  } else {
	  left_hand_pos.x = response.left_hand_position().x();
	  left_hand_pos.y = response.left_hand_position().y();
	  left_hand_pos.z = response.left_hand_position().z();
	  right_hand_pos.x = response.right_hand_position().x();
	  right_hand_pos.y = response.right_hand_position().y();
	  right_hand_pos.z = response.right_hand_position().z();
	  action = convert_tracker_action_to_bayesact_behaviour(response.action());
	  return true;
  }
}

bool TrackerClient::SendHandTrackerRequest(
  const HandTrackerRequest& request) {
  // convert to HandTrackerRequest message sent to server
  string message;
  // send out message if suceeded
  if (!request.SerializeToString(&message)) {
    cout << "Message.SerializeToString(&message) Failed!" << endl;
    return false;
  } else {
    handtracker_socket_.send(message.c_str(), message.length());
    cout << "[Log Info] TrackerClient: Sent Request Message." << endl
         << "Waiting for TrackerServer's response..." << endl;
    return true;
  }
}

bool TrackerClient::ReceiveHandTrackerResponse(HandTrackerResponse& response) {
  zmq::message_t message;
  handtracker_socket_.recv(&message);
  if (!response.ParseFromString(
    string((const char *)message.data(), message.size()))) {
    cout << "Message.ParseFromString(message) Failed!" << endl;
    return false;
  } else {
    cout << "[Log Info] TrackerClient: Response Message received:" << endl
         << response.DebugString() << endl;    
    return true;
  }
}

}  // namespace
