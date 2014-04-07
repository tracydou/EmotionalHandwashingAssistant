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

// ZMQ & others, used in class ZMQServer
#include <string>
#include <vector>
#include <zmq.h>
#include "../../cppzmq/zmq.hpp"
#include "tracker_message.pb.h"

using std::string;
using std::vector;

#define BILLION  1000000000L;

// converted from mainIdle() by changing func name
gint trackerIdle(void* data);

// listen to requests from "client" and process recordingly
gint processRequestsIdle(TrackerServerStub* server_stub);
// 1) if requests for LeftHandPos
bool respondWithLeftHandPos(TrackerServerStub* server_stub);
// 2) else if requests for RightHandPos
bool respondWithRightHandPos(TrackerServerStub* server_stub);
// 3) else if requests for HandAction
bool respondWithAction(TrackerServerStub* server_stub);

public class TrackerServerStub {
public:
    TrackerServerStub(string addr);
    ~TrackerServerStub();
    int Receive();
    bool SendHandPos(Point3_<float> hand_position);
    bool SendAction(int action);
    
    const static int TYPE_ERROR = -1;
    const static int TYPE_LEFT_HAND_POS = 0;
    const static int TYPE_RIGHT_HAND_POS = 1;
    const static int TYPE_ACTION = 2;
private:    
    zmq::context_t context;
    zmq::socket_t socket;
};

#endif  // TRACKER_
