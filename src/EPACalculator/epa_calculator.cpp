/*
 *  File:				epa_calculator.cpp
 *  Created by:			Luyuan Lin
 *  
 */

#include "epa_calculator.hpp"
namespace EHwA {

const double EPACalculator::THRESHOLD_DIST_FOR_POTENCY = 150;
const double EPACalculator::THRESHOLD_DIFF_FOR_ACTIVITY = 30;
const double EPACalculator::max_epa = 4.3;
const double EPACalculator::min_epa = -4.3;

EPACalculator::EPACalculator() {
  // do nothing
}

EPACalculator::~EPACalculator() {
	// do nothing
}

// threshold based solution
double EPACalculator::ConvertDistToPotency(
  const list<pair<Position, Position> >& hand_pos) {
  // use whatever is collected (up to NUM_POSITIONS_NEEDED handpositions)
  double dist = 0;
  list<pair<Position, Position> >::const_iterator it = hand_pos.begin();
  unsigned int i = 0;
  for (; it != hand_pos.end() && i < NUM_POSITIONS_NEEDED; ++it, ++i) {
    dist += get_distance_between_points((*it).first, (*it).second);
  }
  dist = dist / NUM_POSITIONS_NEEDED;
  if (dist >= THRESHOLD_DIST_FOR_POTENCY) { // max epa value
    return max_epa;
  } else {
    return (max_epa - min_epa) * dist / THRESHOLD_DIST_FOR_POTENCY + min_epa;
  }  
}

// threshold based solution
double EPACalculator::ConvertDiffToActivity(
  const list<pair<Position, Position> >& hand_pos) {
  // use whatever is collected (up to NUM_POSITIONS_NEEDED handpositions)
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
  if (diff >= THRESHOLD_DIFF_FOR_ACTIVITY) { // max epa value
    return max_epa;
  } else {
    return (max_epa - min_epa) * diff / THRESHOLD_DIFF_FOR_ACTIVITY + min_epa;
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
