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

bool TrackerClient::GetHandPosition(bool left_hand, Position& pos) { 
  // Construct request message
  HandTrackerRequest request;
  if (left_hand == true) {
    request.set_request_type(HandTrackerRequest_RequestType_LEFT_HAND_POS);
  } else {
    request.set_request_type(HandTrackerRequest_RequestType_RIGHT_HAND_POS);
  }
  // Send out request and wait for response
  SendHandTrackerRequest(request);
  // Decode received message
  HandTrackerResponseHandPos response;
  if (!ReceiveHandPos(response)) {
	  cout << "Failed in getting hand pos!" << endl;
	  return false;
  } else {
	  pos.x = response.x();
	  pos.y = response.y();
	  pos.z = response.z();
	  return true;
  }
};

bool TrackerClient::GetHandAction(int& action) {
  // Construct request message
  HandTrackerRequest request;
  request.set_request_type(HandTrackerRequest_RequestType_ACTION);
  // Send out request and wait for response
  SendHandTrackerRequest(request);
  // Decode received message
  HandTrackerResponseAction response;
  if (!ReceiveAction(response)) {
	  cout << "Failed in getting current action!" << endl;
	  return false;
  } else {
	  action = response.action();
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
    cout << "=========[TrackerClient] Sent Request Message:" << endl
         << request.DebugString() << endl
         << "Waiting for response..." << endl;
    return true;
  }
}

bool TrackerClient::ReceiveHandPos(
  HandTrackerResponseHandPos& response) {
  zmq::message_t message;
  handtracker_socket_.recv(&message);
  if (!response.ParseFromString(
    string((const char *)message.data(), message.size()))) {
    cout << "Message.ParseFromString(message) Failed!" << endl;
    return false;
  } else {
    cout << "=========[TrackerClient] Response Message received:"
         << endl << response.DebugString() << endl;    
    return true;
  }
}

bool TrackerClient::ReceiveAction(
  HandTrackerResponseAction& response) {
  zmq::message_t message;
  handtracker_socket_.recv(&message);
  if (!response.ParseFromString(
    string((const char *)message.data(), message.size()))) {
    cout << "Message.ParseFromString(message) Failed!" << endl;
    return false;
  } else {
    cout << "=========[TrackerClient] Response Message received:"
         << endl << response.DebugString() << endl;    
    return true;
  }
}

}  // namespace
