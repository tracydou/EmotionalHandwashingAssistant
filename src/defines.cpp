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

}  // namespace EHwA
