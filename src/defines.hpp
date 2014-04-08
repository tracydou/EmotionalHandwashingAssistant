/*
 *  File:				defines.hpp
 *  Created by:			Luyuan Lin
 * 
 *  Shared definitions among all EHwA src files
 */

#ifndef DEFINES_
#define DEFINES_

//OpenCV libraries
#include "opencv2/opencv.hpp"

using cv::Point3_;

namespace EHwA {

typedef Point3_<float> Position;
const int UNKNOWN_ACTION = -1;


// ===================================================================
// DO NOT edit the following, unless the depending files changed!
// Dependency files:
//   ../lib/hand_tracker/config.txt
//   ../lib/hand_tracker/hand_tracker.hpp & hand_tracker.cpp
// ===================================================================

// Currently, type condHandAction is used by the HandTrackerServer.
// See processRequestsIdle() defined in ../lib/hand_tracker/hand_tracker.hpp for details

// rawHandAction constants:
// This MUST be consistant with ../lib/hand_tracker/regions.txt
// const int AWAY = 0;
// const int SINK = 1;
// const int SOAP = 2;
// const int WATER = 3;
// const int L_TAP = 4;
// const int R_TAP = 5;
// const int TOWEL = 6;

// condHandAction constants:
// This MUST be consistant with ../lib/hand_tracker/config.txt (the activity part)
const int UNDEF = 0;	// task step is undefined
const int AWAY	= 1;	// user does not have his/her hands in any active regions
const int SINK	= 2;	// user has his/her hands in the sink, but is not under the water
const int SOAP	= 3;	// user is getting soap
const int TAP	= 4;    // user is adjusting the taps
const int WATER = 5;	// user has his/her hands under the water
const int TOWEL = 6;	// user is drying his/her hands

}  // namespace EHwA

#endif  // DEFINES_
