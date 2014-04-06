/*
 *  File:				kinectgrab.cpp
 *  Created by:			Steve Czarnuch
 *  Created:			January 2013
 *  Last Modified:		January 2013
 *  Last Modified by:	Steve Czarnuch
 */


#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "ni/XnCppWrapper.h"
#include "grab.h"
#include "defines.h"

using namespace std;

/*****************************************************************************/
Grabber::Grabber(cameraSource typ, int ncols, int nrows, ROI roi, unsigned int size) : histogram(WIDTH*HEIGHT, vector<int>(256)) {

	if(!setupGrabber(typ, ncols, nrows, roi)){
		cout << "Couldn't initiate grabber type " << typ << ". Trying to initialize an empty grabber." << endl;
		this->source = NONE;
		if(!setupGrabber(this->source, ncols, nrows, roi)){
			cout << "Couldn't initialize empty grabber. Shutting down." << endl;
			exit(0);
		}
	}

	maxBufferSize = size;
	background = 0;					// invalid background
	nextFrame = 0;
	depthBuffer.resize(maxBufferSize);
	maskBuffer.resize(maxBufferSize);
	cur_image = 0;

	readCameraConfig();
}

/*****************************************************************************/
int Grabber::setupGrabber(cameraSource typ, int width, int height, ROI roi) {

	source = typ;
	grab_rows = height;
	grab_cols = width;
	roi_x = roi.x;
	roi_y = roi.y;
	roi_w = roi.w;
	roi_h = roi.h;
	isCamera = 0;
	isDepthGenerator = 0;
	bgImage = Mat::zeros(Size(roi.w,roi.h),CV_16UC1);
	bgValidMask = Mat::zeros(Size(roi.w,roi.h),CV_16UC1);
	bgInvalidMask = Mat::zeros(Size(roi.w,roi.h),CV_16UC1);

	switch (source){
	case WEB:
		cout << "\tInitializing camera..." << endl;

		capture.open(CV_CAP_ANY); // [CV_CAP_ANY to autodetect or CV_CAP_OPENNI] open the default camera
		if(!capture.isOpened())  // check if we succeeded
		{
			// If Kinect is not physically connected, try to create a mock depth generator node
			cerr << "Could not find a camera connected to system. Shutting down." << endl;
			return(1);
		}
		else{
			isCamera = 1;
		}

		cout << "\tCamera initialized." << endl;
		break;
	case KIN:
	{
		cout << "\tInitializing Kinect..." << endl;

		capture.open(CV_CAP_ANY); // [CV_CAP_ANY to autodetect or CV_CAP_OPENNI] open the default camera
		if(!capture.isOpened())  // check if we succeeded
		{
			// If Kinect is not physically connected, try to create a mock depth generator node
			cout << "\tKinect camera not connected to system." << endl;
			return(0);
		}
		else
		{
			isCamera = 1;			// Camera connected and initialized

			// Read depth generator sensor configuration data

			// OpenNI stuff for projective-world coordinate conversion
			xn::DepthGenerator depth;

			// Initialize context object
			if( context.Init() != XN_STATUS_OK){
				cout << "\tFailed to initialize context object." << endl;
			}

			// Create a depth generator node
			if(depth.Create(context) != XN_STATUS_OK){
				cout << "\tFailed to create depth generator node." << endl;
				return(1);
			}
			else{
				cout << "\tDepth generator node created." << endl;
				isDepthGenerator = 1;
			}

			// Capture sensor confguration parameters for future use with conversion between projective and real-world coordinates
			XnMapOutputMode depthMapMode;
			depth.GetMapOutputMode(depthMapMode);

			xRes = depthMapMode.nXRes;
			yRes = depthMapMode.nYRes;

			XnFieldOfView FOV;
			depth.GetFieldOfView(FOV);

			fXToZ = tan(FOV.fHFOV/2)*2;
			fYToZ = tan(FOV.fVFOV/2)*2;

			cout << "\tKinect initialized." << endl;
		}

	}
	break;
	case DISK:
		cout << "\tInitializing grabber to read from disk." << endl;

		// initialize camera configuration parameters
		xRes = -1;
		yRes = -1;
		fXToZ = -1;
		fYToZ = -1;

		this->isCamera = 1;

		break;
	case NONE:
		cout << "\tInitializing empty grabber." << endl;

		// initialize camera configuration parameters
		xRes = -1;
		yRes = -1;
		fXToZ = -1;
		fYToZ = -1;

		this->isCamera = 0;

		cout << "\tEmpty grabber initialized. Note that this doesn't do anything..." << endl;
		break;
	default:
		cout << "No source defined for setupGrabber. Shutting down." << endl;
		shutDown();
		break;
	}
	return(1); // successfully initialized
}
/*****************************************************************************/

