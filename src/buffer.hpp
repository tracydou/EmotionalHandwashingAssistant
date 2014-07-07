/*
 *  File:				buffer.hpp
 *  Created by:			Luyuan Lin
 * 
 *  Defines a Buffer class that sits between the handtracker-server
 *  and the bayesact-server. The buffer contains an EPA-Calc to compute
 *  EPA values of user actions and temporal smooths the results.
 * 
 *  State A of buffer:
 *    * check if user behaviour stays the same during a "timeout" period
 *    * go to state B if new behaviour detected
 *    * go to state C if timeout
 *  State B of buffer:
 *    * check if user behaviour stays the same during a "timeup" period
 *    * go to state A if new behaviour detected
 *    * go to state C if timeup
 *  State C of buffer:
 *    * send msg to the bayesact engine
 *    * go to state A
 */

#ifndef BUFFER_H_
#define BUFFER_H_

#include <iostream>
#include <utility>
#include <list>

#include "defines.hpp"
#include "EPACalculator/epa_calculator.hpp"

using std::cout;
using std::endl;
using std::list;

namespace EHwA {

class Buffer {
public:
  Buffer(double threshold_timeout, double threshold_timeup,
          vector<double> default_epa);
  ~Buffer();
  
  int get_current_state();
  int get_current_user_behaviour();
  vector<double> get_current_epa();
    
  // called for each frame
  void Update(int hand_action, const Position& left_hand_pos,
               const Position& right_hand_pos);
  void ChangeToState(int new_state, int hand_action);
    
public:
  static const int STATE_B;
  static const int STATE_A;
  static const int STATE_C;
  static const double ALPHA; // the weight of previous_epa v.s. the weight of current_epa
  
private:
  bool time0_;

  // maintain the value of "current behaviour" 
  int current_state_;
  double threshold_timeout_, threshold_timeup_; // threshold_timeup_ < threshold_timeout_
  double time_count_A_, time_count_B_; // time count in state A & B
  int behaviour_state_A_, behaviour_state_B_; // behaviour to check in state A & B
  
  // maintain the value of "EPA computed for current behaviour"
  list<pair<Position, Position> > hand_positions_;
  vector<double> current_epa_;
  
};

}  // namespace EHwA

#endif // BUFFER_H_
