/*
 *  File:				tracker.hpp
 *  Created by:			Luyuan Lin
 *  Created:			April 2014
 *  Last Modified:		April 2014
 *  Last Modified by:	Luyuan Lin
 *  
 *  This file and main.cpp are all modified from the old main.cpp
 *  file. Changed places include newly added funcs and several changes
 *  to original main(), mainIdle() (now trackerIdle()), etc.
 * 
 *  Functions declared in this file are implemented in main.cpp. 
 */
 
#ifndef TRACKER_
#define TRACKER_

//OpenCV libraries
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

// openni
#include "ni/XnCppWrapper.h"

// NITE
#include "nite/XnV3DVector.h"

// Open MP
#include <omp.h>

#include "gtk/gtk.h"
#include <gdk/gdkkeysyms.h>

#include <fstream>
#include <dirent.h>
#include <iomanip>
#include <limits>

#include "tree.h"
#include "defines.h"
#include "grab.h"
#include "kalmanfilter.h"

#define BILLION  1000000000L;

// converted from mainIdle() by changing func name
gint trackerIdle(void* data);

// listen to requests from "client" and process recordingly
gint processRequestsIdle(void* data);
// 1) if requests for LeftHandPos
void respondWithLeftHandPos();
// 2) else if requests for RightHandPos
void respondWithRightHandPos();
// 3) else if requests for HandAction
void respondWithHandAction();

// connect to server

#endif  // TRACKER_