void Grabber::setDiskPath(string diskpath, string valParentDir){
	if (diskpath != ""){
		if(VERBOSE)
			cout << "\tSetting paths." << endl;
		this->path = diskpath;
		this->depthPath = diskpath + "/depth16bit";
		if(valParentDir != "")
			this->validateParentDir = valParentDir;
	}
	if(this->source == DISK)
		cout << "\tGrabber initialized to read images from " << this->path << endl;

	this->cur_image = 0;
	this->last_image = -1;

}

void Grabber::getRGBImage(Mat &rgbim) {
	getRawRGBImage(rgbim);
}

/*****************************************************************************/

// returns 0 on failure
int Grabber::getRawRGBImage( Mat &rgbim) {
	// get new image

	switch (source) {
	case WEB:
	case KIN:{

		captureRGB(rgbim);
		cur_image ++;
	}
	break;
	default:
		cout << "No source defined for getRawRGBImage. Shutting down..." << endl;
		shutDown();
		exit(0);
		return 0;
	}

	return 1;
}
/*****************************************************************************/

void Grabber::getDepthImage(Mat& depthim, Mat& depthmask) {
	getRawDepthImage(depthim, depthmask);
}
/*****************************************************************************/

// returns 0 on failure
int Grabber::getRawDepthImage(Mat& depthim, Mat& depthmask) {
	// get new image

	switch (source) {
	case WEB:{
		cout << "Why are you grabbing a depth image with an RGB camera?" << endl;

	}
	break;
	case KIN:{
		if( !capture.grab() )
		{
			cout << "Cannot grab images." << endl;
			return -1;
		}
		else
		{
			captureDepth(depthim, depthmask);
			cur_image ++;
		}
	}
	break;
	case DISK:{
		// loop to beginning if no more images are available
		if (cur_image == num_images)
			cur_image = 0;

		string depthFile = this->depthPath + "/" + depthList[cur_image];
		depthim = imread(depthFile.data(),CV_LOAD_IMAGE_UNCHANGED);
		cur_image++;
	}
	break;
	default:
		cout << "No source defined for getRawDepthImage. Shutting down..." << endl;
		shutDown();
		exit(0);
		return 0;
	}

	return 1;
}
/*****************************************************************************/

void Grabber::getOptimizingImages(Mat& depthim, Mat& labelim, Mat& foreim) {
	getRawOptimizingImages(depthim, labelim, foreim);

}
/*****************************************************************************/

// returns 0 on failure
int Grabber::getRawOptimizingImages(Mat& depthim, Mat& labelim, Mat& foreim) {
	// get new image

	switch (source) {

	case DISK:{
		string depthFile = this->depthPath + "/" + depthList[cur_image];
		string labelFile = this->labelPath + "/" + labelList[cur_image];
		depthim = imread(depthFile.data(),CV_LOAD_IMAGE_ANYDEPTH);		// read image as grayscale image
		labelim = imread(labelFile.data(),CV_LOAD_IMAGE_UNCHANGED);
		cur_image++;
	}
	break;
	default:
		cout << "Invalid source defined for getRawOptimizingImages. Shutting down." << endl;
		shutDown();
		exit(0);
		return 0;
	}

	return 1;
}
/*****************************************************************************/

