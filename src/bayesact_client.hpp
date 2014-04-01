/*
 *  File:				bayesact_client.hpp
 *  Created by:			Luyuan Lin
 *  Created:			March 2014
 *  Last Modified:		March 2014
 *  Last Modified by:	Luyuan Lin
 * 
 *  Define a client communicate with the BayesAct engine.
 */

#ifndef BAYESACT_CLIENT_
#define BAYESACT_CLIENT_

#include <string>
#include <vector>
#include <../lib/cppzmq/zmq.hpp>
#include "bayesact_message.pb.h"
#include "defines.hpp"

using std::string;
using std::vector;

namespace EHwA {

class BayesactClient {
public:
  BayesactClient(string addr);
  ~BayesactClient();
  
  bool Send(const vector<double>& EPA, int hand_action); // encode & send
  bool Receive(); // receive & decode responded epa & prompt
  vector<double> get_response_epa(); // call after Receive() is called
  int get_response_prompt(); // call after Receive() is called
  
private:
  zmq::context_t context;
  zmq::socket_t socket;
  BayesactResponse response;
};

}  // namespace EHwA

#endif  // BAYESACT_CLIENT_

