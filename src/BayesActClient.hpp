/*
 *  File:				BayesActClient.hpp
 *  Created by:			Luyuan Lin
 *  Created:			March 2014
 *  Last Modified:		March 2014
 *  Last Modified by:	Luyuan Lin
 * 
 *  Define a client communicate with the BayesAct engine.
 */

#ifndef BAYES_ACT_CLIENT_
#define BAYES_ACT_CLIENT_

#include <string>
#include <vector>
#include <../lib/cppzmq/zmq.hpp>
#include "bayesact_message.pb.h"
#include "defines.hpp"

using std::string;
using std::vector;

namespace EHwA {

class BayesActClient {
public:
  BayesActClient(string addr);
  ~BayesActClient();
  
  bool Send(const vector<double>& EPA, int handAction); // encode & send
  bool Receive(); // receive & decode responded epa & prompt
  vector<double> getResponseEPA(); // call after Receive() is called
  int getResponsePrompt(); // call after Receive() is called
  
private:
  zmq::context_t context;
  zmq::socket_t socket;
  BayesActResponse response;
};

}  // namespace EHwA

#endif  // BAYES_ACT_CLIENT_

