/*
 *  File:				epa_calculator.cpp
 *  Created by:			Luyuan Lin
 *  Created:			March 2014
 *  Last Modified:		March 2014
 *  Last Modified by:	Luyuan Lin
 *  
 */

#include "epa_calculator.hpp"
namespace EHwA {
	
EPACalculator::EPACalculator() {
  // init currentEPA to <0,0,0>
  current_epa_.resize(3);
  for (int i = 0; i < 3; ++i) {
	  current_epa_[i] = 0.0;
  }
}

EPACalculator::~EPACalculator() {
	// do nothing
}

    
double EPACalculator::ConvertDistToPotency(
  const vector<pair<Position, Position> >& hand_pos) {
  // TODO: implement this method
  return 0.0;
}

double EPACalculator::ConvertDiffToActivity(
  const vector<pair<Position, Position> >& hand_pos) {
  // TODO: implement this method
  return 0.0;
}

vector<double> EPACalculator::Calculate(
  const vector<pair<Position, Position> >& hand_pos) {
  current_epa_[1] = ConvertDistToPotency(hand_pos);
  current_epa_[2] = ConvertDiffToActivity(hand_pos);
  return current_epa_;
}

vector<double> EPACalculator::get_current_epa() const{
  return current_epa_;
}

}  // namespace
