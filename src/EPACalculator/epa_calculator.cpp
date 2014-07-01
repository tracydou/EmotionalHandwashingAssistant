/*
 *  File:				epa_calculator.cpp
 *  Created by:			Luyuan Lin
 *  
 */

#include "epa_calculator.hpp"
namespace EHwA {

const double EPACalculator::THRESHOLD_DIST[] = {0,8,40,128,160};
const double EPACalculator::THRESHOLD_POTECY[] = {-4.3,0,1,2,4.3};
const double EPACalculator::THRESHOLD_DIFF[] = {0,3.5,17.5,35,70};
const double EPACalculator::THRESHOLD_ACTIVITY[] = {-4.3,-2,-1,0,4.3};
const int EPACalculator::LENGTH_FOR_POTENCY = 5, EPACalculator::LENGTH_FOR_ACTIVITY = 5;

EPACalculator::EPACalculator() {
  // do nothing
}

EPACalculator::~EPACalculator() {
	// do nothing
}

// threshold based solution
double EPACalculator::ConvertDistToPotency(
  const list<pair<Position, Position> >& hand_pos) {
  // compute dist using whatever is collected (up to NUM_POSITIONS_NEEDED handpositions)
  double dist = 0;
  list<pair<Position, Position> >::const_iterator it = hand_pos.begin();
  unsigned int i = 0;
  for (; it != hand_pos.end() && i < NUM_POSITIONS_NEEDED; ++it, ++i) {
    dist += get_distance_between_points((*it).first, (*it).second);
  }
  dist = dist / NUM_POSITIONS_NEEDED;
  // convert dist to the P value of user behaviour
  int size = LENGTH_FOR_POTENCY;
  if (dist <= THRESHOLD_DIST[0]) { // min epa value
    return THRESHOLD_POTECY[0];
  } else {
    for (int k = 0; k < size -1; ++k) {
      if (dist <= THRESHOLD_DIST[k+1]) {
        double dist_range = THRESHOLD_DIST[k+1] - THRESHOLD_DIST[k];
        double potency_range = THRESHOLD_POTECY[k+1] - THRESHOLD_POTECY[k];
        return potency_range * (dist - THRESHOLD_DIST[k]) / dist_range + THRESHOLD_POTECY[k];
      }
    }
    return THRESHOLD_POTECY[size-1];
  }  
}

// threshold based solution
double EPACalculator::ConvertDiffToActivity(
  const list<pair<Position, Position> >& hand_pos) {
  // compute diff using whatever is collected (up to NUM_POSITIONS_NEEDED handpositions)
  double diff = 0;
  list<pair<Position, Position> >::const_iterator it = hand_pos.begin();
  list<pair<Position, Position> >::const_iterator previous_it = it;
  ++it;
  unsigned int i = 1;
  for (; it != hand_pos.end() && i < NUM_POSITIONS_NEEDED;
       ++previous_it, ++it, ++i) {
    double diff_left = get_distance_between_points((*previous_it).first, (*it).first);
    double diff_right = get_distance_between_points((*previous_it).second, (*it).second); 
    diff += (diff_left > diff_right) ? diff_left : diff_right;
  }
  diff = diff / (NUM_POSITIONS_NEEDED - 1);
  // convert diff to the A value of user behaviour
  int size = LENGTH_FOR_ACTIVITY;
  if (diff <= THRESHOLD_DIFF[0]) { // min epa value
    return THRESHOLD_ACTIVITY[0];
  } else {
    for (int k = 0; k < size -1; ++k) {
      if (diff <= THRESHOLD_DIFF[k+1]) {
        double diff_range = THRESHOLD_DIFF[k+1] - THRESHOLD_DIFF[k];
        double activity_range = THRESHOLD_ACTIVITY[k+1] - THRESHOLD_ACTIVITY[k];
        return activity_range * (diff - THRESHOLD_DIFF[k]) / diff_range + THRESHOLD_ACTIVITY[k];
      }
    }
    return THRESHOLD_ACTIVITY[size-1];
  }  
}

vector<double> EPACalculator::Calculate(
  const list<pair<Position, Position> >& hand_pos) {
  // currently "evaluation" stays as 0.0 for the whole process
  vector<double> epa(3);
  epa[0] = 0;
  epa[1] = ConvertDistToPotency(hand_pos);
  epa[2] = ConvertDiffToActivity(hand_pos);
  return epa;
}

}  // namespace
