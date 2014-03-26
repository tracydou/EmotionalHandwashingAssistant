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
#include "BayesActMessage.pb.h"

namespace EHwA {

class BayesActMessage;

BayesActClient::BayesActClient(string addr) {
  // init field attributes to default values
  respondedEPA.resize(3);
  for (int i = 0; i < 3; ++i) {
    respondedEPA[i] = 0.0;
  }
  respondedPrompt = -1;
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
  // pack and convert to BayesActMessage sent to server
  BayesActMessage message;
  message.set_evaluation(EPA[0]);
  message.set_potency(EPA[1]);
  message.set_activity(EPA[2]);
  message.set_handaction(handAction);
  message.set_messagetype(BayesActMessage::CLIENT_TO_SERVER);
  // send out message if suceeded
  if (!message.SerializeToString(&buffer)) {
    cout << "Message.SerializeToString(&buffer) Failed!" << endl;
    return false;
  } else {
    zmq_send (requester, buffer.c_str(), sizeof(buffer.c_str()), 0);
    return true;
  }
}

// receive & decode responded epa & prompt
bool BayesActClient::Receive() {
  zmq_recv (requester, (void*)(&buffer), sizeof(BayesActMessage), 0);
  BayesActMessage message;
  if (!message.ParseFromString(buffer)) {
    cout << "Message.ParseFromString(buffer) Failed!" << endl;
    return false;
  } else {
	assert(message.messagetype() == BayesActMessage::SERVER_TO_CLIENT);
	respondedEPA[0] = message.evaluation();
	respondedEPA[1] = message.potency();
	respondedEPA[2] = message.activity();
	if (message.has_prompt()) {
      respondedPrompt = message.prompt();
    }
    return true;
  }
}

vector<double> BayesActClient::getRespondedEPA() {
  return respondedEPA;
}

int BayesActClient::getRespondedPrompt() {
  return respondedPrompt;
}

}  // namespace EHwA

