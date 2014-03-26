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
using std::string;
using std::vector;

namespace EHwA {

class BayesActClient {
public:
  BayesActClient(string addr);
  ~BayesActClient();
  
  bool Send(const vector<double>& EPA, int handAction); // encode & send
  bool Receive(); // receive & decode responded epa & prompt
  vector<double> getRespondedEPA(); // call after Receive() is called
  int getRespondedPrompt(); // call after Receive() is called
  
protected:  
  string buffer; // used to store messages sent to and received from server
  vector<double> respondedEPA;
  int respondedPrompt;
  
  void* context;
  void* requester;
};

}  // namespace EHwA
#endif  // BAYES_ACT_CLIENT_

