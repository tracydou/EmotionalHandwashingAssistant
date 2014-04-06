/*
 *  File:				defines.h
 *  Created by:			Steve Czarnuch
 *  Created:			January 2013
 *  Last Modified:		January 2013
 *  Last Modified by:	Steve Czarnuch
 */

#ifndef DEFINES_
#define DEFINES_

// Image parameters
#define WIDTH 640		// cropped image width
#define HEIGHT 480		// cropped image height
#define MIN_X 0.0			// minimum value of 8-bit image
#define MAX_X 255.0		// maximum value of 8-bit image
#define MIN_RAW_X 800.0	// minimum distance (in mm x 10) from the sensor. For kinect the minimum value is 1000 (100 mm)
#define MAX_RAW_X 4200.0	// maximum distance (in mm x 10) from the sensor. For kinect the max value is 4000 (400 mm)
#define OPENNI_ALPHA (MAX_X - MIN_X)/(MAX_RAW_X - MIN_RAW_X) 			// scaling factor for conversion from 16-bit to 8-bit depth image using OpenCV/OpenNI
#define OPENNI_BETA -(MAX_X - MIN_X)*(MIN_RAW_X)/(MAX_RAW_X - MIN_RAW_X)	// delta for conversion from 16-bit to 8-bit depth image using OpenCV/OpenNI
#define OPENNI_SCALE 255.0/MAX_RAW_X 		// scaling factor from 16-bit to 8-bit depth image using OpenCV/OpenNI (Linear scale from 0.0 to 420.0 mm)

#define VERBOSE 1				// verbose output

// runtime parameters
#define BUFFER_SIZE 10			// images in capture buffer
#define NULL_PROP_COUNT 3		// number of null proposals from classifier before disabling filter outputs

// Training parameters
#define INV_DEPTH 100000		// arbitrary value for an invalid depth. Note that this can be anything outside 0-65535...
#define BG_PIXEL 100001			// arbitrary value for an unlabeled (background) pixel in the training data set
#define GENERIC_BODY 100002		// generic body (unlabeled part)
#define MIN_GAIN 0.00			// Minimum gain required
#define MAX_OFFSET 250			// Set maximum pixel-meter offset for each split candidate
#define MAX_THRESHOLD 2000		// Set max and minimum threshold for feature comparison (4000mm is the largest distance readable by kinect)
#define NUM_THRESHOLDS 100		// Thresholds per feature pair
#define SPLIT_CANDIDATES 3000 	// Feature pairs
#define TRAIN_PIXELS 3000		// Random pixels per image for classification for tree training
#define RUN_PIXELS 3000			// Random pixels per image for classification for classification
#define MAX_DEPTH 12			// Depth including root

// error metric parameters
#define PROP_DIST 5			// Distance between ground truth joint proposal and classified mode (in pixel-meters)
#define VAL_DIST 20			// Distance between ground truth joint proposal and classified part centre (in pixel-meters)
#define BETA 0.5			// F-score weighting

// spearmint parameters
#define NUM_TEST 35

using namespace cv;

typedef enum {
	NONE = 0,
	WEB,			// Webcam
	KIN,			// Kinect
	DISK			// File
} cameraSource;

typedef enum {
	NIL = 0,
	GT_CENTRE,			// labeling part ground truth centres
	ACTION,				// labeling participant actions
	GT_L_HAND,			// labeling ground truth left hand task step
	GT_R_HAND			// labeling ground truth right hand task step
} annotateModes;

//region of interest within the captured image
typedef struct
{
	int x;	// roi start
	int y;	// roi start
	int w;	// roi width
	int h;	// roi height
} ROI;

typedef struct
{
	Point3_<int> centre;		// region centre in projective coordinates
	float radius;				// radius of region
	string label;				// name of the region
} objectRegion;

typedef struct
{
	int x;		// x position of sample
	int y;		// y position of sample
	int img;	// image number of sample
	int gt;		// ground truth colour label of sample
} imgSample;

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

struct runConfig{

	// forest parameters
	int num_trees;
	vector<string> treePaths;

	// initialization parameters
	cameraSource source;
	string diskpath;

	// annotation parameters
	vector<string> positions;							// vector of user positions, read from config.txt
	vector<string> activities;							// vector of user activities, read from config.txt

};

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

#endif