void Grabber::getImages(Mat& rgbim, Mat &depthim, Mat& depthmask) {
	getRawImages(rgbim, depthim, depthmask);
}
/*****************************************************************************/

int Grabber::getRawImages(Mat& rgbim, Mat &depthim, Mat& depthmask) {
	// get new image

	switch (source) {
	case WEB:{
		captureRGB(rgbim);

	}
	break;
	case KIN:{

		if( !capture.grab() )
		{
			cout << "Cannot grab images." << endl;
			return -1;
		}
		else
		{
			captureDepth(depthim, depthmask);
			captureRGB(rgbim);
			cur_image ++;
		}
	}
	break;
	case DISK:{
		// loop to beginning if no more images are available
		if (cur_image == num_images)
			cur_image = 0;

		if(cur_image != last_image){
			string depthFile = path + "/depth16bit/" + depthList[cur_image];
			depthim = imread(depthFile.data(),-1);

			// START
			// sanctioned hack to display RGB images during annotation
			//TODO remove this hack

			string path2 = path;
			string file2 = depthList[cur_image];

			string newfile = file2.replace(file2.find("depth16bit"), 10, "rgb");
			string rgbFile = path2.replace(path2.find("validation_images"), 17, "trials") + "/camera/rgb/" + file2;

			rgbim = imread(rgbFile.data(),1);  	// Load as 3 channel colour image

			// end sanctioned hack
			// END

			last_image = cur_image;
			this->newDepthData = TRUE;
		}
	}
	break;
	default:
		cout << "No source defined for getRawImages. Shutting down..." << endl;
		shutDown();
		exit(0);
	}

	return 1;
}
/*****************************************************************************/

void Grabber::captureDepth(Mat& depthim, Mat& depthmask){
	Mat depthMap;
	bool newDepth = FALSE;
	bool newMask = FALSE;
	if( capture.retrieve( depthim, CV_CAP_OPENNI_DEPTH_MAP )){				// CV_16UC1
		newDepth = TRUE;
	}
	else
		cout << "Could not capture depth image." << endl;

	Mat mask;
	if( capture.retrieve( mask, CV_CAP_OPENNI_VALID_DEPTH_MASK )){			// CV_8UC1
		mask.convertTo(depthmask, CV_16UC1);					// CV_8UC1
		newMask = TRUE;
	}
	else
		cout << "Could not capture depth image valid pixel mask." << endl;

	if(newDepth && newMask)
		this->newDepthData = TRUE;
	else{
		this->newDepthData = FALSE;
		cout << "Failed to capture new depth data." << endl;
		exit(0);
	}

}
/*****************************************************************************/

void Grabber::captureRGB(Mat& rgbim){
	if( capture.retrieve( rgbim, CV_CAP_OPENNI_BGR_IMAGE )){				// CV_8UC3
		// do nothing
	}
	else
		cout << "Could not capture RGB image." << endl;
}
/*****************************************************************************/

// push current depth image and valid pixel mask onto buffers
void Grabber::pushFrame(Mat& depthim, Mat& depthmask){

	curFrame = nextFrame;
	depthim.copyTo(depthBuffer[curFrame]);
	depthmask.copyTo(maskBuffer[curFrame]);
	nextFrame = (curFrame + 1) % maxBufferSize;
}
/*****************************************************************************/

