/*
 *  File:				kalmanfilter.h
 *  Created by:			Steve Czarnuch
 *  Created:			July 2013
 *  Last Modified:		July 2013
 *  Last Modified by:	Steve Czarnuch
 */

#ifndef _FILTER
#define _FILTER

#include "opencv2/opencv.hpp"

#include "defines.h"

#define NUM_DYNAMIC 6		// dynamic parameters x,y,z and dx, dy, dz
#define NUM_MEASURED 6		// measured parameters are x,y,z position (dx, dy, dz are still here but always zero)

using namespace cv;

class Filter {
public:

	Filter(int bpart);

	KalmanFilter KF;						// x,y,z and dx, dy, dz are dynamic parameters, x,y,z are measured parameters
	Mat_<float> state;
	Mat_<float> processNoise;				// x,y,z and dx, dy, dz
	Mat_<float> measurement;  				// x,y,z position

private:


};
#endif
