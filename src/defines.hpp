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
const int NOT_FOUND_PROPOSITION_PROMPT = -1; // used in PromptSelecter
const double MAX_DIST = 3000000; // used in PromptSelecter

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
// These values are shared between HandTrackerServer and bayesactlib.py

// TODO: make sure these values are consistant with each other
/***************************************************************/

}  // namespace EHwA

#endif  // DEFINES_
