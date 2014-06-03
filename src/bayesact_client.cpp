/*
 *  File:				bayesact_client.cpp
 *  Created by:			Luyuan Lin
 * 
 */

#include <iostream>
#include <zmq.h>
#include "../lib/cppzmq/zmq.hpp"
#include "bayesact_client.hpp"

using std::cout;
using std::endl;

namespace EHwA {

class BayesactRequest;
class BayesactResponse;

BayesactClient::BayesactClient(string addr):
  context_(1), socket_(context_, ZMQ_REQ) {
  // set up connection to server
  cout << "Connecting to server..." << endl;
  socket_.connect(addr.c_str());
}

BayesactClient::~BayesactClient() {
  // Optional:  Delete all global objects allocated by libprotobuf.
  google::protobuf::ShutdownProtobufLibrary();
}

bool BayesactClient::Send(const vector<double>& epa, int hand_action) {
  // pack and convert to BayesActRequest sent to server
  BayesactRequest request;
  request.set_evaluation(epa[0]);
  request.set_potency(epa[1]);
  request.set_activity(epa[2]);
  request.set_hand_action(hand_action);
  string message;
  // send out message if suceeded
  if (!request.SerializeToString(&message)) {
    cout << "Message.SerializeToString(&buffer) Failed!" << endl;
    return false;
  } else {
    socket_.send(message.c_str(), message.length());
    cout << "[Log Info] BayesactClient: Sent Request Message:" << endl
         << request.DebugString()
         << "Waiting for BayesactServer's response..." << endl;
    return true;
  }
}

// receive & decode responded epa & prompt
bool BayesactClient::Receive() {
  zmq::message_t message;
  socket_.recv(&message);
  if (!response_.ParseFromString(
    string((const char *)message.data(), message.size()))) {
    cout << "Message.ParseFromString(buffer) Failed!" << endl;
    return false;
  } else {
    cout << "[Log Info] BayesactClient: Response Message received:" << endl
         << response_.DebugString() << endl;    
    return true;
  }
}

vector<double> BayesactClient::get_response_epa() {
  vector<double> epa(3);
  epa[0] = response_.evaluation();
  epa[1] = response_.potency();
  epa[2] = response_.activity();
  return epa;
}

int BayesactClient::get_response_prompt() {
  return response_.prompt();
}

bool BayesactClient::is_done() {
	return response_.is_done();
}

}  // namespace EHwA

