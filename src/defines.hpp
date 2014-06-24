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

typedef Point3_<float> Position; // used in TrackerClient & BayesactClient
const int UNKNOWN_ACTION = -1; // used in TrackerClient
const int INVALID_PROMPT = -1; // used in PromptSelecter
const int NO_PROMPT_PLEASE = 0; //used in PromptSelecter
const int NOT_FOUND_PROPOSITION_PROMPT = -1; // used in PromptSelecter
const double MAX_DIST = 3000000; // used in PromptSelecter

const unsigned int NUM_POSITIONS_NEEDED = 10; // Maximum # of handpositions needed; used in EPACalc & main.cpp
                                     // No minimum requirement

/***************************************************************/
float get_distance_between_points(Position p1, Position p2);

/***************************************************************/

// ----------- Constant variable "Prompt Number" ---------------------------

// "Propositional prompt number" used by Bayesact and "prompt number" used
// in PromptSelecter/ OutputMappingResult.csv should be consistant.
// To achieve this, a member func convert_prompt_number() is added in
// ../lib/bayesact/bayesactlib.py

/***************************************************************/

// ------------ Constant variable "Action" ---------------------------------

// Currently, type condHandAction, defined in ../lib/hand_tracker/defines.h
// is used by the HandTrackerServer.
// See processRequestsIdle() defined in ../lib/hand_tracker/hand_tracker.hpp for details
// condHandAction constants MUST be consistant with
// ../lib/hand_tracker/config.txt (the activity part)
// These values are shared between HandTrackerServer and bayesactlib.py, so need to be converted.

// Const values used in hand-tracker
const int TRACKER_ACTION_UNDEF = 0;	// task step is undefined
const int TRACKER_ACTION_AWAY	= 1;	// user does not have his/her hands in any active regions
const int TRACKER_ACTION_SINK	= 2;	// user has his/her hands in the sink, but is not under the water
const int TRACKER_ACTION_SOAP	= 3;	// user is getting soap
const int TRACKER_ACTION_TAP	= 4;    // user is adjusting the taps
const int TRACKER_ACTION_WATER = 5;	// user has his/her hands under the water
const int TRACKER_ACTION_TOWEL = 6;	// user is drying his/her hands

// Action/Behaviour constants used in bayesactlib.py:
const int BAYESACT_NOTHING = 0;
const int BAYESACT_SOAP = 1;
const int BAYESACT_TAP = 2;
const int BAYESACT_WATER = 3;
const int BAYESACT_TOWEL = 4;

// The converting process is called in TrackerClient's Receive()
int convert_tracker_action_to_bayesact_behaviour(int action);

// TODO: make sure these values are consistant with each other
/***************************************************************/

}  // namespace EHwA

#endif  // DEFINES_
