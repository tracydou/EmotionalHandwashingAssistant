#include <math.h>
#include "defines.hpp"

namespace EHwA {

int convert_tracker_action_to_bayesact_behaviour(int action) {
	if (action == TRACKER_ACTION_SOAP) {
		return BAYESACT_SOAP;
	} else if (action == TRACKER_ACTION_TAP) {
		return BAYESACT_TAP;
	} else if (action == TRACKER_ACTION_WATER) {
		return BAYESACT_WATER;
	} else if (action == TRACKER_ACTION_TOWEL) {
		return BAYESACT_TOWEL;
	} else {
		return BAYESACT_NOTHING;
	}
}

float get_distance_between_points(Position p1, Position p2) {
	float dist = 0;
	dist += (p1.x - p2.x) * (p1.x - p2.x);
	dist += (p1.y - p2.y) * (p1.y - p2.y);
	dist += (p1.z - p2.z) * (p1.z - p2.z);
	return sqrt(dist);
}

}  // namespace EHwA
