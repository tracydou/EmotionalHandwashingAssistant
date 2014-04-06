/*
 *  File:				kinectgrab.h
 *  Created by:			Steve Czarnuch
 *  Created:			January 2013
 *  Last Modified:		January 2013
 *  Last Modified by:	Steve Czarnuch
 */

#ifndef KINGRABBER
#define KINGRABBER

#include "opencv2/opencv.hpp"
#include "defines.h"

#define FRAMES_TO_LEARN 10

using namespace cv;
using namespace std;

class Grabber {
public:

	// constructor: initializes image sizes generically,
	// sets up the camera (if needed) of type typ
	Grabber(cameraSource typ, int width, int height, ROI roi, unsigned int size);

	void setDiskPath(string diskpath, string valParentDir = "");

	// Image capture
	void getRGBImage(Mat &rgbim);
	void getDepthImage(Mat& depthim, Mat& depthmask);
	void getImages(Mat& rgbim, Mat& depthim, Mat& depthmask);
	void getOptimizingImages(Mat& depthim, Mat& labelim, Mat& foreim);
	int convertProjToReal(int nCount, cv::Point3_<float> aProjective[], cv::Point3_<float> aRealWorld[]);
	int convertRealToProj(int nCount, cv::Point3_<float> aRealWorld[], cv::Point3_<float> aProjective[]);

	void fillBgBuffer(Mat& depthim, Mat& depthmask);

	// Background
	void createMask();
	void updateBackground(Mat& depthim, Mat& depthmask);
	//void subtractBG(Mat& depthim, Mat& foreimfull, Mat& foreim, Point3_<int> bgColour);
	void subtractBG(Mat& depthim, Mat& foreim);

	int release();

	cameraSource source;

	// Custom stuff for projective-world coordinate conversion
	// addapted from XnMapOutputMode outputMode;
	int xRes;	// Number of elements in the X-axis
	int yRes;	// Number of elements in the Y-axis
	double fXToZ;
	double fYToZ;

	// read from disk
	string path;					// root path for video files
	string depthPath, labelPath, forePath;	// paths to depth and labeled images
	int num_images;					// number of images to read
	int cur_image;					// current image
	int last_image;					// previous image loaded from disk
	vector<std::string> depthList, labelList;				// vectors to hold list of labeled, foreground and depth images

	// annotate
	vector<string> validateDirs;	// list of directories holding validation images
	string validateParentDir;		// path to parent directory of validation images

	// Background items
	bool background;					// background trained
	Mat bgImage;						// background image (CV_16UC1)
	Mat bgValidMask, bgInvalidMask;		// masks of valid and invalid pixels in background image (CV_16UC1)

	bool isCamera;						// camera is connected
	bool isDepthGenerator;				// depth generator node created

	bool newDepthData;					// new depth data available
	bool newRGBData;					// new RGB data available

private:

	int setupGrabber(cameraSource typ, int ncols, int nrows, ROI roi);
	int readCameraConfig(void);

	// Image capture
	int getRawRGBImage(Mat &rgbim);
	int getRawDepthImage(Mat& depthim, Mat& depthmask);
	int getRawImages(Mat& rgbim, Mat& depthim, Mat& depthmask);
	int getRawOptimizingImages(Mat& depthim, Mat& labelim, Mat& foreim);
	void captureDepth(Mat& depthim, Mat&depthmask);
	void captureRGB(Mat& rgbim);
	int grabConvertProjToReal(int nCount, cv::Point3_<float>* aProjective, cv::Point3_<float>* aRealWorld);
	int grabConvertRealToProj(int nCount, cv::Point3_<float>* aRealWorld, cv::Point3_<float>* aProjective);

	// background processing
	//void subtractBackGrnd(Mat& depthim, Mat& foreimfull, Mat& foreim, Point3_<int> bgColour);
	void subtractBackGrnd(Mat& depthim, Mat& foreim);

	// the size of the image grabbed from device
	int grab_rows, grab_cols;

	// the size of the image after downsample (may be the same as grab_rows,grab_cols)
	int roi_x, roi_y, roi_w, roi_h;

	// frame buffer
	void pushFrame(Mat& depthim, Mat& depthmask);

	int shutDown();

	vector<Mat> depthBuffer;			// buffer of depth frames
	vector<Mat> maskBuffer;				// buffer of valid pixel masks for depth frames
	unsigned int curFrame, nextFrame;
	int maxBufferSize;
	vector<vector<int> > histogram;

	VideoCapture capture;
	XnStatus nRetVal;
	xn::Context context;

};
#endif
