/*
 *  File:				BayesActClient.cpp
 *  Created by:			Luyuan Lin
 *  Created:			March 2014
 *  Last Modified:		March 2014
 *  Last Modified by:	Luyuan Lin
 * 
 */

#include <iostream>
#include <zmq.h>
#include <../lib/cppzmq/zmq.hpp>
#include "BayesActClient.hpp"

using std::cout;
using std::endl;

namespace EHwA {

class BayesActRequest;
class BayesActResponse;

BayesActClient::BayesActClient(string addr): context(1), socket(context, ZMQ_REQ) {
  // set up connection to server
  cout << "Connecting to server..." << endl;
  socket.connect(addr.c_str());
}

BayesActClient::~BayesActClient() {
  // Optional:  Delete all global objects allocated by libprotobuf.
  google::protobuf::ShutdownProtobufLibrary();
}

bool BayesActClient::Send(const vector<double>& epa, int hand_action) {
  // pack and convert to BayesActRequest sent to server
  BayesActRequest request;
  request.set_evaluation(epa[0]);
  request.set_potency(epa[1]);
  request.set_activity(epa[2]);
  request.set_handaction(hand_action);
  string message;
  // send out message if suceeded
  if (!request.SerializeToString(&message)) {
    cout << "Message.SerializeToString(&buffer) Failed!" << endl;
    return false;
  } else {
    socket.send(message.c_str(), message.length());
    cout << "=========[Log Info] Sent Request Message:" << endl
         << request.DebugString() << endl
         << "Waiting for response..." << endl;
    return true;
  }
}

// receive & decode responded epa & prompt
bool BayesActClient::Receive() {
  zmq::message_t message;
  socket.recv(&message);
  if (!response.ParseFromString(string((const char *)message.data(), message.size()))) {
    cout << "Message.ParseFromString(buffer) Failed!" << endl;
    return false;
  } else {
    cout << "=========[Log Info] Response Message received:" << endl
         << response.DebugString() << endl;    
    return true;
  }
}

vector<double> BayesActClient::getResponseEPA() {
  vector<double> epa(3);
  epa[0] = response.evaluation();
  epa[1] = response.potency();
  epa[2] = response.activity();
  return epa;
}

int BayesActClient::getResponsePrompt() {
  return response.prompt();
}

}  // namespace EHwA

