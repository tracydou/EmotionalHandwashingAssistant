/*
 *  File:				tracking_model.h
 *  Created by:			Luyuan Lin
 *  Created:			April 2014
 *  Last Modified:		April 2014
 *  Last Modified by:	Luyuan Lin
 * 
 *  Defined struct trackingModel, modified from Steve Czarnuch's codes
 *  (originally was in defines.h)
 */
 
#ifndef TRACKING_MODEL_
#define TRACKING_MODEL_

#include <string> 
#include <vector>
#include "opencv2/opencv.hpp"

using std::string;
using std::vector;
using cv::Point3_;
using cv::Point;
using cv::Mat;

// from configuration file
struct part{
	vector<Point3_<int> > colours;		// colour of label (BGR)
	vector<string> label;				// readable names of the parts
	vector<bool> classify;					// classified or not
	vector<float> bandwidths;				// per-part bandwidths for mean shift
	vector<float> probThreshs;				// probability thresholds for initial classification
	int count;									// total number of labels
	Point3_<int> bgColour;					// background colour
};

typedef struct
{
	Point3_<int> centre;		// region centre in projective coordinates
	float radius;				// radius of region
	string label;				// name of the region
} objectRegion;

struct trackingModel{

	// general configuration
	part bodyParts;										// configuration parameters for body parts in tracking model
	vector<objectRegion> regions;						// vector of environmental regions

	// primary tracking data for current frame
	vector<Point> samples;								// randomly generated set of image samples

	// intermediate tracking data for current frame
	vector<Point3_<float> > worldPtsVec;				// vectors for classified points in world space
	vector<Mat> weightings;								// vector of PDFs of each classified point (i.e., all points that were successfully classified out of original random samples)
	vector<vector<Point3_<float> > > startPoints;		// vector of start points for each body part for mode search
	vector<vector<Point3_<float> > > modes;				// vector of all modes (in world space coordinates) for each body part
	vector<vector<float> > modeConf;					// vector of the confidence estimate of each mode for each body part

	// final tracking data for current frame
	vector<Point3_ <int> > partCentres;					// vector of body part centres in projective space
	vector<float> partConf;								// confidence estimate of part centres
	vector<int> nullProposals;							// count of consecutive failed proposals
	Point handAction;									// task action of left (x) and right (y) hand for current frame

	// parameters over an entire trial
	vector<vector<Point3_<float> > > gtPartCentres;		// vector of ground truth body part centres (in world space coordinates) for each frame in a set of validation images
	vector<vector<Point3_<float> > > classPartCentres;	// vector of classified body part centres (in world space coordinates) for each frame in a set of validation images
	vector<Point> gtHandPos;							// vector of ground truth hand positions (x - left, y - right) for each frame over a set of validation images
	vector<int> gtAction;								// vector of participant action for each frame over a set of validation images
	vector<Point> classHandPos;							// vector of classified hand positions (x - left, y - right) for each frame over a set of validation images
	vector<Point> classHandAction;
};

#endif  // TRACKING_MODEL_
