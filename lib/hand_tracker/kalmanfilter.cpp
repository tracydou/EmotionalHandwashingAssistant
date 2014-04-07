/*
 *  File:				kalmanfilter.cpp
 *  Created by:			Steve Czarnuch
 *  Created:			July 2013
 *  Last Modified:		July 2013
 *  Last Modified by:	Steve Czarnuch
 */

#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "kalmanfilter.h"
#include "defines.h"

using namespace cv;

Filter::Filter(int bpart) : KF(NUM_DYNAMIC,NUM_MEASURED,0){

	state = Mat_<float>::zeros(Size(1,NUM_DYNAMIC));
	processNoise = Mat_<float>::zeros(Size(NUM_DYNAMIC,NUM_DYNAMIC));
	measurement = Mat_<float>::zeros(Size(1,NUM_MEASURED));

	// corrected state - for initialization of the filter
	KF.statePost.at<float>(0) = WIDTH/2;	// x
	KF.statePost.at<float>(1) = HEIGHT/2;// y
	KF.statePost.at<float>(2) = 128;	// z
	KF.statePost.at<float>(3) = 0;	// dx
	KF.statePost.at<float>(4) = 0;	// dy
	KF.statePost.at<float>(5) = 0;	// dz

	/* Set part-specific values for process noise covariance matrix (Q): Large value if the signal varies quickly (need an adaptable filter).
	 * Small value means big variations will be attributed to noise
	 */
	switch(bpart)
	{
	case 0:
	case 1:
		KF.processNoiseCov = *(cv::Mat_<float>(NUM_DYNAMIC,NUM_DYNAMIC) <<
				1e-4, 0.0 , 0.0 , 1e-4, 0.0 , 0.0 ,
				0.0 , 1e-4, 0.0 , 0.0 , 1e-4, 0.0 ,
				0.0 , 0.0 , 1e-4, 0.0 , 0.0 , 1e-4,
				0.0 , 0.0 , 0.0 , 1e-2, 0.0 , 0.0 ,
				0.0 , 0.0 , 0.0 , 0.0 , 1e-2, 0.0 ,
				0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 1e-2);
		break;
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
		KF.processNoiseCov = *(cv::Mat_<float>(NUM_DYNAMIC,NUM_DYNAMIC) <<
				0.2 , 0.0 , 0.0 , 0.2 , 0.0 , 0.0 ,
				0.0 , 0.2 , 0.0 , 0.0 , 0.2 , 0.0 ,
				0.0 , 0.0 , 0.2 , 0.0 , 0.0 , 0.2 ,
				0.0 , 0.0 , 0.0 , 0.3 , 0.0 , 0.0 ,
				0.0 , 0.0 , 0.0 , 0.0 , 0.3 , 0.0 ,
				0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.3 );
		break;
	default:
		std::cout << "Body part " << bpart << " not configured for Kalman filter. Shutting down." << std::endl;
		exit(0);
	}


	// State transition matrix (A)
	KF.transitionMatrix = *(Mat_<float>(NUM_DYNAMIC, NUM_DYNAMIC) <<
			1 , 0 , 0 , 1 , 0 , 0 , 		// x is dependent on x + dx
			0 , 1 , 0 , 0 , 1 , 0 , 		// y is dependent on y + dy
			0 , 0 , 1 , 0 , 0 , 1 , 		// z is dependent on z + dz
			0 , 0 , 0 , 1 , 0 , 0 , 		// dx is only dependent on dx
			0 , 0 , 0 , 0 , 1 , 0 , 		// dy is only dependent on dy
			0 , 0 , 0 , 0 , 0 , 1);			// dz is only dependent on dz

	// Measurement Matrix (H)
	setIdentity(KF.measurementMatrix);

	// TODO estimate measurement noise by observing "sensor" variation (from output of Mode Find)
	// Measurement noise covariance matrix (R)
	// Adjustments to this seem to have the most significant effect on how quickly the filter responds to noisy data. This also means that fast hand motions are lost.
	// Small values mean that the sensors don't have a lot of noise (i.e., trust each reading). Large values mean that readings are not entirely reliable.
	KF.measurementNoiseCov = *(Mat_<float>(NUM_MEASURED,NUM_MEASURED) <<
			5e-2, 0.0 , 0.0 , 0.0 , 0.0 , 0.0 ,
			0.0 , 5e-2, 0.0 , 0.0 , 0.0 , 0.0 ,
			0.0 , 0.0 , 5e-2, 0.0 , 0.0 , 0.0 ,
			0.0 , 0.0 , 0.0 , 1e-1, 0.0 , 0.0 ,
			0.0 , 0.0 , 0.0 , 0.0 , 1e-1, 0.0 ,
			0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 1e-1);

	// Priori error estimate covariance matrix
	setIdentity(KF.errorCovPost, Scalar::all(.1));

}
