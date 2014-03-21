/*
 *  File:				EPACalculator.cpp
 *  Created by:			Luyuan Lin
 *  Created:			March 2014
 *  Last Modified:		March 2014
 *  Last Modified by:	Luyuan Lin
 *  
 */

#include "EPACalculator.hpp"

EPACalculator::EPACalculator(){
  // init currentEPA to <0,0,0>
  currentEPA.resize(3);
  for (int i = 0; i < 3; ++i) {
	  currentEPA[i] = 0;
  }
}

EPACalculator::~EPACalculator(){
	// do nothing
}

double EPACalculator::ConvertFacialExpressionToEvaluation(
  const vector<FacialExpression>& facialExp) {
  // TODO
  return 0.0;
}
    
double EPACalculator::ConvertDistToPotency(
  const vector<vector<Point3_>>& handPos) {
  // TODO
  return 0.0;
}

double EPACalculator::ConvertDiffToActivity(
  const vector<vector<Point3_>>& handPos) {
  return 0.0;
}

vector<double> EPACalculator::Calculate(
  const vector<FacialExpression>& facialExp,
  const vector<vector<Point3_>>& handPos);){
  currentEPA[0] = ConvertFacialExpressionToEvaluation(facialExp);
  currentEPA[1] = ConvertDistToPotency(handPos);
  currentEPA[2] = ConvertDiffToActivity(handPos);
  return currentEPA;
}

vector<double> EPACalculator::getCurrentEPA(){
  return currentEPA;
}
