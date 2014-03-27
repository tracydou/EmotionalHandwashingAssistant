/*
 *  File:				BayesActClient.cpp
 *  Created by:			Luyuan Lin
 *  Created:			March 2014
 *  Last Modified:		March 2014
 *  Last Modified by:	Luyuan Lin
 * 
 */

#include <iostream>
using std::cout;
using std::endl;

#include <zmq.h>
#include "BayesActClient.hpp"

namespace EHwA {

class BayesActRequest;
class BayesActRespond;

BayesActClient::BayesActClient(string addr) {
  // set up connection to server
  cout << "Connecting to server..." << endl;
  context = zmq_ctx_new ();
  requester = zmq_socket (context, ZMQ_REQ);
  zmq_connect (requester, addr.c_str());
}

BayesActClient::~BayesActClient() {
  // close connection to server
  cout << "Closing connection to server..." << endl;
  zmq_close (requester);
  zmq_ctx_destroy (context);
  // Optional:  Delete all global objects allocated by libprotobuf.
  google::protobuf::ShutdownProtobufLibrary();
}

bool BayesActClient::Send(const vector<double>& EPA, int handAction) {
  // pack and convert to BayesActRequest sent to server
  requestMessage.set_evaluation(EPA[0]);
  requestMessage.set_potency(EPA[1]);
  requestMessage.set_activity(EPA[2]);
  requestMessage.set_handaction(handAction);
  // send out message if suceeded
  if (!requestMessage.SerializeToString(&requestBuffer)) {
    cout << "Message.SerializeToString(&buffer) Failed!" << endl;
    return false;
  } else {
    zmq_send (requester, (void*)requestBuffer.c_str(),
              requestBuffer.length(), 0);
    cout << "=========[Log Info] Sent Request Message:" << endl
         << requestMessage.DebugString() << endl
         << "Waiting for response..." << endl;
    return true;
  }
}

// receive & decode responded epa & prompt
bool BayesActClient::Receive() {
  zmq_recv (requester, respondBuffer, MAX_RESPOND_BUFFER_SIZE, 0);
  if (!respondMessage.ParseFromString(
    string(respondBuffer, MAX_RESPOND_BUFFER_SIZE))) {
    cout << "Message.ParseFromString(buffer) Failed!" << endl;
    cout << "respond buffer = " << respondBuffer << endl;
    return false;
  } else {
    cout << "=========[Log Info] Response Message received:" << endl
         << respondMessage.DebugString() << endl;    
    return true;
  }
}

vector<double> BayesActClient::getRespondedEPA() {
  vector<double> epa(3);
  epa[0] = respondMessage.evaluation();
  epa[1] = respondMessage.potency();
  epa[2] = respondMessage.activity();
  return epa;
}

int BayesActClient::getRespondedPrompt() {
  return respondMessage.prompt();
}

}  // namespace EHwA

