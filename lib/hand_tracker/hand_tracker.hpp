/*
 *  File:				tracker.hpp
 *  Created by:			Luyuan Lin
 *  Created:			April 2014
 *  Last Modified:		April 2014
 *  Last Modified by:	Luyuan Lin
 *  
 *  This file and tracker.cpp are both modified from the old main.cpp
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

// ZMQ & others, used in class HandTrackerServerStub
#include <string>
#include <vector>
#include <zmq.h>
#include "../cppzmq/zmq.hpp"
#include "hand_tracker_message.pb.h"

using std::string;
using std::vector;

#define BILLION  1000000000L;

class HandTrackerServerStub {
public:
    HandTrackerServerStub(const char* addr);
    ~HandTrackerServerStub();
    int ReceiveRequest();
    bool SendResponse(const vector<Point3_<float> >& hand_positions, int action);
    
    const static int TYPE_PARSE_ERROR = 0;
    const static int TYPE_MESSAGE_SUCCESS = 1;
    const static int TYPE_NO_MESSAGE = 2;
private:    
    zmq::context_t context;
    zmq::socket_t socket;
};


int hand_tracker_start(int argc, char** argv, HandTrackerServerStub* stub);

int fake_hand_tracker_start(int argc, char** argv, HandTrackerServerStub* stub);

// converted from mainIdle() by changing func name
gint handTrackerIdle(void* data);

// listen to requests from "client" and process recordingly
gint processRequestsIdle(void* server_stub);

#endif  // TRACKER_