// Create background mask from buffered depth images.
// Create Mat of valid pixels in background mask.
void Grabber::createMask() {

	Mat depth32FC1(Size(grab_cols,grab_rows), CV_32FC1);
	Mat mask16UC1(Size(grab_cols,grab_rows), CV_16UC1);
	Mat depthAccum = Mat::zeros(Size(grab_cols,grab_rows), CV_32FC1);
	bgValidMask = Mat::zeros(Size(grab_cols,grab_rows), CV_16UC1);

	// Element-wise addition of all buffered background images. Only pixels that are valid (based on valid depth mask)
	// are accumulated. Final mask is the element-wise average of the valid pixels.
	for (unsigned int i = 0; i < depthBuffer.size(); i++){

		depthBuffer[i].convertTo(depth32FC1, CV_32FC1);				// convert depth image to 16UC1
		add (depth32FC1, depthAccum, depthAccum);					// add depth image to the accumulator

		divide(maskBuffer[i],maskBuffer[i],mask16UC1);				// normalize elements of the mask
		add (mask16UC1, bgValidMask, bgValidMask);					// add normalized mask to the mask accumulator
	}

	// convert mask accumulator to 16UC1
	Mat avgMask32 (cv::Size(WIDTH,HEIGHT),CV_32FC1);
	bgValidMask.convertTo(avgMask32,CV_32FC1);

	// average the depth accumulator for each individual pixel
	Mat mask32 (cv::Size(WIDTH,HEIGHT),CV_32FC1);
	divide(depthAccum,avgMask32, mask32);

	// Normalize valid mask and then set all valid pixels to 255 (invalid will be zero)
	divide(bgValidMask, bgValidMask, bgValidMask);
	multiply(bgValidMask, Scalar::all(65535), bgValidMask);

	// Set all invalid pixels to 255 (valid will be zero)
	subtract(Scalar::all(65535), bgValidMask, bgInvalidMask);

	// convert to 8UC1 and store in mask
	mask32.convertTo(bgImage, CV_16UC1);

	if(VERBOSE)
		cout << "Background mask created from " << depthBuffer.size() << " frames." << endl;

	this->background = 1;
}
/*****************************************************************************/

/* Adapted from inline XnStatus ConvertProjectiveToRealWorld(XnUInt32 nCount, const XnPoint3D aProjective[], XnPoint3D aRealWorld[]) const
 * found in XnCppWrapper.h
 * Note that the depth here must be in mm, not m
 */
int Grabber::convertProjToReal(int nCount, cv::Point3_<float> aProjective[], cv::Point3_<float> aRealWorld[]){
	return grabConvertProjToReal(nCount, aProjective, aRealWorld);
}
/*****************************************************************************/

/* Adapted from XnStatus xnConvertProjectiveToRealWorld(XnNodeHandle hInstance, XnUInt32 nCount, const XnPoint3D* aProjective, XnPoint3D* aRealWorld)
 * found in XnOpenNi.cpp
 */
int Grabber::grabConvertProjToReal(int nCount, cv::Point3_<float>* aProjective, cv::Point3_<float>* aRealWorld){

	for (int i = 0; i < nCount; ++i)
	{
		double fNormalizedX = (aProjective[i].x / xRes - 0.5);
		aRealWorld[i].x = (float)(fNormalizedX * aProjective[i].z * fXToZ);

		double fNormalizedY = (0.5 - aProjective[i].y / yRes);
		aRealWorld[i].y = (float)(fNormalizedY * aProjective[i].z * fYToZ);

		aRealWorld[i].z = aProjective[i].z;
	}

	return (1);
}
/*****************************************************************************/

/* Adapted from inline XnStatus ConvertRealWorldToProjective(XnUInt32 nCount, const XnPoint3D aRealWorld[], XnPoint3D aProjective[]) const
 * found in XnCppWrapper.h
 * Note that this returns the depth in mm, not m
 */
int Grabber::convertRealToProj(int nCount, cv::Point3_<float> aRealWorld[], cv::Point3_<float> aProjective[]){
	return grabConvertRealToProj(nCount, aRealWorld, aProjective);
}
/*****************************************************************************/

/* Adapted from XnStatus xnConvertProjectiveToRealWorld(XnNodeHandle hInstance, XnUInt32 nCount, const XnPoint3D* aProjective, XnPoint3D* aRealWorld)
 * found in XnOpenNi.cpp
 */
