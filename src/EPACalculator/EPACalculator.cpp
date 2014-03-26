/*
 *  File:				EPACalculator.cpp
 *  Created by:			Luyuan Lin
 *  Created:			March 2014
 *  Last Modified:		March 2014
 *  Last Modified by:	Luyuan Lin
 *  
 */

#include "EPACalculator.hpp"
namespace EHwA {
	
EPACalculator::EPACalculator() {
  // init currentEPA to <0,0,0>
  currentEPA.resize(3);
  for (int i = 0; i < 3; ++i) {
	  currentEPA[i] = 0.0;
  }
}

EPACalculator::~EPACalculator() {
	// do nothing
}

    
double EPACalculator::ConvertDistToPotency(
  const vector<pair<Position, Position> >& handPos) {
  // TODO: implement this method
  return 0.0;
}

double EPACalculator::ConvertDiffToActivity(
  const vector<pair<Position, Position> >& handPos) {
  // TODO: implement this method
  return 0.0;
}

vector<double> EPACalculator::Calculate(
  const vector<pair<Position, Position> >& handPos) {
  currentEPA[1] = ConvertDistToPotency(handPos);
  currentEPA[2] = ConvertDiffToActivity(handPos);
  return currentEPA;
}

vector<double> EPACalculator::getCurrentEPA() const{
  return currentEPA;
}

}  // namespace