int Grabber::grabConvertRealToProj(int nCount, cv::Point3_<float>* aRealWorld, cv::Point3_<float>* aProjective){

	double fCoeffX = xRes / fXToZ;
	double fCoeffY = yRes / fYToZ;

	// we can assume resolution is even (so integer div is sufficient)
	double nHalfXres = xRes / 2;
	double nHalfYres = yRes / 2;

	for (int i = 0; i < nCount; ++i)
	{
		aProjective[i].x = (float)fCoeffX * aRealWorld[i].x / aRealWorld[i].z + nHalfXres;
		aProjective[i].y = nHalfYres - (float)fCoeffY * aRealWorld[i].y / aRealWorld[i].z;
		aProjective[i].z = aRealWorld[i].z;
	}

	return (1);
}

/*****************************************************************************/


// fill background buffer with depth images and valid pixle mask
void Grabber::fillBgBuffer(Mat& depthim, Mat& depthmask){

	if(VERBOSE)
		cout << "Capturing " << maxBufferSize << " images into initial buffer." << endl;

	int bufferSize = 0;
	// fill up background buffer
	while( bufferSize < this->maxBufferSize ){
		getDepthImage(depthim, depthmask);
		pushFrame(depthim, depthmask);
		bufferSize++;
	}

	if(VERBOSE)
		cout << "Initial background buffer ready." << endl;
}
/*****************************************************************************/

void Grabber::updateBackground(Mat& depthim, Mat& depthmask){
	pushFrame(depthim, depthmask);
}
/*****************************************************************************/

//void Grabber::subtractBG(Mat& depthim, Mat& foreimfull, Mat& foreim, Point3_<int> bgColour){
void Grabber::subtractBG(Mat& depthim, Mat& foreim){

	//subtractMask(depthim, foreim);	//background subtraction, threshold
	subtractBackGrnd(depthim, foreim);
}
/*****************************************************************************/

// Subtract the background mask
void Grabber::subtractBackGrnd(Mat& depthim, Mat& foreim){

	Mat depthFore16bit (Size(WIDTH, HEIGHT), CV_16UC1);		// final 16-bit depth foreground image
	Mat depthFore8bit (Size(WIDTH, HEIGHT), CV_8UC1);		// final 8-bit depth foreground image (for labeling)
	Mat normalFore (Size(WIDTH, HEIGHT), CV_16UC1);
	Mat rawFore, normDepthMask16, normDepthMask8, blurFore, blurThresh, threshForeContours, raw_thresh, blobs, bgImInvPixel8bit;

	bgInvalidMask.convertTo(bgImInvPixel8bit, CV_8UC1, 1.0/256.0);

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	// Find foreground image.
	// Note that depth image is subtracted from background image because pixels further from the camera have a larger value.
	// Two issues must still be resolved with the foreground image:
	// 1. If the background image had any invalid pixels (value = zero), the raw foreground image will have a zero value as well
	// regardless of the pixel state in depth image.
	// 2. The raw foreground image has also overwritten any invalid pixels in the depth image that were invalid.

	subtract(bgImage, depthim, rawFore);

	// Deal with Issue 1: background image invalid pixels
	// Add the depth image to the raw foreground, masking for invalid background pixels only. This will overwrite any invalid
	// background image pixels that are valid in the depth image.
	add(rawFore, depthim, rawFore, bgImInvPixel8bit);

	// Deal with issue 2: depth image invalid pixels
	// Normalize the depth image valid pixel mask (valid = 1, invalid = 0); Multiply the foreground by normalized mask to set all invalid pixels to zero.

	divide(depthim,depthim, normDepthMask16);
	multiply(rawFore, normDepthMask16, rawFore);

	// blur salt and pepper noise
	medianBlur(rawFore,blurFore,1);

	// Entering the 8-bit world to be able to use cv::threshold and cv::findContours, because both require 8-bit images

	// Note that we do the full conversion from 0 to 4m here because of the threshold operation
	blurFore.convertTo(blurFore, CV_8UC1, OPENNI_SCALE);

	// threshold blurred image to remove pixels with a small amount of difference from the background image (attributable to sensor noise) or too close to sensord
	// Note that this creates a foreground "blob" with anything in the foreground set to 255
	// TODO this threshold is not working well.
	threshold(blurFore,blurThresh,3,255, THRESH_BINARY);

	// copy to new Mat because findContours destroys original Mat
	blurThresh.copyTo(threshForeContours);

	// Find contours
	findContours( threshForeContours, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

	// initialize a new image for blobs
	blobs = Mat::zeros( Size(WIDTH, HEIGHT), CV_8UC1 );

	// Draw all small contours
	for(unsigned int i = 0; i< contours.size(); i++ )
	{
		/*when using contours, see http://opencvpython.blogspot.ca/2013/01/contours-5-hierarchy.html for good information.
		 * The hierarchy returned by findContours has the following form:
		 * hierarchy[idx][{0,1,2,3}]={next contour (same level), previous contour (same level), child contour, parent contour}
		 * CV_RETR_CCOMP, returns a hierarchy of outer contours and holes. This means elements 2 and 3 of hierarchy[idx] have at most one of these not equal to -1:
		 * that is, each element has either no parent or child, or a parent but no child, or a child but no parent.
		 * An element with a parent but no child would be a boundary of a hole.
		 */

		if (contourArea(contours[i]) < 150){
			// hierarchy[j][3]!=-1 inner hole boundaries (any boundary that has a parent is a hole)
			//if(hierarchy[i][2] != -1)	//  outer boundaries (any boundary that has a child is an external contour)
			//{
			Scalar colour = Scalar(255);
			drawContours( blobs, contours, i, colour, CV_FILLED, 8, hierarchy);
			//}
		}
	}

	// remove all small contours from foreground image (noise)
	subtract(blurThresh, blobs, foreim);

	// Back to the 16-bit world

	// convert the foreground mask to 16-bit
	//foreim.convertTo(normalFore, CV_16UC1);

	// normalize foreground image
	//divide(normalFore,normalFore,normalFore);

	// mask the depth image by normalized foreground mask to create the final, 16-bit depth foreground image
	//multiply(normalFore, depthim, depthFore16bit);

	// 8-bit conversion between MIN_X and MAX_X to maximize 8-bit depth resolution
	//depthFore16bit.convertTo(depthFore8bit, CV_8UC1, OPENNI_ALPHA, OPENNI_BETA);

	// create a three channel foreground output image
	//Mat channels[] = {depthFore8bit, depthFore8bit, depthFore8bit};
	//merge(channels,3,foreimfull);

	// set all "background" pixels to the BACKGROUND colour. Anything on the background, as well as invalid pixels, will have a value of zero.
	//Mat3b src = foreimfull;

	//for (Mat3b::iterator it = src.begin(); it != src.end(); it++) {
	//	if (*it == Vec3b(0, 0, 0)) {
	//		*it = Vec3b(bgColour.x, bgColour.y, bgColour.z);
	//	}
	//}

	// Set all invalid pixels back to 0
	//normDepthMask16.convertTo(normDepthMask8, CV_8UC1);
	//Mat normDepthMask3ch;
	//Mat in[] = {normDepthMask8, normDepthMask8, normDepthMask8};
	//merge(in, 3, normDepthMask3ch);

	//multiply(foreimfull, normDepthMask3ch, foreimfull);

}

/*****************************************************************************/

int Grabber::readCameraConfig(void){

	return(XN_STATUS_OK);

}

/*****************************************************************************/

int Grabber::release(){

	cout << "Releasing current grabber." << endl;
	shutDown();
	return 1;
}

int Grabber::shutDown() {
	switch ( source ){
	case WEB:
	case KIN:
		context.Release();
		if (capture.isOpened())
			capture.release();
		cout << "Camera shut down." << endl;
		break;
	case DISK:
		context.Release();
		break;
	default:
		cout << "No source specified. Can't shut down camera..."  << endl;
		break;
	}
	return 1;
}
/*****************************************************************************/

