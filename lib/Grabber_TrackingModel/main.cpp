/*
 *  File:				main.cpp
 *  Created by:			Steve Czarnuch
 *  Created:			January 2013
 *  Last Modified:		January 2013
 *  Last Modified by:	Steve Czarnuch
 */

#include "tracker.hpp"

using namespace std;
using namespace cv;

// global variables
ROI roi={0, 0, WIDTH, HEIGHT};


trackingModel* trackModel = new trackingModel;					// model to hold tracking information
Grabber* grabber(NULL);											// grabber model for image aquisition
DTree** forest;													// tree class for classification model
runConfig runConf;												// run configuration model

// Raw images
Mat rgbIm (cv::Size(WIDTH,HEIGHT),CV_8UC3);
Mat depthIm (cv::Size(WIDTH,HEIGHT),CV_16UC1);


// Processed images
Mat foreIm	(cv::Size(WIDTH,HEIGHT), CV_8UC1);			// binary foreground image (without background separated from invalid pixels)
Mat depthMask (cv::Size(WIDTH,HEIGHT),CV_8UC1);			// mask of valid depth pixels

// Kalman Filter
Filter** KFilters;

// GTK setup
GtkWindow *window;											// main window
GtkWidget *mainWindow, *mainvbox, *hbox, *hbox2, *vbox;		// boxes
GtkWidget *button, *runButton, *sourceButton, *check, *annotateButton, *setRegions;		// buttons
GtkWidget *arrowR;											// arrow
GtkWidget *table;											// table
GtkWidget *label, *gtkframe, *diskpath, *annotateDirNum;	// misc
GtkWidget *gtkLImage, *gtkRImage;							// images
GtkWidget *partEntry;										// body part selection
GtkWidget *scalebutton;										// slider
GtkWidget *partClass;										// whether or not to classify a part
GSList *group;												// radio button group
GtkWidget *statusBar;										// a status bar...
gint context_id;											// context id for status bar
gint idleTag;												// tag for removing idle (e.g., when training a tree)
GtkEntryBuffer** treebuffer;								// textbox entries for tree paths
GTimer* runTimer;											// Timer
gulong acc_us;												// Accumulated usecs
double acc_s, last_acc_s;									// Accumulated seconds

int lImg = 0;												// offset of image to display in left image window
int rImg = 0;												// offset of image to display in right image window
int curLeftStep = 0;										// current step left hand is completing
int curRightStep = 0;										// current step right hand is completing
vector<int> gtkBodyPart;									// list of body parts available for annotation
unsigned int gtkBodyPartOffset = 0;							// active offset in list of body parts available for annotation
unsigned int gtkAction = 0;									// participant position/action in washroom
unsigned int gtkStep = 0;									// task step participant is currently completing


// Training setup
vector<split_cond> candidates;
vector<int> thresholds;

string projectPath, trialPath;

// Training setup - command line arguments
int offset, threshMagnitude;
float cutoffGain, maxDepth;									// tree training parameters
//float s_band;												// spearmint bandwidth
//int s_part;													// spearmint part to optimize
int spearmint = 0;											// optimize using spearmint

// General setup
int running = 0;											// running trials
int forestLoaded = 0;										// status of decision forest
bool annotating = 0;										// annotating
bool setRegs = 0;											// enable/disable region setting
int curRegionToSet = 0;										// current region
bool regionSetMode = 0;										// 0 = centre, 1 = radius
annotateModes annotateMode = ACTION;						// annotate mode
int annotateDir = 0;										// current directory in set of validation images

bool saveRGB(false);										// toggle state for saving RGB images during run
bool saveDepth(false);										// toggle state for saving depth images during run
bool saveDMask(false);										// toggle state for saving depth mask images during run
bool saveFore(false);										// toggle state for saving foreground images during run
bool saveSkel(false);										// toggle state for saving skeleton images during run
bool enableFilters(true);									// toggle state for enabling/disabling Kalman Filters
bool allModes(false);										// toggle state for showing all modes or just the most confident joint proposal
bool allSamples(false);										// toggle state for showing all samples
bool dispRegions(false);									// toggle state for showing configured regions

int partSelect = 0;											// body part selected through GUI
float scaleValue = 0;										// value from scale button

/***************************************************************************
 *
 * Function prototypes
 *
 **************************************************************************/

// Gtk
gboolean gtkOnKeyPress (GtkWidget *widget, GdkEventKey *event, gpointer user_data); 	// keypress events
static gboolean gtkButtonPressEvent( GtkWidget *widget, GdkEventButton *event );	// mouse button press event
static void gtkQuit( GtkWidget *widget, gpointer data );			// Quit
static void gtkSwitchSource ( GtkWidget *widget, gpointer data);	// Switch between run modes
static void gtkChangeImage ( GtkWidget *widget, gpointer data );	// Manually change image
static void gtkChangeAnnotationDir ( GtkWidget *widget, gpointer data ); // increment or decrement the annotation directory containing validation images
static void gtkRun ( GtkWidget *widget, gpointer data );			// Start COACH running
static void gtkSetDiskPath ( GtkWidget *widget, gpointer data );	// Set path to files from disk
static string gtkGetDirPath ( string title );						// Dialog to get directory path
static string gtkGetFilePath ( string title );						// Dialog to get file path
static void gtkImageRadioCallback( GtkButton *b, gpointer data );			// Generic radio button callback
static void gtkKalmanFiltersCallback( GtkButton *widget, gpointer data );	// toggle enable/disable of Kalman Filters
static void gtkAllModesCallback( GtkButton *widget, gpointer data );		// toggle display of mean-shift modes
static void gtkAllSamplesCallback( GtkButton *widget, gpointer data );		// toggle display of randomly generated samples
static void gtkDispRegions( GtkButton *widget, gpointer data );				// display currently configured regions
static void gtkPartClassifyCallback( GtkButton *widget, gpointer data );	// toggle which parts are classified
static void gtkScaleChangeCallback( GtkButton *widget, gpointer data );		// change bandwidth for mode find for selected body part
static void gtkPartSelectCallback( GtkComboBox *widget, gpointer data );	// select a body part
static void gtkSaveCallback (GtkWidget *widget, gpointer data);
static void gtkTrainTree( GtkWidget *widget, gpointer data );
//static int gtkLoadForest( GtkWidget *widget, gpointer data );
static void gtkAnnotationMode( GtkWidget *widget, gpointer data );						// enter annotation mode to select part centres from depth images
static void gtkForestPerClassMetrics( GtkWidget *widget, gpointer data );				// test a tree/forest for raw classification accuracy
static void gtkForestPartPropMetrics( GtkWidget *widget, gpointer data );				// test a tree/forest for part proposal accuracy
static void gtkValidationMetrics( GtkWidget *widget, gpointer data );					// test a tree/forest for part proposal accuracy accross validation image sets
static void gtkSetRegions( GtkWidget *widget, gpointer data );							// set runtime regions
static void gtkPushStatus( GtkWidget *widget, string message, gint data );				// push item onto the status bar
static int gtkMakeLabelImages( GtkWidget *widget, string message, gint data );			// create a set of 3-channel depth images for labeling
static void gtkCreateBackground( GtkWidget *widget, string message, gint data );		// create background image (16-bit) from a set of depth images
static void gtkRemoveBackground( GtkWidget *widget, string message, gint data );		// remove background image from depth images in a directory
static void gtkMakeDepthImages( GtkWidget *widget, string message, gint data );			// create a set of 3-channel depth images for labeling
static gboolean gtkDeleteEvent( GtkWidget *widget, GdkEvent  *event, gpointer   data );
static void gtkDestroy( GtkWidget *widget, gpointer   data );

// General
int listFiles (string dir, vector<string>& files, string extension);					// list files of type "extension" in a single directory
int listDirectories (string sourcedir, vector<string>& dirs);				// list directories within a directory
int nextTrial (void);
int isBackground (Point pt, Mat label);
int colourFeature (int tree, Point pt, string file, Mat rgbImage);
int depthFeature (int tree, Point pt, split_cond split, Mat rgbImage, Mat depthMap);
int createCandidates (vector<split_cond>& cand);
int createThresholds (vector<int> & thresh);
int createSamples (vector<Point>& pix, int number);
int loadForest ();
void loadConfig (string file);								// load configuration file
int loadCamConfig (string file);							// load camera configuration file
int saveCamConfig(string file);								// save camera configuration file
int loadRegions (string file);								// load region configuration file
int saveRegions (string file);								// save region configuration file
int updateConfig(string file, string parameter, string value);
int setTreeConfig ();
Point3_<uchar> partToColour(DTree* checkTree, int labelIndex);
void showImage(GtkWidget* imwin, Mat& image);
string buildTrainMessage();
string trialDate(void);
bool inSpheriod(Point3_<float> &A, Point3_<float> &B, float radius);			// check if a point in inside a spheroid


// Running
void initGrabber(cameraSource typ, int ncols, int nrows, ROI roi, unsigned int size);						 			// initialize a new Grabber of type typ
int loadDepthImageDirectory();																							// initialize depth image directory
int initBackground(const string path = "");																				// create/load background image
void refreshImageWindows();																								// update GUI image windows
void updateGrabber(bool loadGT, string valParentDir = "");																// update disk path and image list, with optional ground truth file
void drawForegroundOutline(Mat& foreImage, Mat& outImage);																// draw the foreground image outline on an image
void drawPartProposals(Mat& outImage);																					// draw all modes for each part or final proposal on an image
void drawPartPoints(vector<vector<Point3_<float> > >& gtPartCentres, Mat& outputImage);									// draw a general point for each part on an image
void drawSamples(Mat& outImage);																						// draw the randomly proposed samples onto a 3-channel image
void drawRegions(Mat& outImage);																						// draw currently configured regions onto a 3-channel image
void highlightRegions(Mat& outImage);
float distance3D(Point3_<float> &p1, Point3_<float> &p2);

int classifyParts(int numTrees, Mat& foreim, Mat &depthMap);						// produce intermediate classification of a set of sample pixels
void modeFind(Mat& depthMap);														// find the modes of intermediate classification
void filterPartProposals();															// temporal filtering of part proposals
void findAction(Point3_<int> LHandPosition, Point3_<int> RHandPosition, Point& rawHandAction, Point& conditionedHandAction);		// find the hand washing task action each hand is completing
void savePng(string dir, int frame, Mat& outimage);

// Validation
int forestPerClassMetrics(string dirCheck, int num_trees, int tree_num = -1, int debug = 0);
int forestPartPropMetrics(string dirCheck, int num_trees, int debug = 0);
int loadGroundTruthCentres(string path);																	// load ground truth part centres from file
int saveGroundTruthPartCentres(string path, string title, vector<string>& depthList);						// save ground truth part centres to file
int loadActions (vector<Point>& handActions, string path);													// load hand actions from file
int saveActions (vector<Point>& rawHandActions, vector<Point>& condHandActions, vector<vector<Point3_<float> > >& partCentres, string path, vector<string>& depthList);	// save classified hand positions and hand actions from file
int groundTruthPartCentresFromLabel(string path, vector<Mat>& labelim, vector<Mat>& depthim, vector<string>& labellist);
void validationPartPropMetrics(vector<string>& output, vector<vector<Point3_<float> > >& partCentres);
void initGroundTruthPartCentres();																			// initialize empty ground truth part centres
void changeAnnotationMode();
void changeAnnotationDir(int index);

// Image manipulation
int createBackground( string rootpath);			// create background image (16-bit) from a set of depth images
int removeBackground( string rootpath, string targetpath);			// remove background image from depth images in a directory
int makeLabelImages( string rootpath, string sourcepath);			// create a set of 3-channel depth images for labeling

// Training
int trainTree(string dirTrain = "", string dirCheck = "");
//int checkPartAccuracy(Mat& labelim, Mat& depthim, vector<Point3_<int> > partCentres, vector<float> partConf, float maxDist, int frame);

// Main
void usage();
gint mainIdle(void* data);

// decision tree training thread parameters
struct trainParams{
	float maxGain;
	vector<Point> lPixels, rPixels; 				// vars for the split candidates/threshold pair with the highest gain
	Mat labels;										// label histogram for storage in the node - based on the split_candidates/threshold pair with the largest gain
	int candPos, threshPos;
	int firstCand, lastCand;						// first and last split candidates for thread to process
};

/***************************************************************************
 *
 * GTK Callback functions
 *
 **************************************************************************/

gboolean gtkOnKeyPress (GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{

	switch (event->keyval)
	{
	case GDK_q:
	case GDK_Q:
		gtkQuit(mainWindow, NULL);
		break;
	case GDK_Right:
	case GDK_d:
	case GDK_c:


		if(annotating && !setRegs)
		{
			gtkChangeImage(widget,gpointer(1));			// index to the next image

			if(trackModel->classHandAction.size() != 0){		// classified hand positions were loaded from file
				curLeftStep = trackModel->classHandAction[grabber->cur_image].x;
				curRightStep = trackModel->classHandAction[grabber->cur_image].y;
			}
			else{
				curLeftStep = 0;
				curRightStep = 0;
			}

			string title = "created by gtkAnnotationMode for use with manual part centre annotation.";
			saveGroundTruthPartCentres(runConf.diskpath, title, grabber->depthList);

			if(annotateMode == GT_CENTRE){								// ground truth centre mode
				gtkBodyPartOffset = 0;
			}
			else if (annotateMode == ACTION){						// participant position in washroom mode
				// update with the same value as the previous frame if annotating for the first time
				if(trackModel->gtAction[grabber->cur_image] == 0)
					trackModel->gtAction[grabber->cur_image] = gtkAction;

			}
			else if (annotateMode == GT_L_HAND){						// participant step in handwashing task
				// update with the same value as the previous frame if annotating for the first time
				if(trackModel->gtHandPos[grabber->cur_image].x == 0 && trackModel->gtAction[grabber->cur_image] == 2)
					trackModel->gtHandPos[grabber->cur_image].x = gtkStep;
			}
			else if (annotateMode == GT_R_HAND){						// participant step in handwashing task
				// update with the same value as the previous frame if annotating for the first time
				if(trackModel->gtHandPos[grabber->cur_image].y == 0 && trackModel->gtAction[grabber->cur_image] == 2)
					trackModel->gtHandPos[grabber->cur_image].y = gtkStep;
			}
		}
		break;
	case GDK_e:
		if(setRegs){
			regionSetMode = 0; // setting the centre of the current region
		}
		break;
	case GDK_r:
		if(setRegs){
			regionSetMode = 1; // setting the radius of the current region
		}
		break;
	case GDK_Left:
	case GDK_a:
	case GDK_x:

		if(annotating &&  !setRegs)
		{
			gtkChangeImage(widget,gpointer(-1));		// index to the previous image
			if(trackModel->classHandAction.size() != 0){		// classified hand positions were loaded from file
				curLeftStep = trackModel->classHandAction[grabber->cur_image].x;
				curRightStep = trackModel->classHandAction[grabber->cur_image].y;
			}
			else{
				curLeftStep = 0;
				curRightStep = 0;
			}

			string title = "created by gtkAnnotationMode for use with manual part centre annotation.";
			saveGroundTruthPartCentres(runConf.diskpath, title, grabber->depthList);

			if(annotateMode == GT_CENTRE){								// ground truth centre mode
				gtkBodyPartOffset = 0;
			}
			else if (annotateMode == ACTION){						// participant action in washroom mode
				// update with the same value as the previous frame if annotating for the first time
				if(trackModel->gtAction[grabber->cur_image] == 0)
					trackModel->gtAction[grabber->cur_image] = gtkAction;

			}
			else if (annotateMode == GT_L_HAND){						// participant step in handwashing task
				// update with the same value as the previous frame if annotating for the first time
				if(trackModel->gtHandPos[grabber->cur_image].x == 0 && trackModel->gtAction[grabber->cur_image] == 2)
					trackModel->gtHandPos[grabber->cur_image].x = gtkStep;
			}
			else if (annotateMode == GT_R_HAND){						// participant step in handwashing task
				// update with the same value as the previous frame if annotating for the first time
				if(trackModel->gtHandPos[grabber->cur_image].y == 0 && trackModel->gtAction[grabber->cur_image] == 2)
					trackModel->gtHandPos[grabber->cur_image].y = gtkStep;
			}
		}
		break;
	case GDK_0:
		if(annotating && !setRegs){
			if(annotateMode == GT_CENTRE){								// ground truth centre mode
				// do nothing
			}
			else if (annotateMode == ACTION){						// participant position in washroom mode
				gtkAction = 0;
				trackModel->gtAction[grabber->cur_image] = gtkAction;
			}

			else if (annotateMode == GT_L_HAND){						// participant step in handwashing task
				gtkStep = 0;
				trackModel->gtHandPos[grabber->cur_image].x = gtkStep;
			}
			else if (annotateMode == GT_R_HAND){						// participant step in handwashing task
				gtkStep = 6;
				trackModel->gtHandPos[grabber->cur_image].y = gtkStep;
			}
		}
		else if(setRegs){
			curRegionToSet = 0;
		}
		break;
	case GDK_1:
		if(annotating  && !setRegs){
			if(annotateMode == GT_CENTRE){								// ground truth centre mode
				gtkBodyPartOffset = 0;							// annotating L_HAND
			}
			else if (annotateMode == ACTION){						// participant position in washroom mode
				gtkAction = 1;
				trackModel->gtAction[grabber->cur_image] = gtkAction;
			}
			else if (annotateMode == GT_L_HAND){						// participant step in handwashing task
				gtkStep = 1;
				trackModel->gtHandPos[grabber->cur_image].x = gtkStep;
			}
			else if (annotateMode == GT_R_HAND){						// participant step in handwashing task
				gtkStep = 6;
				trackModel->gtHandPos[grabber->cur_image].y = gtkStep;
			}
		}
		else if(setRegs){
			curRegionToSet = 1;
		}
		break;
	case GDK_2:
		if(annotating && !setRegs){
			if(annotateMode == GT_CENTRE){								// ground truth centre mode
				gtkBodyPartOffset = 1;							// annotating R_HAND
			}
			else if (annotateMode == ACTION){						// participant position in washroom mode
				gtkAction = 2;
				trackModel->gtAction[grabber->cur_image] = gtkAction;
			}
			else if (annotateMode == GT_L_HAND){						// participant step in handwashing task
				gtkStep = 2;
				trackModel->gtHandPos[grabber->cur_image].x = gtkStep;
			}
			else if (annotateMode == GT_R_HAND){						// participant step in handwashing task
				gtkStep = 6;
				trackModel->gtHandPos[grabber->cur_image].y = gtkStep;
			}
		}
		else if(setRegs){
			curRegionToSet = 2;
		}
		break;
	case GDK_3:
		if(annotating && !setRegs){
			if(annotateMode == GT_CENTRE){								// ground truth centre mode
				gtkBodyPartOffset = 2;							// annotating HEAD
			}
			else if (annotateMode == ACTION){						// participant position in washroom mode
				gtkAction = 3;
				trackModel->gtAction[grabber->cur_image] = gtkAction;
			}
			else if (annotateMode == GT_L_HAND){						// participant step in handwashing task
				gtkStep = 3;
				trackModel->gtHandPos[grabber->cur_image].x = gtkStep;
			}
			else if (annotateMode == GT_R_HAND){						// participant step in handwashing task
				gtkStep = 6;
				trackModel->gtHandPos[grabber->cur_image].y = gtkStep;
			}
		}
		else if(setRegs){
			curRegionToSet = 3;
		}
		break;
	case GDK_4:
		if(annotating && !setRegs){
			if(annotateMode == GT_CENTRE){								// ground truth centre mode
				// do nothing
			}
			else if (annotateMode == ACTION){						// participant position in washroom mode
				gtkAction = 4;
				trackModel->gtAction[grabber->cur_image] = gtkAction;
			}
			else if (annotateMode == GT_L_HAND){						// participant step in handwashing task
				gtkStep = 4;
				trackModel->gtHandPos[grabber->cur_image].x = gtkStep;
			}
			else if (annotateMode == GT_R_HAND){						// participant step in handwashing task
				gtkStep = 6;
				trackModel->gtHandPos[grabber->cur_image].y = gtkStep;
			}
		}
		else if(setRegs){
			curRegionToSet = 4;
		}
		break;
	case GDK_5:
		if(annotating && !setRegs){
			if(annotateMode == GT_CENTRE){								// ground truth centre mode
				// do nothing
			}
			else if (annotateMode == ACTION){						// participant position in washroom mode
				// do nothing
			}
			else if (annotateMode == GT_L_HAND){						// participant step in handwashing task
				gtkStep = 5;
				trackModel->gtHandPos[grabber->cur_image].x = gtkStep;
			}
			else if (annotateMode == GT_R_HAND){						// participant step in handwashing task
				gtkStep = 6;
				trackModel->gtHandPos[grabber->cur_image].y = gtkStep;
			}
		}
		else if(setRegs){
			curRegionToSet = 5;
		}
		break;
	case GDK_6:
		if(annotating && !setRegs){
			if(annotateMode == GT_CENTRE){								// ground truth centre mode
				// do nothing
			}
			else if (annotateMode == ACTION){						// participant position in washroom mode
				// do nothing
			}
			else if (annotateMode == GT_L_HAND){						// participant step in handwashing task
				gtkStep = 6;
				trackModel->gtHandPos[grabber->cur_image].x = gtkStep;
			}
			else if (annotateMode == GT_R_HAND){						// participant step in handwashing task
				gtkStep = 6;
				trackModel->gtHandPos[grabber->cur_image].y = gtkStep;
			}
		}
		else if(setRegs){
			curRegionToSet = 6;
		}
		break;
	default:
		return FALSE;
	}


	return FALSE;
}

static gboolean gtkButtonPressEvent( GtkWidget *widget, GdkEventButton *event )
{
	if(annotating && !setRegs){
		if(annotateMode == GT_CENTRE){
			if (event->button == 1){		// Left click. Store the value of the point in the ground truth model
				Point3_<float> projCoords = Point3_<float>(event->x, event->y, depthIm.at<ushort>(Point(event->x,event->y))/10.0);		// coordinates are x,y in image window with depth of pixel (x,y) in mm
				Point3_<float> realCoords;

				if(projCoords.z == 0){
					cout << "Invalid pixel (depth == 0)." << endl;
					return FALSE;
				}
				else{
					grabber->convertProjToReal(1, &projCoords, &realCoords);
					trackModel->gtPartCentres[gtkBodyPart[gtkBodyPartOffset]][grabber->cur_image] = realCoords;
					gtkBodyPartOffset ++;
					if(gtkBodyPartOffset > (gtkBodyPart.size() - 1) )
						gtkBodyPartOffset = 0;
				}
			}
			else if (event->button == 3){		// Right click. Remove the point stored for the current point from the ground truth model.
				trackModel->gtPartCentres[gtkBodyPart[gtkBodyPartOffset]][grabber->cur_image] = Point3_<float>(0,0,-1);
				gtkBodyPartOffset ++;
				if(gtkBodyPartOffset > (gtkBodyPart.size() - 1) )
					gtkBodyPartOffset = 0;
			}
		}
	}
	if(setRegs == 1){
		if(trackModel->regions[curRegionToSet].label == "AWAY" || trackModel->regions[curRegionToSet].label == "SINK"){
			cout << "We don't explicitly set AWAY or SINK." << endl;
			return FALSE;
		}
		if(!regionSetMode){					// setting centre of current region
			if(event->button == 1){			// Left click
				Point3_<int> projCoords = Point3_<int>(event->x, event->y, depthIm.at<ushort>(Point(event->x,event->y))/10.0);		// coordinates are x,y in image window with depth of pixel (x,y) in mm

				if(projCoords.z == 0){
					cout << "Invalid pixel (depth == 0)." << endl;
					return FALSE;
				}
				else{
					trackModel->regions[curRegionToSet].centre = projCoords;
					if(VERBOSE)
						cout << "Centre for " << trackModel->regions[curRegionToSet].label << " set to " << trackModel->regions[curRegionToSet].centre << endl;
					saveRegions("regions.txt");
				}
			}
		}
		else{								// setting radius of current region
			if(trackModel->regions[curRegionToSet].centre.z != 0){
				trackModel->regions[curRegionToSet].radius = sqrt(pow(trackModel->regions[curRegionToSet].centre.x - event->x,2) + pow(trackModel->regions[curRegionToSet].centre.y - event->y,2));
				if(VERBOSE)
					cout << "Radius for " << trackModel->regions[curRegionToSet].label << " set to " << trackModel->regions[curRegionToSet].radius << endl;
				saveRegions("regions.txt");
			}
			else{
				cout << "Set the centre for this region first before setting the radius" << endl;
				return 0;
			}
		}
	}
	return TRUE;
}

static void gtkQuit( GtkWidget *widget, gpointer data )
{
	cout << "Quit button pressed." << endl;
	gtk_widget_destroy(mainWindow);
}

// Switch between run modes
static void gtkSwitchSource ( GtkWidget *widget, gpointer data){

	// save current source in case disk fails
	cameraSource curSource = grabber->source;

	// Reinitialize grabber to read from DISK
	if(curSource == KIN){

		// Change to DISK
		runConf.source = DISK;
		initGrabber(DISK, 640, 480, roi, BUFFER_SIZE);

		updateGrabber(0);

		gtk_button_set_label(GTK_BUTTON(sourceButton), "Source: DISK");
	}
	else if (curSource == DISK){
		runConf.source = KIN;
		initGrabber(KIN, 640, 480, roi, BUFFER_SIZE);

		updateGrabber(0);

		gtk_button_set_label(GTK_BUTTON(sourceButton), "Source: KIN");
	}
	else{
		cout << "Current source: " << curSource << " is unknown. Cannot switch image sources." << endl;
		exit(0);
	}


}

// increment, decrement or reset the current image count
static void gtkChangeImage ( GtkWidget *widget, gpointer data ){

	int index = *((char*)(&data));

	if (index == 0){
		grabber->cur_image = 0;
	}
	else{
		if(grabber->cur_image == 0 && index < 0){					// rollover backward
			grabber->cur_image = grabber->depthList.size() - 1;
		}
		else if (grabber->cur_image == (grabber->depthList.size() - 1) && index > 0){	// rollover forward
			grabber->cur_image = 0;
		}
		else{
			grabber->cur_image = grabber->cur_image + index;
			if((annotateMode == GT_L_HAND || annotateMode == GT_R_HAND )&& !setRegs)									// load the next image that was flagged WASH if only annotating the washing steps
				if(trackModel->gtAction[grabber->cur_image] != 2)
					gtkChangeImage(widget,data);

		}
	}
}

// increment or decrement the annotation directory containing validation images
static void gtkChangeAnnotationDir ( GtkWidget *widget, gpointer data ){

	int index = *((char*)(&data));

	changeAnnotationDir(index);

}

// Start COACH running from live camera
static void gtkRun( GtkWidget *widget, gpointer data )
{
	if (forestLoaded){
		if (running == 0){		// Start running

			running = 1;

			if(grabber->source == KIN){

				// if any images are selected to log to disk, create a new trial
				if(saveRGB || saveDepth || saveDMask || saveFore || saveSkel)
					nextTrial();

				// crate a background mask from the last few frames
				grabber->createMask();
				gtk_button_set_label(GTK_BUTTON(runButton), "STOP");

				running = 1;
			}
			else if(grabber->source == DISK){
				int ready = 1;

				if(grabber->num_images < 1){
					cout << "No images in directory" << endl;
					ready = 0;
				}

				// check to make sure both images were loaded
				if(grabber->bgImage.empty()){
					cout << "Background image was not in directory." << endl;
					ready = 0;
				}

				if(grabber->bgInvalidMask.empty()){
					cout << "Background image invalid pixel mask was not in directory." << endl;
					ready = 0;
				}

				if (ready == 1){
					running = 1;
					gtk_button_set_label(GTK_BUTTON(runButton), "STOP");
				}
			}
			else{
				cout << "Unknown source selected during start." << endl;
				exit(0);
			}

			gtkPushStatus(statusBar, "COACH is running.", context_id);
			g_timer_reset( runTimer );
			g_timer_start( runTimer );
			last_acc_s = 0;

		}
		else{					// Stop running
			running = 0;

			gtk_button_set_label(GTK_BUTTON(runButton), "RUN");
			gtkPushStatus(statusBar, "System is stopped.", context_id);
			g_timer_stop( runTimer );
		}
	}
	else
		gtkPushStatus(statusBar, "Load forest before trying to run.", context_id);
}

static void gtkSetDiskPath ( GtkWidget *widget, gpointer data ) {

	string sourcePath = gtkGetDirPath("Select a directory containing trial images.");

	runConf.diskpath = sourcePath;

	updateConfig("config.txt", "DISK_PATH", runConf.diskpath);
	gtk_label_set_label(GTK_LABEL(diskpath), runConf.diskpath.c_str());

	updateGrabber(1);
}

string gtkGetDirPath ( string title ){

	GtkWidget *dialog;
	string path;

	dialog = gtk_file_chooser_dialog_new (title.c_str(),
			window,
			GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
			NULL);
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char* filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		path.assign(filename);
		g_free (filename);
	}

	gtk_widget_destroy (dialog);

	return path;
}

string gtkGetFilePath ( string title ){

	GtkWidget *dialog;
	string filepath;

	dialog = gtk_file_chooser_dialog_new (title.c_str(),
			window,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
			NULL);
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char* filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		filepath.assign(filename);
		g_free (filename);
	}

	gtk_widget_destroy (dialog);

	return filepath;
}

/* generic callback for radio buttons
 * 0-9 are reserved for left image window radio buttons
 * 10-19 are reserved for right image window radio buttons
 */
static void gtkImageRadioCallback( GtkButton *b, gpointer data )
{
	int index = *((char*)(&data));

	if(index < 10)
		lImg = index;
	else
		rImg = index-10;
}

static void gtkKalmanFiltersCallback( GtkButton *widget, gpointer data )
{
	enableFilters = GTK_TOGGLE_BUTTON (widget)->active;
}

static void gtkAllModesCallback( GtkButton *widget, gpointer data )
{
	allModes = GTK_TOGGLE_BUTTON (widget)->active;
}

static void gtkAllSamplesCallback( GtkButton *widget, gpointer data )
{
	allSamples = GTK_TOGGLE_BUTTON (widget)->active;
}

static void gtkDispRegions( GtkButton *widget, gpointer data ){

	dispRegions= GTK_TOGGLE_BUTTON (widget)->active;

}

static void gtkPartClassifyCallback( GtkButton *widget, gpointer data )
{
	trackModel->bodyParts.classify[partSelect] = GTK_TOGGLE_BUTTON (partClass)->active;
	//TODO create a saveconfigTree function to update config file when this is changed
}


static void gtkScaleChangeCallback( GtkButton *widget, gpointer data )
{
	//gtk_scale_button_set_value(GTK_SCALE_BUTTON(scalebutton), model->bodyParts.bandwidths[partSelect]);
	scaleValue = gtk_scale_button_get_value(GTK_SCALE_BUTTON(widget));

	ostringstream out;
	out << trackModel->bodyParts.label[partSelect] << " bandwidth chagned to " << scaleValue;

	gtkPushStatus(statusBar, out.str(), context_id);

	// set the bandwidth to the value from the scale
	trackModel->bodyParts.bandwidths[partSelect] = scaleValue;

}

static void gtkPartSelectCallback( GtkComboBox *widget, gpointer data )
{
	// set partSelect to the current option in the combobox
	partSelect = gtk_combo_box_get_active (widget);

	// set the classify toggle to the state of the part
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (partClass), trackModel->bodyParts.classify[partSelect]);
}

void gtkSaveCallback (GtkWidget *widget, gpointer data)
{
	switch (*((char*)(&data)))
	{
	case 0:
		saveRGB = GTK_TOGGLE_BUTTON (widget)->active;
		if(saveRGB) gtkPushStatus(statusBar, "Saving RGB images.", context_id);
		else gtkPushStatus(statusBar, "...", context_id);
		break;
	case 1:
		saveDepth = GTK_TOGGLE_BUTTON (widget)->active;
		if(saveDepth) gtkPushStatus(statusBar, "Saving depth images.", context_id);
		else gtkPushStatus(statusBar, "...", context_id);
		break;
	case 2:
		saveDMask = GTK_TOGGLE_BUTTON (widget)->active;
		if(saveDMask) gtkPushStatus(statusBar, "Saving depth masks.", context_id);
		else gtkPushStatus(statusBar, "...", context_id);
		break;
	case 3:
		saveFore = GTK_TOGGLE_BUTTON (widget)->active;
		if(saveFore) gtkPushStatus(statusBar, "Saving foreground images.", context_id);
		else gtkPushStatus(statusBar, "...", context_id);
		break;
	case 4:
		saveSkel = GTK_TOGGLE_BUTTON (widget)->active;
		if(saveSkel) gtkPushStatus(statusBar, "Saving skeleton images.", context_id);
		else gtkPushStatus(statusBar, "...", context_id);
		break;
	default:
		gtkPushStatus(statusBar, "Unknown image save option selected.", context_id);
		break;
	}
}

static void gtkTrainTree( GtkWidget *widget, gpointer data )
{

	if(!running){
		// TODO: this locks the GUI. Should be fixed but OpenMP doesn't support detached threads

		if(trainTree()){				// train new tree
			cout << "Tree training failed." << endl;
		}
	}
	else{
		gtkPushStatus(statusBar, "You can't train a new tree while the system is running.", context_id);
	}

}

/*static int gtkLoadForest( GtkWidget *widget, gpointer data ){

	return (loadForest(model));
}*/

static void gtkAnnotationMode( GtkWidget *widget, gpointer data ){

	changeAnnotationMode();

}

static void gtkForestPerClassMetrics( GtkWidget *widget, gpointer data ){

	string dirCheck = gtkGetDirPath("Select validation folder");

	for(int i = 0; i < runConf.num_trees; i++){
		forestPerClassMetrics(dirCheck, 1, i, 1);
	}

	forestPerClassMetrics(dirCheck, runConf.num_trees ,0,1);

	//checkForestAccuracy(dirCheck, runConf.num_trees);
}

static void gtkForestPartPropMetrics( GtkWidget *widget, gpointer data ){

	string dirCheck = gtkGetDirPath("Select validation folder");

	forestPartPropMetrics(dirCheck, runConf.num_trees, 1);

}

static void gtkValidationMetrics( GtkWidget *widget, gpointer data ){

	bool origAnnotating = annotating;

	// ensure that we are enabling annotating
	annotating = 0;

	// ask user to input validation trial parent directory
	changeAnnotationMode();

	vector<string> results;																	// vector to hold the results of the hand positions over the validation trials

	// loop through each validation trial and calculate metrics
	for(unsigned int i = 0; i < grabber->validateDirs.size(); i++){

		cout << "\tCalculating metrics for " << grabber->validateDirs[i] << endl;

		vector<vector<Point3_<float> > > partCentres(trackModel->bodyParts.count);			// vector to hold the classified part centres for each image in current validation trial

		// find the part centres and calculate error metrics over validation images
		validationPartPropMetrics(results, partCentres);									// generate validation image part proposal metrics

		vector<Point> rawHandActions(partCentres[0].size());								// vector to hold the raw left and right hand actions for each image in current validation trial
		vector<Point> condHandActions(partCentres[0].size());								// vector to hold the conditined left and right hand actions for each image in current validation trial

		// loop through each image
		for(unsigned int image = 0; image < partCentres[0].size(); image ++){

			// use hand locations to find the task action each hand is completing
			findAction(partCentres[0][image], partCentres[1][image], rawHandActions[image], condHandActions[image]);

		}

		saveActions(rawHandActions, condHandActions, partCentres, runConf.diskpath, grabber->depthList);		// save classified part centres and hand actions to file

		changeAnnotationDir(1);																// index to the next validation trial
	}

	// switch back to original annotating mode
	if (origAnnotating != annotating)
		changeAnnotationMode();

}

static void gtkSetRegions( GtkWidget *widget, gpointer data ){

	if(!dispRegions){
		cout << "Select the 'Disp Regions' toggle button first." << endl;
		return;
	}

	if (!setRegs){
		if(annotating){
			gtk_button_set_label(GTK_BUTTON(setRegions), "Stop setting regions");
			setRegs = TRUE;
		}
		else
			cout << "Must be in annotate mode" << endl;
	}
	else{
		gtk_button_set_label(GTK_BUTTON(setRegions), "Set regions");
		setRegs = FALSE;
	}
}

static void gtkPushStatus( GtkWidget *widget, string message, gint data )
{
	gtk_statusbar_remove_all(GTK_STATUSBAR (widget), data);
	gtk_statusbar_push (GTK_STATUSBAR (widget), data, message.c_str());

}

static int gtkMakeLabelImages( GtkWidget *widget, string message, gint data ){			// Automatically perform the three steps to convert 16-bit depth images to 3-channel labeling images

	// get user-specified source directory
	string rootpath = gtkGetDirPath("Please select root directory containing 16-bit depth image directory 'depth16bit'");
	string bgpath = rootpath + "/depthNoBG";

	if(!createBackground(rootpath)){
		cout << "\tCould not create background images." << endl;
		return 0;
	}
	if(!removeBackground(rootpath, bgpath)){
		cout << "\tCould not remove background." << endl;
		return 0;
	}
	if(!makeLabelImages(rootpath, bgpath)){
		cout << "\tCould not make three channel label images." << endl;
		return 0;
	}

	cout << "\tSuccessfully created labeling images." << endl;

	return 1;
}

static void gtkCreateBackground( GtkWidget *widget, string message, gint data ){

	// get user-specified source directory
	string rootpath = gtkGetDirPath("Please select root directory containing 16-bit depth image directory 'depth16bit'");

	createBackground(rootpath);

}

static void gtkRemoveBackground( GtkWidget *widget, string message, gint data ){

	// get user-specified source directory
	string rootpath = gtkGetDirPath("Please select directory containing 16-bit depth images and background images");
	string targetpath = rootpath + "/depthNoBG";

	removeBackground(rootpath, targetpath);

}

static void gtkMakeDepthImages( GtkWidget *widget, string message, gint data ){

	string rootpath = gtkGetDirPath("Please select root directory containing 8-bit depth images (in a directory called 'depth') and a directory called 'label'.");
	string sourcepath = rootpath + "/depthNoBG";

	makeLabelImages(rootpath, sourcepath);

}

static gboolean gtkDeleteEvent( GtkWidget *widget, GdkEvent  *event, gpointer   data )
{
	cout << "Main window closed." << endl;;

	return FALSE;
}

static void gtkDestroy( GtkWidget *widget, gpointer   data )
{
	gtk_main_quit ();
}

/***************************************************************************
 * END GTK Callback functions
 **************************************************************************/

int listFiles (string dir, vector<string>& files, string extension)
{
	DIR* dp;
	struct dirent* entry;
	if((dp  = opendir(dir.c_str())) == NULL){
		cout << "listFiles: Error opening " << dir << endl;
		return 0;
	}

	files.clear();

	while ((entry = readdir(dp)) != NULL) {
		string file = string(entry->d_name);
		// return files that are of the type defined by extension
		if( file != "." && file != ".." && file.find(extension) != string::npos){
			files.push_back(file);
		}
	}
	closedir(dp);
	sort(files.begin(), files.end());
	return 1;
}
/******************************************************************************/

int listDirectories (string sourcedir, vector<string>& dirs)
{

	DIR* dp;
	struct dirent* entry;
	if((dp  = opendir(sourcedir.c_str())) == NULL){
		cout << "listFiles: Error opening " << sourcedir << endl;
		return 0;
	}

	while ((entry = readdir(dp)) != NULL) {
		string dirname = string(entry->d_name);

		if(entry->d_type == 0x4 && dirname != "." && dirname != "..")		// is a directory
			dirs.push_back(dirname);

	}
	closedir(dp);
	sort(dirs.begin(), dirs.end());
	return 1;
}
/******************************************************************************/

int nextTrial (void)
{
	DIR* dp;
	string dir = projectPath + "/trial";
	string trial = trialDate();

	struct dirent* entry;
	if((dp  = opendir(dir.c_str())) == NULL){
		cout << "Error opening " << dir << endl;
		return 0;
	}

	int count = 0;
	while ((entry = readdir(dp)) != NULL) {
		if( strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 ){
			string name(entry->d_name);
			if (name.compare(0,trial.size(),trial) == 0 )
				count ++;
		}
	}
	closedir(dp);

	// Initialize folders

	stringstream trialDir;
	trialDir << "/" << trial << "trial-" << setfill('0') << setw(3) << count;
	trialPath = dir + trialDir.str();
	string folder = trialPath + "/camera";

	if(mkdir(trialPath.data(),0777) != 0)
		cout << "Error making /trialRoot folder" << endl;

	if(mkdir(folder.data(),0777) != 0)
		cout << "Error making /camera folder" << endl;

	folder = trialPath + "/camera/rgb";
	if(mkdir(folder.data(),0777) != 0)
		cout << "Error making /camera/rgb folder" << endl;

	folder = trialPath + "/camera/depth";
	if(mkdir(folder.data(),0777) != 0)
		cout << "Error making /camera/depth folder" << endl;

	folder = trialPath + "/camera/mask";
	if(mkdir(folder.data(),0777) != 0)
		cout << "Error making /camera/mask folder" << endl;

	folder = trialPath + "/camera/fore";
	if(mkdir(folder.data(),0777) != 0)
		cout << "Error making /camera/fore folder" << endl;

	folder = trialPath + "/camera/skel";
	if(mkdir(folder.data(),0777) != 0)
		cout << "Error making /camera/skel folder" << endl;

	return 1;
}
/******************************************************************************/

// checks to see if point is a background pixel. Note that this requires invalid pixels to be removed before creating the training samples
int isBackground (Point pt, Mat label){

	int blue = label.at<Vec3b>(pt.y,pt.x)[0];
	int green = label.at<Vec3b>(pt.y,pt.x)[1];
	int red = label.at<Vec3b>(pt.y,pt.x)[2];

	// black (0,0,0) pixels are background and invalid in labeled images. Invalid samples should be removed before calling this.
	if (blue == 0 && green == 0 && red == 0)
		return(1);

	return(0);

}

/******************************************************************************/

/* Calculate the ground-truth colour feature. Return values:
 *	1. BG_PIXEL if pixel colour = (0,0,0)
 *	2. forest[tree]->parts.count if pixel colour is greyscale (i.e., unlabeled but on the foreground image)
 *	3. index of part colour if it's a valid colour
 */
int colourFeature (int tree, Point pt, string file, Mat labelImage){

	int blue = labelImage.at<Vec3b>(pt.y,pt.x)[0];
	int green = labelImage.at<Vec3b>(pt.y,pt.x)[1];
	int red = labelImage.at<Vec3b>(pt.y,pt.x)[2];

	int i;

	if (blue == green && blue == red)	// background pixel is anything not labeled (except a black/unknown pixel)
	{
		if (blue == 0)
			return BG_PIXEL;			// assume anything black is a background pixel
		else
			return forest[tree]->parts.count - 1;		// the last configured part is for any unlabeled body parts (anything on the greyscale spectrum other than black)
	}

	for(i = 0; i < forest[tree]->parts.count; i++){
		if (blue == forest[tree]->parts.colours[i].x){
			if(green == forest[tree]->parts.colours[i].y){
				if(red == forest[tree]->parts.colours[i].z){
					return i;
				}
			}
		}
	}
	if (i == forest[tree]->parts.count){
		cout << "Error in colourFeature. Unknown pixel colour, (R,G,B) = (" << red << "," << green << "," << blue << ")" << endl;
		cout << "Image: " << file << ", point: " << pt.x << "," << pt.y << endl;
		exit(0);
	}
	return i;
}
/******************************************************************************/

// Calculate the depth feature for node splitting criteria according to
// "Real-Time human pose recognition in parts from single depth images" by Shotton et. all (2011)

int depthFeature (int tree, Point pt, split_cond split, Mat labelImage, Mat depthMap){

	unsigned short  dIu, dIv;
	Point u,v;

	float dI = depthMap.at<ushort>(Point(pt.x,pt.y))/1000.0;				// find depth of pixel (x,y) in mm*10 and convert to meters

	// calculate all offset points using the depth-normalized offsets of u,v
	u.x = int(pt.x + split.u.x/dI);
	u.y = int(pt.y + split.u.y/dI);
	v.x = int(pt.x + split.v.x/dI);
	v.y = int(pt.y + split.v.y/dI);

	// if the offset point lies on the background or is outside the bounds of the image, the depth probe (dIu/dIv) is given a large positive value
	if( (u.x < 0) || (u.x > WIDTH-1) || (u.y < 0) || (u.y > HEIGHT - 1) || (isBackground(u, labelImage) == 1) )
		dIu = 20000;
	else
		dIu = depthMap.at<ushort>(Point(u.x,u.y));

	if( (v.x < 0) || (v.x > WIDTH-1) || (v.y < 0) || (v.y > HEIGHT - 1) || (isBackground(v, labelImage) == 1) )
		dIv = 20000;
	else
		dIv = depthMap.at<ushort>(Point(v.x,v.y));

	// if either of the offset pixels are invalid, return invalid, else return the feature
	if (dIu == 0 || dIv == 0)
		return INV_DEPTH;
	else
		return (dIu - dIv);
}
/******************************************************************************/

/*int loadCandidates (vector<split_cond>& cand)
{
	string line;
	string filename = projectPath + "/offsets.txt";
	ifstream infile (filename.data());
	int i=0;

	if (infile.is_open())
	{
		int x;
		string head;
		getline(infile,line);
		istringstream splitHead(line);
		splitHead >> head;
		if (head == "SPLIT_CANDIDATES"){
			splitHead >> x;
			if(x != SPLIT_CANDIDATES){
				cout << "Offsets file doesn't have the right number of split candidates." << endl;
				return 1;
			}
		}
		else{
			cout << "First line of offset file should be SPLIT_CANDIDATES <x>." << endl;
			exit(0);
		}
		getline(infile,line);
		istringstream offsetHead(line);
		offsetHead >> head;
		if (head == "MAX_OFFSET"){
			offsetHead >> x;
			if(x != offset){
				cout << "Offsets file doesn't have the maximum offset." << endl;
				return 1;
			}
		}
		else{
			cout << "Second line of offset file should be MAX_OFFSET <x>." << endl;
			exit(0);
		}
		split_cond val;
		while ( infile.good() )
		{
			getline(infile,line);
			istringstream coordinates(line);
			int ux, uy, vx, vy;
			if (!(coordinates >> ux >> uy >> vx >> vy))
			{
				cerr << "Error in split candidate input file" << endl;
				return 1;
			}
			else{
				val.u.x = ux;
				val.u.y = uy;
				val.v.x = vx;
				val.v.y = vy;
				cand.push_back(val);
			}
			i++;
		}
		infile.close();
	}

	else {
		cout << "Unable to open offset file!" << endl;
		return 1;
	}

	if (i == SPLIT_CANDIDATES){
		cout << "Loaded " << i << " randomized offset point pairs from file." << endl;
		return 0;
	}
	else {
		cout << "Not enough points in file!" << endl;
		return 1;
	}
	return 0;
}
 */
/******************************************************************************/

// load ground truth part centres from file. Note that labellist is optional, and only used if gt file was created using centre of moments.
int loadGroundTruthCentres(string path){

	string line;
	string filename = path + "/gtCentres.txt";
	int val;
	Point pt;
	ifstream infile (filename.data());

	int count = 0;

	for(int part = 0; part < trackModel->bodyParts.count; part++){
		trackModel->gtPartCentres[part].clear();
		trackModel->gtHandPos.clear();
		trackModel->gtAction.clear();
	}

	if (infile.is_open())
	{
		while ( getline(infile,line) )
		{
			istringstream nextline(line);
			string head, filename;
			if (!(nextline >> head))
			{
				cout << "Error in ground truth part centres file at iteration " << count << ". Data: " << head << endl;
				exit(0);
				return 0;
			}

			if (head == "F" ){
				nextline >> filename;

				// Note that these static_casts don't handle the int->enum very well. No error-checking is done here so they MUST pass a valid enum value
				nextline >> val;
				trackModel->gtAction.push_back(val);
				nextline >> pt.x;
				nextline >> pt.y;
				trackModel->gtHandPos.push_back(pt);

				Point3_<float> centre;
				for(int part = 0; part < trackModel->bodyParts.count; part++){
					nextline >> centre.x >> centre.y >> centre.z;
					trackModel->gtPartCentres[part].push_back(centre);
				}
				count ++;
			}

		}

		infile.close();
	}
	else {
		cout << "\tUnable to open ground truth body part centres file " << filename.data() << "." << endl;
		return 0;
	}

	// check if part centre list and depth list have the same number of entries
	if(trackModel->gtPartCentres[0].size() != grabber->depthList.size()){
		cout << "\t" << filename << " does not have the same number of entries as depth list." << endl;
		cout << "\tPart centres: " << trackModel->gtPartCentres[0].size() << ", depth list: " << grabber->depthList.size() << endl;
		return 0;

	}


	cout << "\tLoaded ground truth part centres for " << count << " images." << endl;

	return 1;

}
/******************************************************************************/

int saveGroundTruthPartCentres(string path, string title, vector<string>& depthList){
	ofstream outputFile;
	string filename = path + "/gtCentres.txt";

	ifstream fin(filename.data());
	if (fin)
	{
		fin.close();
		if( remove( filename.data() ) != 0 ){
			cout << "Error deleting " << filename.data() << endl;
			return 0;
		}
	}

	outputFile.open(filename.data());

	// header info
	outputFile << "# " << title << endl;
	outputFile << "# part centres are in world space coordinates." << endl;
	outputFile << "# ACTION: " << endl;
	for(unsigned int i = 0; i < runConf.positions.size(); i++)
		outputFile << "#\t" << i << " " << runConf.positions[i] << endl;

	outputFile << "# GT_HAND: " << endl;
	for(unsigned int i = 0; i < runConf.activities.size(); i++)
		outputFile << "#\t" << i << " " << runConf.activities[i] << endl;

	outputFile << "# output format is: " << endl;
	outputFile << "# F <filename> ACTION GT_L_HAND GT_R_HAND ";
	for(int i = 0; i < trackModel->bodyParts.count; i++)
		outputFile << trackModel->bodyParts.label[i] << " ";

	outputFile << endl;

	for(unsigned int img = 0; img < depthList.size(); img++){
		outputFile << "F " << depthList[img] << " " << trackModel->gtAction[img] <<
				" " << trackModel->gtHandPos[img].x << " " << trackModel->gtHandPos[img].y;
		for(int part = 0; part < trackModel->bodyParts.count; part++){
			outputFile << " " << trackModel->gtPartCentres[part][img].x << " " << trackModel->gtPartCentres[part][img].y << " " << trackModel->gtPartCentres[part][img].z;
			if (part != trackModel->bodyParts.count - 1)
				outputFile << " ";
		}
		outputFile << endl;
	}
	outputFile.close();
	return 1;
}
/******************************************************************************/

int loadActions (vector<Point>& handActions, string path){
	string line, type;
	string filename = path + "/classCentresAndActions.txt";

	handActions.clear();

	if(VERBOSE)
		cout << "\tOpening region file: " << filename << endl;

	ifstream infile (filename.data());

	unsigned int count = 0;

	if (infile.is_open())
	{
		while ( infile.good() )
		{
			getline(infile,line);
			istringstream nextline(line);

			Point val;
			string head;						// head of line
			nextline >> head;

			if (head == "F" ){
				nextline >> filename;

				// Note that these static_casts don't handle the int->enum very well. No error-checking is done here so they MUST pass a valid enum value
				nextline >> val.x;
				nextline >> val.y;
				handActions.push_back(val);

				count ++;
			}
		}
		infile.close();
	}
	else {
		cout << "Unable to open classified centres and actions file." << endl;
		return 0;
	}

	if (count != grabber->depthList.size()){
		cout << "Not enough classified hand actions were present in file." << endl;
		return 0;
	}
	else if(VERBOSE)
		cout << "\tLoaded classified hand actions." << endl;

	return 1;
}

/******************************************************************************/
int saveActions (vector<Point>& rawHandActions, vector<Point>& condHandActions, vector<vector<Point3_<float> > >& partCentres, string path, vector<string>& depthList){
	if(VERBOSE)
		cout << "Saving actions." << endl;

	ofstream outputFile;
	string filename = path + "/classCentresAndActions.txt";

	// delete the file if it exists
	ifstream f(filename.c_str());
	if (f.good()) {
		f.close();
		if( remove( filename.data() ) != 0 ){
			cout << "\tError deleting " << filename.data() << endl;
			return 0;
		}
	}

	outputFile.open(filename.data());

	// header info
	outputFile << "# classified part centres and hand actions."  << endl;
	outputFile << "# classified part centres are in projective space coordinates." << endl;

	outputFile << "# COND_X_HAND: " << endl;
	for(unsigned int i = 0; i < trackModel->regions.size(); i++)
		outputFile << "#\t" << i << " " << trackModel->regions[i].label << endl;

	outputFile << "# RAW_X_HAND: " << endl;
	for(unsigned int i = 0; i < runConf.activities.size(); i++)
		outputFile << "#\t" << i << " " << runConf.activities[i] << endl;

	outputFile << "# output format is: " << endl;
	outputFile << "# F <filename> COND_L_HAND COND_R_HAND RAW_L_HAND RAW_R_HAND ";
	for(int i = 0; i < trackModel->bodyParts.count; i++)
		outputFile << trackModel->bodyParts.label[i] << " ";

	outputFile << endl;

	for(unsigned int img = 0; img < partCentres[0].size(); img++){
		outputFile << "F " << depthList[img] <<
				" " << condHandActions[img].x << " " << condHandActions[img].y <<
				" " << rawHandActions[img].x << " " << rawHandActions[img].y;
		for(int part = 0; part < trackModel->bodyParts.count; part++){
			outputFile << " " << partCentres[part][img].x << " " << partCentres[part][img].y << " " << partCentres[part][img].z;
			if (part != trackModel->bodyParts.count - 1)
				outputFile << " ";
		}
		outputFile << endl;
	}
	if(VERBOSE)
		cout << "Actions saved to file." << endl;
	outputFile.close();
	return 1;
}
/******************************************************************************/

int createCandidates (vector<split_cond>& cand){

	split_cond val;
	const float  pi = 3.14159265358979f;
	float theta, mag;

	// Randomly propose a set of offsets (u,v)
	for(int i=0; i< SPLIT_CANDIDATES; i++){
		theta = ( ( 1 - (-1) )*(rand() / double(RAND_MAX)) - 1 ) * pi;		// random angle between -1 and 1 (radians)
		mag = rand()%(offset);												// random magnitude between 0 and offset
		val.u.x = mag*sin(theta);
		val.u.y = mag*cos(theta);
		theta = ( ( 1 - (-1) )*(rand() / double(RAND_MAX)) - 1 ) * pi;		// random angle between -1 and 1 (radians)
		mag = rand()%(offset);												// random magnitude between 0 and offset
		val.v.x = mag*sin(theta);
		val.v.y = mag*cos(theta);
		cand.push_back(val);
	}

	// Randomly propose a set of offsets (u,v). Each set of offsets is first generated as the square root of the total requested candidates.
	// Then each combination of u and v is added to the final candidate vector.
	/*vector<split_cond> rawUV;
	int numCands = ceil(sqrt(SPLIT_CANDIDATES));
	for(int i = 0; i < numCands; i++){
		theta = ( ( 1 - (-1) )*(rand() / double(RAND_MAX)) - 1 ) * pi;		// random angle between -1 and 1 (radians)
		mag = rand()%(offset);												// random magnitude between 0 and offset
		val.u.x = mag*sin(theta);
		val.u.y = mag*cos(theta);
		theta = ( ( 1 - (-1) )*(rand() / double(RAND_MAX)) - 1 ) * pi;		// random angle between -1 and 1 (radians)
		mag = rand()%(offset);												// random magnitude between 0 and offset
		val.v.x = mag*sin(theta);
		val.v.y = mag*cos(theta);
		rawUV.push_back(val);
	}
	for(int i = 0; i < numCands; i++){
		for(int j = 0; j < numCands; j++){
			split_cond temp;
			temp.u = rawUV[i].u;
			temp.v = rawUV[j].v;
			cand.push_back(temp);
		}
	}*/
	cout << "Created " << cand.size() << " split candidates." << endl;


	return 1;
}
/******************************************************************************/

int createThresholds (vector<int> & thresh){

	cout << "Creating set of training thresholds." << endl;

	// Generate a set of evenly spaced thresholds spanning the threshold range
	for(int i = 0; i < NUM_THRESHOLDS; i++){
		thresh.push_back(-threshMagnitude + (i*threshMagnitude*2)/(NUM_THRESHOLDS-1));
	}

	cout << "\tSorting thresholds by magnitude." << endl;
	// sort from smallest to largest by magnitude (ignoring sign) so that gain calculations will favour thresholds closer to the median threshold
	for(unsigned int i = 0; i < thresh.size()-1; i++){
		unsigned int min = i;
		for(unsigned int j = i+1; j < thresh.size(); j++){
			if (abs(thresh[j]) < abs(thresh[min])) min = j;
		}
		if (min != i){	// swap the elements
			int temp = thresh[i];
			thresh[i] = thresh[min];
			thresh[min] = temp;
		}
	}
	return 1;
}
/******************************************************************************/

// create a set of numSamples samples
int createSamples (vector<Point>& samples, int numSamples){

	samples.clear();

	// Randomly propose a set of NUM_PIXELS pixels per image for training
	int x,y;
	for(int i=0; i < numSamples; i++){
		x = rand()%((WIDTH-1)-0)+0;
		y = rand()%((HEIGHT-1)-0)+0;
		samples.push_back(Point(x,y));
	}
	return 1;
}
/******************************************************************************/

int loadForest (){

	if(VERBOSE)
		cout << "Loading " << runConf.num_trees << " decision trees." << endl;

	for(int k = 0; k < runConf.num_trees; k++){
		forest[k]->destroy_tree();
		if(!forest[k]->loadTreeFromFile(projectPath + runConf.treePaths[k]))
			return(0);
	}

	if(VERBOSE)
		cout << "\tForest of " << runConf.num_trees << " trees loaded." << endl;

	// check to make sure all trees in the forest are similar

	if(VERBOSE)
		cout << "\tChecking to make sure that all decision trees match each other and the configuration file." << endl;

	for(int  i= 0; i < runConf.num_trees; i++){
		if(trackModel->bodyParts.count != forest[i]->parts.count){
			cout << trackModel->bodyParts.count << "," << forest[i]->parts.count << endl;
			cout << "Tree " << i << " doesn't have the same number of parts as config file." << endl;
			return 0;
		}
		for (int j = 0; j < trackModel->bodyParts.count; j++){
			if (trackModel->bodyParts.label[j] != forest[i]->parts.label[j]){
				cout << "Tree " << i << " doesn't have the same part label (" << forest[i]->parts.label[j] << ") as config file (" << trackModel->bodyParts.label[j] <<  ")." << endl;
				return 0;;
			}
			if (trackModel->bodyParts.colours[j] != forest[i]->parts.colours[j]){
				cout << "Tree " << i << " doesn't have the same colours as config file." << endl;
				return 0;
			}
		}
	}
	forestLoaded = 1;

	if(VERBOSE)
		cout << "\tDecision forest is coordinated." << endl;

	return 1;

}

/******************************************************************************/

void loadConfig (string file)
{
	string line, type;
	string filename = projectPath + file;

	cout << "Opening tree configuration file: " << filename << endl;

	ifstream infile (filename.data());

	int i = 0;

	if (infile.is_open())
	{
		while ( infile.good() )
		{
			getline(infile,line);
			istringstream nextline(line);
			string head;						// head of line
			nextline >> head;

			if (head == "PART"){
				bool clas;
				float bw, prob;
				Point3_ <int> colour;
				string label;
				if (!(nextline >> clas >> bw >> prob >> colour.x >> colour.y >> colour.z >> label))
				{
					cerr << "Error in tree configuration file." << endl;
					exit(0);
				}
				else{
					trackModel->bodyParts.classify.push_back(clas);
					trackModel->bodyParts.bandwidths.push_back(bw);
					trackModel->bodyParts.probThreshs.push_back(prob);
					trackModel->bodyParts.colours.push_back(colour);
					trackModel->bodyParts.label.push_back(label);
				}
				i ++;
			}
			else if (head == "BGCOLOUR"){
				Point3_ <int> colour;
				if(!(nextline >> colour.x >> colour.y >> colour.z))
				{
					cerr << "Error in tree configuration file for background colour." << endl;
					exit(0);
				}
				else{
					trackModel->bodyParts.bgColour = colour;
				}
			}
			else if (head == "SOURCE"){
				nextline >> type;
				if (type == "KIN")
					runConf.source = KIN;
				else if (type == "DISK")
					runConf.source = DISK;
			}
			else if (head == "DISK_PATH"){
				string path;
				nextline >> path;
				runConf.diskpath = path;
			}
			else if (head == "TREE"){
				string path;
				nextline >> path;
				runConf.treePaths.push_back(path);
			}
			else if (head == "POS"){		// annotation positions
				string pos;
				nextline >> pos;
				runConf.positions.push_back(pos);
			}
			else if (head == "ACT"){		// annotation task steps
				string act;
				nextline >> act;
				runConf.activities.push_back(act);
			}
			else if (head == "REGION"){		// regions
				objectRegion reg;
				reg.centre = Point3_<float>(0,0,0);
				reg.radius = 0.0;
				nextline >> reg.label;
				trackModel->regions.push_back(reg);
			}
		}
		infile.close();

		trackModel->bodyParts.count = trackModel->bodyParts.colours.size();
		runConf.num_trees = runConf.treePaths.size();

		// set up tracking model according to number of body parts
		trackModel->startPoints.resize(trackModel->bodyParts.count);
		trackModel->modes.resize(trackModel->bodyParts.count);
		trackModel->modeConf.resize(trackModel->bodyParts.count);
		trackModel->gtPartCentres.resize(trackModel->bodyParts.count);
		trackModel->classPartCentres.resize(trackModel->bodyParts.count);
		trackModel->gtHandPos.resize(trackModel->bodyParts.count);
		trackModel->gtAction.resize(trackModel->bodyParts.count);
		trackModel->classHandPos.resize(trackModel->bodyParts.count);
		trackModel->partCentres.resize(trackModel->bodyParts.count);
		trackModel->partConf.resize(trackModel->bodyParts.count);


		for(int i = 0; i < trackModel->bodyParts.count; i++){
			trackModel->nullProposals.push_back(NULL_PROP_COUNT);	// initialize filters to be disabled until a valid proposal is given by classifier
		}

		if(VERBOSE){
			cout << "\tLoaded configuration for " << trackModel->bodyParts.count << " body parts from configuration file." << endl;
			cout << "\tLoaded configuration for " << runConf.num_trees << " trees from configuration file." << endl;
			cout << "\tCamera source = " << type << endl;
		}

	}
	else {
		cout << "Unable to open configuration file." << endl;
		exit(0);
	}
}
/******************************************************************************/

int loadCamConfig (string file)
{
	string line, type;
	string filename = projectPath + file;

	if(VERBOSE)
		cout << "\tOpening camera configuration file: " << filename << endl;

	ifstream infile (filename.data());

	if (infile.is_open())
	{
		while ( infile.good() )
		{
			getline(infile,line);
			istringstream nextline(line);
			double intval;
			double doubleval;
			string head;						// head of line
			nextline >> head;
			if (head == "XRES"){
				nextline >> intval;
				grabber->xRes = intval;
			}
			else if (head == "YRES"){
				nextline >> intval;
				grabber->yRes = intval;
			}
			else if (head == "FXTOZ"){
				nextline >> doubleval;
				grabber->fXToZ = doubleval;
			}
			else if (head == "FYTOZ"){
				nextline >> doubleval;
				grabber->fYToZ = doubleval;
			}
		}
		infile.close();

		if(VERBOSE)
			cout << "\tLoaded camera configuration." << endl;
	}
	else {
		cout << "Unable to open configuration file." << endl;
		return 0;
	}
	return 1;
}
/******************************************************************************/

int saveCamConfig(string file)
{

	if(VERBOSE)
		cout << "Checking camera configuration." << endl;

	ofstream outputFile;
	string filename = projectPath + "/" + file;

	int tempXRes = grabber->xRes;
	int tempYRes = grabber->yRes;
	double tempFXToZ = grabber->fXToZ;
	double tempFYToZ = grabber->fYToZ;

	if(loadCamConfig(file)){
		if (tempXRes != grabber->xRes || tempYRes != grabber->yRes || tempFXToZ != grabber->fXToZ || tempFYToZ != grabber->fYToZ){

			if(VERBOSE)
				cout << "\tChanges found in " << filename.data() << ", deleting..." << endl;

			if( remove( filename.data() ) != 0 ){
				cout << "\tError deleting " << filename.data() << endl;
				return 0;
			}
			// Set values back to proper values from current camera
			grabber->xRes = tempXRes;
			grabber->yRes = tempYRes;
			grabber->fXToZ = tempFXToZ;
			grabber->fYToZ = tempFYToZ;
		}
		else{
			cout << "\tCurrent camera configuration matches " << filename << "." << endl;
			return 1;
		}
	}
	if(VERBOSE)
		cout << "\tUpdating camera configuration file with parameters from current camera." << endl;

	outputFile.open(filename.data());

	outputFile << "XRES " << grabber->xRes << endl;
	outputFile << "YRES " << grabber->yRes << endl;
	outputFile << "FXTOZ " << std::setprecision (numeric_limits<double>::digits10) << grabber->fXToZ << endl;
	outputFile << "FYTOZ " << std::setprecision (numeric_limits<double>::digits10) << grabber->fYToZ << endl;

	outputFile.close();
	return 1;
}
/******************************************************************************/

int loadRegions (string file){
	string line, type;
	string filename = projectPath + file;

	if(VERBOSE)
		cout << "\tOpening region file: " << filename << endl;

	ifstream infile (filename.data());

	unsigned int count = 0;

	if (infile.is_open())
	{
		while ( infile.good() )
		{
			getline(infile,line);
			istringstream nextline(line);

			string head;						// head of line
			nextline >> head;
			if (head != "#"){
				bool found = FALSE;
				for (unsigned int region = 0; region < trackModel->regions.size(); region ++){
					if(head == trackModel->regions[region].label){
						nextline >> trackModel->regions[region].centre.x;
						nextline >> trackModel->regions[region].centre.y;
						nextline >> trackModel->regions[region].centre.z;
						nextline >> trackModel->regions[region].radius;
						count ++;
						found = TRUE;
					}
				}
				if (!found){
					cout << "Found region " << head << " in " << filename.data() << " but could not find it in configuration file." << endl;
					return(0);
				}
			}

		}
		infile.close();

		if (count != trackModel->regions.size()){
			cout << "Not all regions in configuration file existed in region file." << endl;
		}
		else if(VERBOSE)
			cout << "\tLoaded regions." << endl;
	}
	else {
		cout << "Unable to open region file." << endl;
		return 0;
	}
	return 1;
}

/******************************************************************************/
int saveRegions (string file){
	if(VERBOSE)
		cout << "Saving regions." << endl;

	ofstream outputFile;
	string filename = projectPath + "/" + file;

	// delete the file if it exists
	ifstream f(filename.c_str());
	if (f.good()) {
		f.close();
		if( remove( filename.data() ) != 0 ){
			cout << "\tError deleting " << filename.data() << endl;
			return 0;
		}
	}

	outputFile.open(filename.data());
	outputFile << "# Regions: (x y z radius)" << endl;

	for(unsigned int region = 0; region < trackModel->regions.size(); region ++){
		outputFile << trackModel->regions[region].label <<
				" " << trackModel->regions[region].centre.x <<
				" " << trackModel->regions[region].centre.y <<
				" " << trackModel->regions[region].centre.z <<
				" " << trackModel->regions[region].radius << endl;
	}
	outputFile.close();
	return 1;
}
/******************************************************************************/

int updateConfig(string file, string parameter, string value){

	string line, type;
	string filename = projectPath + "/" + file;

	cout << "Updating " << parameter << " parameter in " << filename << " to " << value << endl;


	ifstream infile (filename.data());
	stringstream content;

	// read data from the original configuration file
	if (infile.is_open())
	{
		while ( infile.good() )
		{
			// read each line. If the line defines the parameter, change the value
			getline(infile,line);
			if(line.find(parameter) != string::npos){
				string newline = line.replace(line.find(" ") + 1,line.length() - line.find(" "),value);
				line = newline;
			}
			// load each line onto the content stringstream
			content << line << endl;
		}
		// done with the original now
		infile.close();
	}
	else {
		cout << "Unable to open configuration file." << endl;
		return 0;
	}

	// reopen the file now as an output file
	ofstream outfile;

	outfile.open(filename.data());
	if(outfile.is_open())
	{
		// write the new contents to the file
		outfile << content.rdbuf();
	}

	// done
	outfile.close();

	return 1;
}

int setTreeConfig ()
{
	cout << "Setting up tree training parameters from configuration." << endl;

	for (int i = 0; i < trackModel->bodyParts.count; i++)
	{
		forest[0]->parts.classify.push_back(trackModel->bodyParts.classify[i]);
		forest[0]->parts.colours.push_back(trackModel->bodyParts.colours[i]);
		forest[0]->parts.label.push_back(trackModel->bodyParts.label[i]);
	}

	forest[0]->parts.bgColour = trackModel->bodyParts.bgColour;

	cout << "\tLoaded " << trackModel->bodyParts.count << " labels from tree configuration." << endl << endl;
	forest[0]->parts.count = trackModel->bodyParts.count;
	return 0;

}
/******************************************************************************/

// return a BGR value based on body part
Point3_<uchar> partToColour(DTree* checkTree, int labelIndex){

	return checkTree->parts.colours[labelIndex];

}
/******************************************************************************/

std::string trialDate(void)
{
	time_t now;
	int maxDate = 20;
	char the_date[maxDate];

	the_date[0] = '\0';

	now = time(NULL);

	if (now != -1)
	{
		strftime(the_date, maxDate, "%y-%m-%d_", localtime(&now));
	}

	return std::string(the_date);
}
/******************************************************************************/

// check if a point is inside a spherioid
bool inSpheriod(Point3_<float> &A, Point3_<float> &B, float radius){

	// spheroid is centred a point (d,e,f) with rx = ry = a, and rz = c
	// point (x,y,z) is inside spheroid if ( (x-d)^2/a^2 + (y-e)^2/b^2 + (z-f)^2/c^2 <= 1
	// we assume that rx = ry = radius and rz = radius/2
	float xdist = pow(A.x-B.x,2)/pow(radius,2);
	float ydist = pow(A.y-B.y,2)/pow(radius,2);
	float zdist = pow(A.z-B.z,2)/pow(radius/2,2);
	if( xdist + ydist + zdist <= 1.0 )
		return true;
	else
		return false;

}
/******************************************************************************/

void initGrabber(cameraSource typ, int ncols, int nrows, ROI roi, unsigned int size){

	cout << "Initializing grabber." << endl;

	// TODO add something here to release the grabber if it already exists before initializing it again
	grabber = new Grabber(typ, 640, 480, roi, BUFFER_SIZE);

	if (grabber->source == DISK || grabber->source == NONE){
		if(loadCamConfig("/configCamera.txt")){				// Load camera parameters for conversion between real-world and projective coordinates
			if(VERBOSE)
				cout << "Camera configuration file loaded successfully." << endl;
		}
		else
		{
			cerr << "Camera configuration file could not be opened. Shutting down." << endl;
			exit(0);
		}
	}
	else{
		saveCamConfig("/configCamera.txt");

		if(VERBOSE)
			cout << "Camera configuration updated." << endl;
	}
}
/******************************************************************************/

int initBackground(const string path){

	grabber->background = 0;
	if(grabber->source == DISK){
		// clear the background images
		grabber->bgImage.release();
		grabber->bgInvalidMask.release();

		// get a list of all the files in the source directory
		vector<string> sourceList;
		listFiles(runConf.diskpath, sourceList, "png");

		// check all files in source directory for bg images
		for(unsigned int i = 0; i < sourceList.size(); i++){

			string imPath = path + "/" + sourceList[i];

			// see if the current file is the background image
			if(sourceList[i].find("bgImage")!= string::npos){
				grabber->bgImage = imread(imPath,CV_LOAD_IMAGE_UNCHANGED);
			}
			// see if the current file is the background image invalid mask
			else if(sourceList[i].find("bgInvalidMask")!= string::npos){
				grabber->bgInvalidMask = imread(imPath,CV_LOAD_IMAGE_UNCHANGED);
			}
		}
		if(!grabber->bgImage.empty() && !grabber->bgInvalidMask.empty()){
			grabber->background = 1;
			return 1;
		}
		else{
			cout << "Could not find the background image and/or background image mask." << endl;
			return 0;
		}
	}
	else if(grabber->source == NONE){
		// Do nothing
	}
	// if a camera is connected, set up background
	else if (grabber->isCamera && !grabber->background){
		grabber->fillBgBuffer(depthIm, depthMask);
		grabber->createMask();
		return 1;
	}

	return 0;
}
/******************************************************************************/

// update GUI image window
void refreshImageWindows(){

	switch (lImg)
	{
	case 0:						// Update left image window with new RGB images
	{
		Mat outImage = Mat::zeros(Size(WIDTH,HEIGHT), CV_8UC3);
		if(!rgbIm.empty()){
			cvtColor(rgbIm,outImage,CV_RGB2BGR);

		}
		if(dispRegions)
			drawRegions(outImage);

		// overlay part proposals
		drawPartProposals(outImage);

		highlightRegions(outImage);

		showImage(gtkLImage, outImage);

		ostringstream path;

		//if(annotating){
		//	cvtColor(outImage,outImage, CV_BGR2RGB);
		//	path << grabber->path << "/full/" << setfill('0') << setw(5) << grabber->cur_image << ".png";
		//	imwrite(path.str(), outImage);
		//}
	}
	break;
	case 1:
	{
		Mat outImage = Mat::zeros(Size(WIDTH,HEIGHT), CV_8UC3);
		if(!depthIm.empty()){
			Mat depthDisp;
			depthIm.convertTo(depthDisp, CV_8UC1, OPENNI_ALPHA, OPENNI_BETA);
			Mat in[] = {depthDisp, depthDisp, depthDisp};
			merge(in, 3, outImage);

		}
		showImage(gtkLImage, outImage);
	}
	break;
	default:
		break;
	}

	switch (rImg)
	{
	case 0:		// Skeleton image
	{
		Mat outImage = Mat::zeros(Size(WIDTH,HEIGHT), CV_8UC3);

		// if annotating, use this window to show the depth image masked by the foreground image
		if(annotating || setRegs){
			if(!depthIm.empty() && !foreIm.empty()){
				Mat depthDisp, foreImNorm;
				depthIm.convertTo(depthDisp, CV_8UC1, OPENNI_ALPHA, OPENNI_BETA);
				divide(foreIm, foreIm, foreImNorm);
				multiply(depthDisp, foreImNorm, depthDisp);
				Mat in[] = {depthDisp, depthDisp, depthDisp};
				merge(in, 3, outImage);

				// update the image with the tracking model ground truth centres
				drawPartPoints(trackModel->gtPartCentres, outImage);
			}
		}
		else if (grabber->background != 1){
			if(!depthIm.empty()){
				Mat depthDisp;
				depthIm.convertTo(depthDisp, CV_8UC1, OPENNI_ALPHA, OPENNI_BETA);
				Mat in[] = {depthDisp, depthDisp, depthDisp};
				merge(in, 3, outImage);
			}
		}
		else {	// otherwise, use this window to show the skeleton image
			if(!foreIm.empty()){
				// Display the contours of the foreground image
				drawForegroundOutline(foreIm, outImage);

				// overlay part proposals
				drawPartProposals(outImage);

				// overlay random samples
				if(allSamples)
					drawSamples(outImage);
			}
		}

		showImage(gtkRImage, outImage);

	}
	break;
	case 1:		// background mask
		if (grabber->isCamera){
			Mat bgImageDisp;
			grabber->bgImage.convertTo(bgImageDisp, CV_8UC1, OPENNI_ALPHA, OPENNI_BETA);
			Mat in[] = {bgImageDisp, bgImageDisp, bgImageDisp};
			Mat mask3ch(cv::Size(WIDTH,HEIGHT), CV_8UC3);
			merge(in, 3, mask3ch);
			showImage(gtkRImage, mask3ch);
		}
		break;
	case 2:		// Foreground image with background pixels coloured according to config.txt
	{
		Mat in[] = {foreIm, foreIm, foreIm};
		Mat foreIm3ch(cv::Size(WIDTH,HEIGHT), CV_8UC3);
		merge(in, 3, foreIm3ch);

		// overlay part proposals
		drawPartProposals(foreIm3ch);

		showImage(gtkRImage, foreIm3ch);
	}
	break;
	default:
		break;
	}

}
/******************************************************************************/

int loadDepthImageDirectory(){
	if(grabber->source == DISK){
		grabber->isCamera = 1;
		listFiles(grabber->depthPath, grabber->depthList, "png");
		grabber->num_images = grabber->depthList.size();
		cout << "\tProcessing " << grabber->num_images << " images." << endl;
	}

	return 1;
}
/******************************************************************************/

// update disk files in grabber, and optionally load ground truth part centres or init gt file
void updateGrabber(bool loadGT, string valParentDir){
	// update the diskpath in the grabber
	grabber->setDiskPath(runConf.diskpath, valParentDir);

	// load the depth image list
	loadDepthImageDirectory();

	// load background
	if(!initBackground(runConf.diskpath))
		cout << "Failed to initialize background." << endl;

	if(loadGT){
		if(!loadGroundTruthCentres(runConf.diskpath)){
			cout << "\tCreating ground truth part centres file." << endl;
			exit(0);		// TEMP remove this
			initGroundTruthPartCentres();
			string title = "created by gtkAnnotationMode for use with manual part centre annotation.";
			saveGroundTruthPartCentres(runConf.diskpath, title, grabber->depthList);
			cout << "\tGround truth part centre file created." << endl;
		}
		if(!loadActions(trackModel->classHandAction, runConf.diskpath))
			trackModel->classHandAction.clear();
	}
}
/******************************************************************************/

// draw the outline of the foreground image onto a 3-channel image
void drawForegroundOutline(Mat& foreImage, Mat& outImage){

	vector<vector<Point> > curContour;

	// copy the original foreImage because it is destroyed by findContours
	Mat foreImageThresh = foreImage.clone();

	// find contours of foreground blob for display
	findContours( foreImageThresh, curContour, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

	/// Draw contours
	Mat outline = Mat::zeros( Size(WIDTH, HEIGHT), CV_8UC1 );
	for(unsigned int j = 0; j < curContour.size(); j++ )
		drawContours( outline, curContour, j, Scalar(255, 0, 0));

	// create three channel skeleton image output
	Mat in[] = {outline, outline, outline};
	Mat outline3ch;
	merge(in, 3, outline3ch);

	add(outImage, outline3ch, outImage);
}
/******************************************************************************/

// draw the part proposals (or all modes) onto a 3-channel image
void drawPartProposals(Mat& outputImage){

	for (int part = 0; part < trackModel->bodyParts.count; part++){
		Scalar colour = Scalar( trackModel->bodyParts.colours[part].z, trackModel->bodyParts.colours[part].y ,trackModel->bodyParts.colours[part].x );
		if(trackModel->bodyParts.classify[part]){//) && model->partConf[part] != 0.0){
			if(allModes){		// display all modes
				vector<Point3_<float> > modesProj (trackModel->modes[part]);
				// convert modes from world coordinates to projective space
				grabber->convertRealToProj((int)trackModel->modes[part].size(),&trackModel->modes[part][0],&modesProj[0]);
				for(unsigned int mode = 0; mode < trackModel->modes[part].size(); mode ++){
					circle( outputImage, Point(modesProj[mode].x, modesProj[mode].y), min(5,(int)(trackModel->modeConf[part][mode]/2) ) , colour, -1, 8, 0 );
				}
			}
			else
				circle( outputImage, Point(trackModel->partCentres[part].x, trackModel->partCentres[part].y), 5, colour, -1, 8, 0 );
		}
	}
}
/******************************************************************************/

// draw a vector of points of size bodyParts.count for each body part
void drawPartPoints(vector<vector<Point3_<float> > >& gtPartCentres,  Mat& outputImage){


	Point3_<float> pointsProj;		// coordinates of a point in projective space

	for (int part = 0; part < trackModel->bodyParts.count; part++){
		Scalar colour = Scalar( trackModel->bodyParts.colours[part].z, trackModel->bodyParts.colours[part].y ,trackModel->bodyParts.colours[part].x );
		if(trackModel->bodyParts.classify[part]){
			if(gtPartCentres[part][grabber->cur_image] != Point3_<float>(0,0,-1)){
				grabber->convertRealToProj(1,&gtPartCentres[part][grabber->cur_image],&pointsProj);
				circle( outputImage, Point(pointsProj.x, pointsProj.y), 5, colour, -1, 8, 0 );
			}
		}

	}
}
/******************************************************************************/

// draw the randomly proposed samples onto a 3-channel image
void drawSamples(Mat& outImage){

	for (unsigned int sample = 0; sample < trackModel->samples.size(); sample ++){
		outImage.at<Vec3b>(trackModel->samples[sample])[0] = (255);
		outImage.at<Vec3b>(trackModel->samples[sample])[1] = (255);
		outImage.at<Vec3b>(trackModel->samples[sample])[2] = (255);
	}

}
/******************************************************************************/

// draw currently configured regions onto a 3-channel image
void drawRegions(Mat& outImage){

	Scalar colour = Scalar( 255, 255, 255 );
	for (unsigned int region = 0; region < trackModel->regions.size(); region ++){
		circle( outImage, Point(trackModel->regions[region].centre.x, trackModel->regions[region].centre.y), trackModel->regions[region].radius, colour, 1, 8, 0 );

	}
}
/******************************************************************************/

// brighten the relevant region by RGB(100,100,100)
void highlightRegions(Mat& outImage){

	int highlights;											// number of separate regions to highlight
	Mat overlay = Mat::zeros(outImage.size(),CV_8UC3);

	if(curLeftStep == curRightStep)
		highlights = 1;
	else
		highlights = 2;

	int region;
	for(int count = 0; count < highlights; count ++){
		if (count == 0)
			region = curLeftStep;
		else
			region = curRightStep;

		switch(region){
		case 2:
			circle(overlay, Point(trackModel->regions[1].centre.x, trackModel->regions[1].centre.y), trackModel->regions[1].radius, Scalar(100,100,100),-1);
			break;
		case 3:
			circle(overlay, Point(trackModel->regions[2].centre.x, trackModel->regions[2].centre.y), trackModel->regions[2].radius, Scalar(100,100,100),-1);
			break;
		case 4:						// Taps, so higlight left and right taps
			circle(overlay, Point(trackModel->regions[4].centre.x, trackModel->regions[4].centre.y), trackModel->regions[4].radius, Scalar(100,100,100),-1);
			circle(overlay, Point(trackModel->regions[4].centre.x, trackModel->regions[5].centre.y), trackModel->regions[5].radius, Scalar(100,100,100),-1);
			break;
		case 5:
			circle(overlay, Point(trackModel->regions[3].centre.x, trackModel->regions[3].centre.y), trackModel->regions[3].radius, Scalar(100,100,100),-1);
			break;
		case 6:
			circle(overlay, Point(trackModel->regions[6].centre.x, trackModel->regions[6].centre.y), trackModel->regions[6].radius, Scalar(100,100,100),-1);
			break;
		default:
			break;
		}
	}

	add(outImage,overlay,outImage);
}
/******************************************************************************/

// 3D distance between two points
float distance3D(Point3_<float> &A, Point3_<float> &B){

	return cv::norm(A-B);

}
/******************************************************************************/
/*Update the tracking model:
 * weightings: weighted PDF of each pixel that was successfully classified (PDF*(depth^2))
 * worldPtsVec: position of each classified point in world space
 * startPoints: all classified points that have a PDF above the cutoff threshold for each part, used for starting mode find
 * From Shotton et. al 2011, "Real-time human pose recognition in parts from single depth images"
 */
void modeFind(Mat& depthMap){

	Point3_<float> zero(0,0,0);

	for (int part = 0; part < trackModel->bodyParts.count; part++){

		trackModel->partConf[part] = 0.0;
		trackModel->modes[part].clear();
		trackModel->modeConf[part].clear();

		if (trackModel->bodyParts.classify[part] && trackModel->startPoints[part].size() > 0){
			vector<Point3_<float> > shift(trackModel->startPoints[part].size());		// mean shifted points for the current part
			vector<float> confidence (trackModel->startPoints[part].size()); 	// confidence estimate for each mode
			float h = trackModel->bodyParts.bandwidths[part];						// Gaussian estimator bandwidth for current body part

			// TODO openmp for this shift
			// shift all starting points to a mode
			for(unsigned int i = 0; i < trackModel->startPoints[part].size(); i++){

				int done = 0;

				Point3_<float> lastPoint;					// point holder for the previous mean shifted point to calculate delta
				shift[i] = trackModel->startPoints[part][i];
				// shift until point converges on mode
				while (!done){


					lastPoint = shift[i];	// store the last value
					float denom = 0;
					float kernel, weight, scale;
					shift[i] = zero;		// initialize the mean shifted point
					confidence[i] = 0.0;

					/* TODO: implement a nearest neighbour approach because Gaussian kernel is almost zero for objects far away.
					 * To do this, sort pixels by row or column. Then, restrict the search to bandwidth/2 rows or columns (depending)
					 * and then restrict the other dimension by bandwidth/2 as well
					 */

					for(unsigned int k = 0; k < trackModel->weightings.size(); k++){

						kernel = exp ( -pow( ( distance3D( lastPoint,trackModel->worldPtsVec[k] ) / h ) , 2) );		// calculate the Gaussian kernel
						weight = trackModel->weightings[k].at<float>(0,part);											// histogram probability (PDF) * depth^2
						scale = weight * kernel;

						// calculate the amount this point contributes to the shift according to the weighted Gaussian kernel and update the shift value for each component
						shift[i].x = shift[i].x + scale*trackModel->worldPtsVec[k].x;
						shift[i].y = shift[i].y + scale*trackModel->worldPtsVec[k].y;
						shift[i].z = shift[i].z + scale*trackModel->worldPtsVec[k].z;
						// TODO should this be denom = denom + kernel?
						denom = denom + scale;
						// TODO should this be just the sum of the pixel weights?
						confidence[i] = confidence[i] + scale;
					}

					// scale each component's shift by the denominator. Note that denom will never be zero because the start point will always be found
					shift[i].x = shift[i].x/denom;
					shift[i].y = shift[i].y/denom;
					shift[i].z = shift[i].z/denom;

					// calculate shift
					float delta = distance3D(lastPoint, shift[i]);

					if (delta < 2){			// Point reached a mode
						done = 1;
					}
				}	// while(!done)

			}// starting points

			// TODO keep the point with the highest confidence for each mode
			// TODO consider some sort of clustering for this. Modes may drift away.
			// loop through starting points to identify what modes the mean-shift converged on
			for(unsigned int i = 0; i < shift.size(); i++){
				// see if this mode is new or the same as a previously found mode
				for(unsigned int j = 0; j <= trackModel->modes[part].size(); j++){
					if (j == trackModel->modes[part].size()){	// new mode found
						trackModel->modes[part].push_back(shift[i]);
						trackModel->modeConf[part].push_back(confidence[i]);
						break;
					}
					if (distance3D(shift[i], trackModel->modes[part][j]) < 2){	// mode already found
						break;
					}
				}
			}

			double testMax;
			int maxIdx[2];

			// find the mode with the largest confidence estimate
			minMaxIdx(trackModel->modeConf[part], 0, &testMax, 0, maxIdx);

			Point3_<float> result;
			// convert the mode with the largest confidence estimate back to projective coordinates
			grabber->convertRealToProj(1,&trackModel->modes[part][maxIdx[1]],&result);

			// use mode with largest confidence estimate for centre of part estimate
			trackModel->partCentres[part] = result;
			trackModel->partConf[part] = trackModel->modeConf[part][maxIdx[1]];
			trackModel->nullProposals[part] = 0;								// a proposal was made so reset null proposal counter
		}
		else{
			trackModel->nullProposals[part] ++;	// increment null proposal counter
			trackModel->partConf[part] = 0.0;	// don't use this part
		}// if(classify[part] && startpoints[part].size() > 0)
	}// part
}
/******************************************************************************/

// implement temporal filter
void filterPartProposals(){

	if(enableFilters){
		// Update Kalman Filters and draw object centres on image
		for (int part = 0; part < trackModel->bodyParts.count; part++){
			if(trackModel->bodyParts.classify[part]){

				// Estimate state at current timestamp
				Mat prediction = KFilters[part]->KF.predict();

				// Capture predicted point (x,y,z)
				Point3_<float> predictPos(prediction.at<float>(0),prediction.at<float>(1),prediction.at<float>(2));
				Point3_<float> predictVel(prediction.at<float>(3),prediction.at<float>(4),prediction.at<float>(5));

				// If classifier provided a new proposal for the part position, use it to correct the kalman filter. Otherwise
				// correct the Kalman filter with the predicted part position
				if( trackModel->partConf[part] != 0.0){
					KFilters[part]->measurement(0) = trackModel->partCentres[part].x;		// x
					KFilters[part]->measurement(1) = trackModel->partCentres[part].y;		// y
					KFilters[part]->measurement(2) = trackModel->partCentres[part].z;		// z
				}
				else{
					KFilters[part]->measurement(0) = predictPos.x;		// x
					KFilters[part]->measurement(1) = predictPos.y;		// y
					KFilters[part]->measurement(2) = predictPos.z;		// z
				}
				// for this model the velocity is always estimated by the filter, not measured by the system, so the measurement is zero
				KFilters[part]->measurement(3) = 0;					// dx
				KFilters[part]->measurement(4) = 0;					// dy
				KFilters[part]->measurement(5) = 0;					// dz


				// Update the predicted state based on measurement
				Mat estimated = KFilters[part]->KF.correct(KFilters[part]->measurement);

				// Capture updated position and velocity into points
				Point3_<float> estimatedPos(estimated.at<float>(0),estimated.at<float>(1), estimated.at<float>(2));
				Point3_<float> estimatedVel(estimated.at<float>(3),estimated.at<float>(4), estimated.at<float>(5));

				// store the updated position
				trackModel->partCentres[part] = estimatedPos;
			}
		}
	}
}
/******************************************************************************/

void findAction(Point3_<int> lHand, Point3_<int> rHand, Point& rawHandAction, Point& conditionedHandAction){

	Point3_<float> LHandPosition = Point3_<float>(lHand.x, lHand.y, lHand.z);
	Point3_<float> RHandPosition = Point3_<float>(rHand.x, rHand.y, rHand.z);

	// set initial distances to the maximum distance possible
	float LHMinDist = sqrt(pow(WIDTH,2) + pow(HEIGHT,2) + pow(256,2));
	float RHMinDist = sqrt(pow(WIDTH,2) + pow(HEIGHT,2) + pow(256,2));

	// set the hands to AWAY
	trackModel->handAction.x = 0;			// left hand equal to AWAY
	trackModel->handAction.y = 0;			// right hand equal to AWAY

	rawHandAction.x = 0;
	rawHandAction.y = 0;

	// Loop through each tracking region to determine if left and right hands are in any region
	// Hand positions are set to the region the hand is closest to
	for(unsigned int region = 0; region < trackModel->regions.size(); region ++){
		// don't process the AWAY and SINK regions. They are assigned later if necessary
		if(trackModel->regions[region].label != "AWAY" && trackModel->regions[region].label != "SINK"){
			Point3_<float> curRegion = Point3_<float>(trackModel->regions[region].centre.x, trackModel->regions[region].centre.y, trackModel->regions[region].centre.z);
			float LHandDistance = distance3D(LHandPosition, curRegion);
			float RHandDistance = distance3D(RHandPosition, curRegion);

			// check if the hands lie within a spheroid bounding a region and are closer than any other region so far
			if(inSpheriod(LHandPosition, curRegion, trackModel->regions[region].radius) && LHandDistance < LHMinDist){
				rawHandAction.x = region;
				LHMinDist = LHandDistance;
			}
			if(inSpheriod(RHandPosition, curRegion, trackModel->regions[region].radius) && RHandDistance < RHMinDist){
				rawHandAction.y  = region;
				RHMinDist = RHandDistance;
			}
		}
	}

	// now we get into the funny stuff. This is an adaptation of extractObservations from COACH handtracker code
	// It basically prioritizes steps in the following order: SOAP, TAP, WATER, TOWEL and sets the opposite hand to SINK if the two hands don't agree

	// strings to hold left and right steps (to be converted to region index)
	string lHandStep = "AWAY";
	string rHandStep = "AWAY";

	// consolidate L_TAP and R_TAP into L_TAP to make things easier
	if(trackModel->regions[rawHandAction.x].label == "L_TAP" || trackModel->regions[rawHandAction.x].label == "R_TAP"){
		// TODO find the offset for L_TAP rather than hard coding this
		rawHandAction.x = 4;		// set to L_TAP
	}
	if(trackModel->regions[rawHandAction.y].label == "L_TAP" || trackModel->regions[rawHandAction.y].label == "R_TAP"){
		// TODO find the offset for L_TAP rather than hard coding this
		rawHandAction.y = 4;		// set to L_TAP
	}

	if(rawHandAction.x != rawHandAction.y){					// left and right hands are not the same, so we choose from a set of existing states
		if(trackModel->regions[rawHandAction.x].label == "SOAP"){															// left hand is at soap, so right hand is forced to sink
			lHandStep = "SOAP";
			rHandStep = "SINK";
		}
		else if(trackModel->regions[rawHandAction.y].label == "SOAP"){														// right hand is at soap, so left hand is forced to sink
			rHandStep = "SOAP";
			lHandStep = "SINK";
		}
		else if(trackModel->regions[rawHandAction.x].label == "L_TAP"){														// left hand is at tap so right hand is forced to water, towel or sink
			lHandStep = "TAP";
			if(trackModel->regions[rawHandAction.y].label == "WATER")
				rHandStep = "WATER";
			else if (trackModel->regions[rawHandAction.y].label == "TOWEL")
				rHandStep = "TOWEL";
			else
				rHandStep = "SINK";
		}
		else if(trackModel->regions[rawHandAction.y].label == "L_TAP"){														// right hand is at tap so left hand is forced to water, towel or sink
			rHandStep = "TAP";
			if(trackModel->regions[rawHandAction.x].label == "WATER")
				lHandStep = "WATER";
			else if (trackModel->regions[rawHandAction.x].label == "TOWEL")
				lHandStep = "TOWEL";
			else
				lHandStep = "SINK";
		}
		else if(trackModel->regions[rawHandAction.x].label == "WATER"){														// left hand is at water so right is forced to towel or sink
			lHandStep = "WATER";
			if(trackModel->regions[rawHandAction.y].label == "TOWEL")
				rHandStep = "TOWEL";
			else
				rHandStep = "SINK";
		}
		else if(trackModel->regions[rawHandAction.y].label == "WATER"){														// right hand is at water so left is forced to towel or sink
			rHandStep = "WATER";
			if(trackModel->regions[rawHandAction.x].label == "TOWEL")
				lHandStep = "TOWEL";
			else
				lHandStep = "SINK";
		}
		else if(trackModel->regions[rawHandAction.x].label == "TOWEL"){														// left hand is at towel so right hand is forced to sink
			lHandStep = "TOWEL";
			rHandStep = "SINK";
		}
		else if(trackModel->regions[rawHandAction.y].label == "TOWEL"){														// right hand is at towel so left hand is forced to sink
			rHandStep = "TOWEL";
			lHandStep = "SINK";
		}
	}
	else{
		if(trackModel->regions[rawHandAction.x].label == "L_TAP")
			lHandStep = "TAP";
		else
			lHandStep = trackModel->regions[rawHandAction.x].label;

		if(trackModel->regions[rawHandAction.y].label == "L_TAP")
			rHandStep = "TAP";
		else
			rHandStep = trackModel->regions[rawHandAction.y].label;
	}

	// set final hand position according to the run configuration for activities.
	for(unsigned int act = 0; act < runConf.activities.size(); act++){
		if(lHandStep == runConf.activities[act])
			conditionedHandAction.x = act;
		if(rHandStep == runConf.activities[act])
			conditionedHandAction.y = act;
	}
}
/******************************************************************************/

/* Classify each pixel on the foreground as one of the body parts in the decision tree
 * foreim is the binary foreground image where pixels = 255 for foreground image and 0 for background
 * depthmap is the depth image (16-bit)
 */

int classifyParts(int numTrees, Mat& foreim, Mat &depthMap){

	int numPoints = trackModel->samples.size();

	trackModel->weightings.clear();										// vector of histograms of each point
	vector<Point3_<float> > projPtsVec;							// vectors for classified points in projective and world space
	Point3_<float> projPoint;									// temporary storage for projective point

	vector<vector<int> > startPointOffsets(trackModel->bodyParts.count);		// position of start points in projection point array

	// find PDF for each pixel that is NOT a background pixel
	for (int i = 0; i < numPoints; i++){

		// Only process points that are on the foreground image
		if(foreim.at<uchar>(Point(trackModel->samples[i].x, trackModel->samples[i].y)) == 255){

			float treeCount = 0.0;
			Mat avgHist = Mat::zeros(1,trackModel->bodyParts.count,CV_32FC1);

			for (int f = 0; f < numTrees; f++){

				// find PDF for pixel (sample) or return NULL
				treeNode* found = forest[f]->Query(trackModel->samples[i], foreim, depthMap);		// query the tree for the leaf node

				// check if pixel was properly classified
				if (found != NULL){
					avgHist = avgHist + found->hist;		// accumulate all valid PDFs
					treeCount +=1.0f;
				}
			}

			// check if at least one tree could classify the point
			if(treeCount != 0 ){

				// TODO consider weighting this so that "more confident" trees have a larger influence than less confident trees
				divide(avgHist,Scalar::all(treeCount),avgHist);			// find average PDF of all trees that classified the point

				int index = trackModel->weightings.size();							// index of the last element

				// loop through PDF and store position of sample in associated part vector if it's above threshold probability (for starting points of mean shift mode search)

				for (int part = 0; part < trackModel->bodyParts.count; part++){
					if(avgHist.at<float>(0,part) > trackModel->bodyParts.probThreshs[part]){
						startPointOffsets[part].push_back(index);
					}
				}

				// find the depth of this pixel
				float dI = depthMap.at<ushort>(Point(trackModel->samples[i].x,trackModel->samples[i].y))/1000.0;	// find depth of pixel (x,y) in mm*10 and convert to meters

				// calculate the pixel weightings for each part (probability * depth^2)
				cv::multiply(avgHist, Scalar::all(dI*dI), avgHist);

				trackModel->weightings.push_back(avgHist);

				// store classification pixel for conversion to world space
				projPoint.x = trackModel->samples[i].x;
				projPoint.y = trackModel->samples[i].y;
				projPoint.z = dI * 100;					// depth for conversion to world space must be in mm
				projPtsVec.push_back(projPoint);

			} // if (treeCount != 0)
		}
	} // for

	// resize destination vector
	trackModel->worldPtsVec.resize(projPtsVec.size());

	// convert projective pixels to world space for mode find
	grabber->convertProjToReal(projPtsVec.size(), &projPtsVec[0], &trackModel->worldPtsVec[0]);

	// find the world space start points for mode find of each body part
	for(int part = 0; part < trackModel->bodyParts.count; part++){
		trackModel->startPoints[part].clear();
		for(unsigned int i = 0; i < startPointOffsets[part].size(); i++)
			trackModel->startPoints[part].push_back(trackModel->worldPtsVec[startPointOffsets[part][i]]);
	}

	return 1;

}
/******************************************************************************/

void savePng(string dir, int frame, Mat& outimage){

	ostringstream path;

	path << trialPath << "/camera/" << dir << "/" << dir << setfill('0') << setw(5) << frame << ".png";
	imwrite(path.str(), outimage);

}
/******************************************************************************/

/* Find per-class error metrics for each tree in forest (raw pixel classification accuracy)
 * dirCheck: directory of holdback images to test error metrics on
 * num_trees: number of trees in forest
 * tree_num: tree offset in forest if only a single tree is being checked
 * debug: set to 1 for verbose output
 */
int forestPerClassMetrics(string dirCheck, int num_trees, int tree_num, int debug){

	string dirLabel = dirCheck + "/label";
	string dirDepth = dirCheck + "/depth16bit";
	string labelCheckStr, depthCheckStr;

	vector<string> labelCheck,depthCheck;

	float invPixels = 0;
	int num_imgs;

	// load sorted list of rgb (training) and depth images
	listFiles(dirLabel, labelCheck, "png");
	listFiles(dirDepth, depthCheck, "png");

	if ( labelCheck.size() != depthCheck.size() ){
		cout << "Label and depth validation images don't have the same count." << endl;
		return 1;
	}
	else{
		num_imgs = labelCheck.size();
	}

	for (int num_im=0 ; num_im < num_imgs; num_im++){
		// compare each image pair to make sure they have the same five digit identifier
		if (labelCheck[num_im].compare(labelCheck[num_im].size()-9,5,depthCheck[num_im],depthCheck[num_im].size()-9,5) != 0){
			cout << "Training image mismatch on images " << labelCheck[num_im] << " and " << depthCheck[num_im] << endl;
			return 1;
		}
	}

	if (num_trees == 1){
		if (tree_num >= 0){
			cout << "Checking accuracy of tree " << tree_num << " with " << num_imgs << " images." << endl;
		}
		else{
			cout << "Requested metrics on an invalid tree number." << endl;
			return 1;
		}
	}
	else if (num_trees < 1){
		cout << "Requested metrics on less than 1 tree." << endl;
		return 1;
	}
	else {
		cout << "Requested metrics on a forest of " << num_trees << " trees with " << num_imgs << " images." << endl;
		tree_num = 0;
	}

	Mat confusionMatrix = Mat::zeros(trackModel->bodyParts.count, trackModel->bodyParts.count,CV_32SC1);

	Mat labelImage, depthMap;

	cout << "Testing with " << RUN_PIXELS << " pixels per image." << endl;

	// Main image loop for performance metrics
	for (int imgs = 0; imgs < num_imgs; imgs++){

		labelCheckStr = dirLabel + "/" + labelCheck[imgs];
		depthCheckStr = dirDepth + "/" + depthCheck[imgs];
		labelImage = imread(labelCheckStr.data(),1);  	// Load as 3 channel colour image
		depthMap = imread(depthCheckStr.data(),-1);		// Load as 16 bit image

		// create a binary foreground image from the label image by normalizing the image and setting all non-zero pixels to 255
		Mat foreImage, labelGray;
		cvtColor(labelImage,labelGray,CV_RGB2GRAY);
		threshold(labelGray, foreImage, 0.0, 255.0, THRESH_BINARY);

		// create a new random set of pixels
		createSamples(trackModel->samples, RUN_PIXELS);


		// -------------------------------------------------------------------------------------------------
		//Per-class accuracy calculation setup and data collection
		// -------------------------------------------------------------------------------------------------

		for (unsigned int i = 0; i < trackModel->samples.size() ; i++){

			float treeCount = 0.0;
			Mat avgHist = Mat::zeros(1,trackModel->bodyParts.count,CV_32FC1);

			// only check pixels on the foreground image
			if(foreImage.at<uchar>( trackModel->samples[i]) == 255){

				for (int f = tree_num; f < tree_num + num_trees; f++){
					treeNode* found = forest[f]->Query(trackModel->samples[i],foreImage, depthMap);		// query the tree for the leaf node

					// check if pixel was properly classified
					if (found != NULL){
						avgHist = avgHist + found->hist;		// accumulate all histograms
						treeCount +=1.0f;
					}
				}
				// check if at least one tree could classify the point
				if(treeCount != 0 ){

					divide(avgHist,Scalar::all(treeCount),avgHist);			// find average of all trees that classified the point

					double testMax;
					int maxIdx[2];
					minMaxIdx(avgHist, 0, &testMax, 0, maxIdx);

					// update the confusion matrix element reflecting the ground-truth part label and the most likely inferred part
					confusionMatrix.at<int>(maxIdx[1],colourFeature(tree_num, Point(trackModel->samples[i].x, trackModel->samples[i].y),labelCheck[imgs], labelImage)) ++;

				} // if(treeCount != 0)
				else{
					invPixels +=1.0f;
				}
			}
			else{
				invPixels +=1.0f;
			}
		}

	} // for each image

	vector<bool> relevantPart;

	// figure out which parts are relevant (i.e., which were either classified or actually exist)
	for(int part = 0; part < trackModel->bodyParts.count; part++){

		// sum up the actual occurrence of the body part
		int actual = 0;
		for(int row = 0; row < trackModel->bodyParts.count; row++)
			actual = actual + confusionMatrix.at<int>(row,part);

		// sum up the predicted occurrence of the part
		int predicted = 0;
		for(int col = 0; col < trackModel->bodyParts.count; col++)
			predicted = predicted + confusionMatrix.at<int>(part,col);

		// if at least one actual or predicted occurrence, part is relevant to calculations
		if(actual !=0 || predicted !=0)
			relevantPart.push_back(true);
		else
			relevantPart.push_back(false);
	}

	// print confusion matrix to console
	if(debug){
		cout << "Confusion Matrix:" << endl << "\t\t\tActual" << endl;

		// print header row
		for (int col = 0; col < trackModel->bodyParts.count; col ++){
			if(relevantPart[col])
				cout << setw(10) << trackModel->bodyParts.label[col];
			if(col == trackModel->bodyParts.count-1)
				cout << endl;
		}

		// print matrix
		for (int row = 0; row < trackModel->bodyParts.count; row ++){
			if(relevantPart[row]){
				for (int col = 0; col < trackModel->bodyParts.count; col ++){
					if(relevantPart[col])
						cout << setw(10) << confusionMatrix.at<int>(row,col) ;
				}
				cout << endl;
			}
		}
	}

	// -------------------------------------------------------------------------------------------------
	// Calculate per-class performance metrics based on the most likely inferred part label
	// -------------------------------------------------------------------------------------------------

	vector<float> precision(trackModel->bodyParts.count);		// Proportion of correctly classified parts out of all the instances where the algorithm classified that part
	vector<float> recall(trackModel->bodyParts.count);			// Proportion of actual positives that are correctly identified as such (also called sensitivity)
	vector<float> specificity(trackModel->bodyParts.count);		// Proportion of actual negatives that are correctly identified as such
	vector<float> accuracy(trackModel->bodyParts.count);		// Proportion of correctly classified parts

	int priorityPartCount = 0;
	int overallPartCount = 0;
	float priorityMetric = 0;
	float overallMetric = 0;

	// Calculate True Positive, True Negative, False Positive and False Negative for each body part

	for(int part = 0; part < trackModel->bodyParts.count; part++){
		float truePos = 0;
		float falsePos = 0;
		float trueNeg = 0;
		float falseNeg = 0;
		for(int row = 0; row < trackModel->bodyParts.count; row++){
			for(int col = 0; col < trackModel->bodyParts.count; col ++){
				if(part == row && part == col)
					truePos = confusionMatrix.at<int>(row,col);

				if(part == row && part != col)
					falsePos = falsePos + confusionMatrix.at<int>(row,col);

				if(part == col && part != row)
					falseNeg = falseNeg + confusionMatrix.at<int>(row,col);

				if(part != row && part != col)
					trueNeg = trueNeg + confusionMatrix.at<int>(row,col);
			}
		}

		// Calculate final precision, recall, specificity and accuracy for each body part. Set to -1 if not possible to calculate.

		if(truePos + falsePos != 0)
			precision[part] = truePos/(truePos + falsePos);
		else
			precision[part] = -1;

		if (truePos + falseNeg != 0)
			recall[part] = truePos/(truePos + falseNeg);
		else
			recall[part] = -1;

		if(trueNeg + falsePos != 0)
			specificity[part] = trueNeg/(trueNeg + falsePos);
		else
			specificity[part] = -1;

		if(truePos + trueNeg + falsePos + falseNeg != 0)
			accuracy[part] = (truePos + trueNeg)/(truePos + trueNeg + falsePos + falseNeg);
		else
			accuracy[part] = -1;

		// When considering all the parts, just use recall as a performance metric

		if(recall[part] != -1){
			overallPartCount ++;
			overallMetric = overallMetric + recall[part];
		}

		// if prioritizing training for only some parts, use the average of precision and recall for the parts that are prioritized/classified

		if (trackModel->bodyParts.classify[part]){
			priorityPartCount ++;

			// if neither was undefined, both have valid numbers so take the average of the two.
			//if either was undefined (equal to -1) then the other will be zero so we take their average as zero
			if(precision[part] != -1 && recall[part] != -1)
				priorityMetric = priorityMetric + (precision[part] + recall[part])/2;
		}
	}

	// -------------------------------------------------------------------------------------------------
	// Report the per-class error metrics
	// -------------------------------------------------------------------------------------------------

	if (overallPartCount != 0){
		cout << "\tOverall_Metric: " << fixed << setprecision(2) << overallMetric/overallPartCount*100 << endl;
		cout.unsetf(ios_base::floatfield);
	}
	else
		cout << "\tOverall_Metric: 0.0" << endl;

	if (priorityPartCount != 0){
		cout << "\tPriority_Metric: " << fixed << setprecision(2) << priorityMetric/priorityPartCount*100 << endl;
		cout.unsetf(ios_base::floatfield);
	}
	else
		cout << "\tPriority_Metric: 0.0" << endl;

	return 0;
}
/******************************************************************************/

// sort a P-R curve vector by Recall
bool sortPRByRecall (cv::Point3_<float> i, cv::Point3_<float> j) {
	return (i.y < j.y);
}
/******************************************************************************/

/* Find part proposal error metrics for forest
 * dirCheck: directory of holdback images to test error metrics on
 * num_trees: number of trees in forest
 * tree_num: tree offset in forest if only a single tree is being checked
 * debug: set to 1 for verbose output
 */
int forestPartPropMetrics(string dirCheck, int num_trees, int debug){

	cout << "Calculating forest part proposal error metrics." << endl;

	string dirLabel = dirCheck + "/label";
	string dirDepth = dirCheck + "/depth16bit";
	vector<string> labelList,depthList;

	int num_imgs;

	// load sorted list of rgb (training) and depth images
	listFiles(dirLabel, labelList, "png");
	listFiles(dirDepth, depthList, "png");

	if ( labelList.size() != depthList.size() ){
		cout << "Label and depth validation images don't have the same count." << endl;
		return 1;
	}
	else{
		num_imgs = labelList.size();
	}

	// preload images for training
	Mat labelRead, depthRead;
	string labelFile, depthFile;								// strings for foreground, label and depth image files to be loaded

	vector<Mat> labelImages;											// vector to hold pre-loaded labeled training images
	vector<Mat> depthImages;											// vector to hold pre-loaded depth images

	cout << "Preloading holdback images." << endl;
	for (int num_im=0 ; num_im < num_imgs; num_im++){
		// compare each image pair to make sure they have the same five digit identifier

		if (labelList[num_im].compare(labelList[num_im].size()-9,5,depthList[num_im],depthList[num_im].size()-9,5) != 0){
			cout << "Image mismatch between label and depth images on images " << labelList[num_im] << " and " << depthList[num_im] << endl;
			return 1;
		}

		// construct image file names
		labelFile = dirLabel + "/" + labelList[num_im];
		depthFile = dirDepth + "/" + depthList[num_im];

		// load images from file
		labelRead = imread(labelFile.data(),1);  	// Load as 3 channel colour image
		depthRead = imread(depthFile.data(),-1);	// Load as 1 channel 16-bit image

		// push images onto image vectors
		labelImages.push_back(labelRead); 			// Load as 3 channel colour image
		depthImages.push_back(depthRead);			// Load as 16 bit image

		if (!labelImages[num_im].data){
			cout << "Label image data not loaded properly" << endl;
			return 1;
		}
		if (!depthImages[num_im].data){
			cout << "Depth image data not loaded properly" << endl;
			return 1;
		}
	}

	// load ground truth part centres file
	if(!loadGroundTruthCentres(dirCheck))
		groundTruthPartCentresFromLabel(dirCheck, labelImages, depthImages, labelList);		// file doesn't exist, so find part centres from labeled images and create a file to store the ground truth part centres for this data set

	// set up metrics
	vector<float> truePos(trackModel->bodyParts.count);			// parts in ground truth image that are classified, within PROP_DIST, as the part
	vector<float> trueNeg(trackModel->bodyParts.count);			// parts not in ground truth image that are not classified as the part
	vector<float> falsePos(trackModel->bodyParts.count);			// parts in ground truth image that are classified, outside PROP_DIST or within PROP_DIST when another more than once, as the part
	vector<float> falseNeg(trackModel->bodyParts.count);			// parts in ground truth image that are not classified as the part
	vector<vector<Point3_<float> > > PRstats(trackModel->bodyParts.count);		// precision (x), recall (y) and threshold (z) stored as x, y and z of Point3_<float>

	cout << "Calculating PR curves for " << trackModel->bodyParts.count << " parts." << endl;

	for (float thresh = 0; thresh <= 1.0; thresh += 0.25){
		cout << "Threshold " << thresh << endl;

		// clear metrics and setup current threshold for precision-recall curve construction
		for (int part = 0; part < trackModel->bodyParts.count; part++){
			truePos[part]=0;
			trueNeg[part]=0;
			falsePos[part]=0;
			falseNeg[part]=0;

			trackModel->bodyParts.probThreshs[part] = thresh;
		}

		// loop through each frame and calculate
		for( int img = 0; img < num_imgs; img ++){

			// create a binary foreground image from the label image by normalizing the image and setting all non-zero pixels to 255
			Mat foreImage, labelGray;
			cvtColor(labelImages[img],labelGray,CV_RGB2GRAY);
			threshold(labelGray, foreImage, 0.0, 255.0, THRESH_BINARY);

			// create a new random set of pixels
			createSamples(trackModel->samples, RUN_PIXELS);

			// update the tracking model to obtain intermediate tracking data using the current threshold
			classifyParts(runConf.num_trees, foreImage, depthImages[img]);

			// update the tracking model to obtain the modes
			modeFind(depthIm);

			// -------------------------------------------------------------------------------------------------
			// Calculate the part proposal error metrics
			// -------------------------------------------------------------------------------------------------

			for(int part = 0; part < trackModel->bodyParts.count; part++){

				// update the tracking model with the ground truth part centre from the current image

				if(trackModel->gtPartCentres[part][img] == Point3_<float>(0,0,-1)){		// part doesn't exist in ground truth image
					if (trackModel->modes[part].size() == 0)
						trueNeg[part] +=1.0;													// classifier didn't provide proposal for this part
					else
						falsePos[part] = falsePos[part] + trackModel->modes[part].size();					// every part proposed by the classifier is a false positive
				}
				else{															// part exists in ground truth image
					if(trackModel->modes[part].size() == 0)
						falseNeg[part] +=1.0;									// classifier didn't provide any part proposals
					else{														// classifier provided at least one part proposal

						int firstTP = 1;

						// First proposal within PROP_DIST is a True Positive. Any other proposal within PROP_DIST is classed as a false positive to penalize too many modes
						for(unsigned int mode = 0; mode < trackModel->modes[part].size(); mode ++){
							float dist = distance3D(trackModel->gtPartCentres[part][img],trackModel->modes[part][mode]);
							if(dist < PROP_DIST){
								if(firstTP){									// First mode within PROP_DIST is a true positive
									truePos[part] +=1.0;
									firstTP = 0;
								}
								else
									falsePos[part] +=1.0;										// Any mode other than the first within PROP_DIST is a false positive
							}
							else
								falsePos[part] +=1.0;											// Any mode outside PROP_DIST is a false positive
						}
					}
				}
			} // body part loop
		} // image loop

		// -------------------------------------------------------------------------------------------------
		// Report the part proposal error metrics
		// -------------------------------------------------------------------------------------------------

		for (int part = 0; part < trackModel->bodyParts.count; part++){
			// only include precision-recall if they are both valid numbers
			if(truePos[part] + falsePos[part] != 0){
				if (truePos[part] + falseNeg[part] != 0){
					Point3_<float> stats;
					stats.x = truePos[part]/(truePos[part]+falsePos[part]);		// precision
					stats.y = truePos[part]/(truePos[part]+falseNeg[part]);		// recall
					stats.z = thresh;											// threshold used to find precision and recall
					PRstats[part].push_back(stats);
				}
				else
					PRstats[part].push_back(Point3_<float>(0,0,thresh));
			}
		}
	} // threshold loop

	// sort PR curves by recall
	for(int part = 0; part < trackModel->bodyParts.count; part++)
		sort (PRstats[part].begin(), PRstats[part].end(), sortPRByRecall);

	vector<float> lastRecall(trackModel->bodyParts.count,0.0);				// vector of previous recall values for each part, initialized to zero
	vector<float> averagePrecision(trackModel->bodyParts.count, 0.0);			// vectore for storage of average precision for each part

	/* print the PR data and calculate the Average Precision
	 * Note that the PR curves are sorted by Recall in ascending order. The average precision is then the sum of the (incremental recall)*(precision) overall all points
	 */
	cout << "Precision-Recall curve data." << endl;
	for (int part = 0; part < trackModel->bodyParts.count; part++){
		// only print PR curves for parts that have data
		if(PRstats[part].size() != 0){
			cout << endl << trackModel->bodyParts.label[part] << endl;
			cout << "Prec.\tRecall\tThreshold" << endl;
			for(unsigned int pair = 0; pair < PRstats[part].size(); pair++){
				cout << setprecision(6) << setw(6) << PRstats[part][pair].x << " " << setprecision(6) << setw(6) << PRstats[part][pair].y << " " << setprecision(3) << PRstats[part][pair].z << endl;
				averagePrecision[part] = averagePrecision[part] + (PRstats[part][pair].y - lastRecall[part] ) * PRstats[part][pair].x;		// adjust the averagePrecision sum
				lastRecall[part] = PRstats[part][pair].y;			// update the recall for the next iteration
			}
		}
	}

	cout << endl << "Average precision:" << endl;
	float mAP = 0.0;		// mean average precision
	float mAPCount = 0.0;
	for (int part = 0; part < trackModel->bodyParts.count; part++){
		if(PRstats[part].size()!=0){
			cout << trackModel->bodyParts.label[part] << " AP: " << averagePrecision[part] << endl;
			mAP = mAP + averagePrecision[part];
			mAPCount += 1.0f;
		}
	}
	cout << endl << "Mean average precision (mAP) = " << mAP/mAPCount << endl;
	return 0;
}
/******************************************************************************/

void showImage(GtkWidget* imwin, Mat& image){

	GdkPixbuf* pix;												// pixel buffer for image display
	pix = gdk_pixbuf_new_from_data((guchar*) image.data, GDK_COLORSPACE_RGB, FALSE, 8, image.cols, image.rows, image.step, NULL, NULL);
	gdk_draw_pixbuf(imwin->window, imwin->style->fg_gc[GTK_WIDGET_STATE (imwin)], pix, 0, 0, 0, 0, WIDTH, HEIGHT, GDK_RGB_DITHER_NONE, 0, 0);
	g_object_unref(G_OBJECT(pix));	// release the Pixbuf
}
/******************************************************************************/

string buildTrainMessage(){

	ostringstream buff, data;

	buff << "\tThresholds: " << NUM_THRESHOLDS << endl;
	buff << "\tThreshold range: -" << MAX_THRESHOLD << ":" << MAX_THRESHOLD << endl;
	buff << "\tSplit candidates: " << SPLIT_CANDIDATES << endl;
	buff << "\tMaximum candidate offset (pixel-meters): " << offset << endl;
	buff << "\tMinimum gain: " << cutoffGain << endl;
	buff << "\tPixels per image: " << TRAIN_PIXELS << endl;
	buff << "\tMaximum tree depth: " << maxDepth << endl;

	string buffer = buff.str();

	return buffer.c_str();
}
/******************************************************************************/

int createBackground( string rootpath)			// create background image (16-bit) from a set of depth images
{
	vector<string> sourceList;
	vector<Mat> depthBuffer, maskBuffer;
	Mat outDepthImage (cv::Size(WIDTH,HEIGHT),CV_16UC1);
	Mat outInvalidImage (cv::Size(WIDTH,HEIGHT),CV_16UC1);
	string sourceStr;

	string sourcepath = rootpath + "/depth16bit";
	listFiles(sourcepath, sourceList, "png");

	string bgImageFilename = sourceList[0];
	bgImageFilename.replace(bgImageFilename.find("_"), bgImageFilename.length()-bgImageFilename.find("_"),"bgImage.png");
	string targetMaskStr = rootpath + "/" + bgImageFilename;

	string bgInvalidFilename = sourceList[0];
	bgInvalidFilename.replace(bgInvalidFilename.find("_"), bgInvalidFilename.length()-bgInvalidFilename.find("_"),"bgInvalidMask.png");
	string targetInvStr = rootpath + "/" + bgInvalidFilename;

	ifstream bgImgFile(targetMaskStr.data());
	ifstream bgInvFile(targetInvStr.data());

	// if both files exist, do nothing
	if(bgImgFile.good() && bgInvFile.good()){
		cout << "Both background images exist. Moving on." << endl;
	}
	// If either file doesn't already exist we need to make them
	else {

		int maxBufferSize = 20;		// TODO make this a prompted option

		if(VERBOSE)
			cout << "Capturing " << maxBufferSize << " images into initial buffer." << endl;

		// fill up background buffer
		for (int img = 0; img < maxBufferSize; img++ ){

			sourceStr = sourcepath + "/" + sourceList[img];

			// read the 16-bit depth image into buffer
			Mat sourceIm (cv::Size(WIDTH,HEIGHT),CV_16UC1);
			sourceIm = imread(sourceStr.data(),-1);					// read depth image as 16-bit
			depthBuffer.push_back(sourceIm);

			Mat maskIm = sourceIm.clone();

			// create valid pixel mask (valid pixel = 1, invalid pixel = 0) and push onto maskBuffer
			divide(maskIm,maskIm,maskIm);

			maskBuffer.push_back(maskIm);
		}
		cout << "Initial background buffer of " << depthBuffer.size() << " created." << endl;

		Mat depth32FC1(Size(WIDTH,HEIGHT), CV_32FC1);
		Mat mask16UC1(Size(WIDTH,HEIGHT), CV_16UC1);
		Mat depthAccum = Mat::zeros(Size(WIDTH,HEIGHT), CV_32FC1);
		Mat bgValidMask = Mat::zeros(Size(WIDTH,HEIGHT), CV_16UC1);

		// Element-wise addition of all buffered background images. Only pixels that are valid (based on valid depth mask)
		// are accumulated. Final mask is the element-wise average of the valid pixels.
		for (unsigned int i = 0; i < depthBuffer.size(); i++){

			depthBuffer[i].convertTo(depth32FC1, CV_32FC1);				// convert depth image to 16UC1
			add (depth32FC1, depthAccum, depthAccum);					// add depth image to the accumulator

			divide(maskBuffer[i],maskBuffer[i],mask16UC1);				// normalize elements of the mask
			add (mask16UC1, bgValidMask, bgValidMask);					// add normalized mask to the mask accumulator
		}
		Mat avgMask32 (cv::Size(WIDTH,HEIGHT),CV_32FC1);
		bgValidMask.convertTo(avgMask32,CV_32FC1);

		// average the depth accumulator for each individual pixel
		Mat bg32 (cv::Size(WIDTH,HEIGHT),CV_32FC1);
		divide(depthAccum, avgMask32, bg32);
		bg32.convertTo(outDepthImage,CV_16UC1);

		// Normalize valid mask and then set all valid pixels to 65535 (invalid will be zero)
		divide(bgValidMask, bgValidMask, bgValidMask);
		multiply(bgValidMask, Scalar::all(65535), bgValidMask);

		// Set all invalid pixels to 65535 (valid will be zero)
		subtract(Scalar::all(65535), bgValidMask, outInvalidImage);

		cout << "Background images created from " << depthBuffer.size() << " frames." << endl;

		// if the background image was missing, write it
		if (!bgImgFile) {
			imwrite(targetMaskStr, outDepthImage);
		}
		// if the invalid pixel image was missing, write it
		if (!bgInvFile) {
			imwrite(targetInvStr, outInvalidImage);
		}
	}
	bgImgFile.close();
	bgInvFile.close();

	return 1;
}

/******************************************************************************/

int removeBackground( string rootpath, string targetpath)			// remove background image from depth images in a directory
{
	string sourcepath;
	string sourceStr, targetStr;
	vector<string> sourceList, targetList;

	sourcepath = rootpath + "/depth16bit";
	if(!listFiles(sourcepath, sourceList, "png")){
		cout << "Source directory ('depth16bit') does not exist" << endl;
		return 0;
	}
	if(sourceList.size() < 1){
		cout << "No images in source directory." << endl;
		return 0;
	}

	if(!listFiles(targetpath, targetList, "png")){
		cout << "Target directory does not exist. Creating it..." << endl;

		if(mkdir(targetpath.data(),0777) != 0){
			cout << "Error making " << targetpath << endl;
			return 0;
		}
	}
	if(targetList.size() != 0){
		cout << "Target directory " << targetpath << " is not empty." << endl;
		return 0;
	}

	// construct filename for background image
	string filename = sourceList[0];
	filename.replace(filename.find("_"), filename.length()-filename.find("_"),"bgImage.png");

	// get path to background image
	string bgimagepath = rootpath + "/" + filename;

	// read background image
	Mat bgIm = imread(bgimagepath.data(),-1);				// read background image as a 16-bit image

	// construct filename for background image invalid pixel mask
	filename = sourceList[0];
	filename.replace(filename.find("_"), filename.length()-filename.find("_"),"bgInvalidMask.png");

	// get path to background image invalid pixel mask
	string bgImageInvalidPixelPath = rootpath + "/" + filename;

	// read background image invaid pixel mask
	Mat bgImInvPixel = imread(bgImageInvalidPixelPath.data(),-1);		// read background mask as a 16-bit image

	//create an 8-bit mask for cv::add
	Mat bgImInvPixel8bit;
	bgImInvPixel.convertTo(bgImInvPixel8bit, CV_8UC1, 1.0/256.0);

	Mat sourceIm, outputIm;

	Mat depthFore16bit (Size(WIDTH, HEIGHT), CV_16UC1);		// final 16-bit depth foreground image
	Mat depthFore8bit (Size(WIDTH, HEIGHT), CV_8UC1);		// final 8-bit depth foreground image (for labeling)
	Mat depthFore8bitFull (Size(WIDTH, HEIGHT), CV_8UC3);	// final 8-bit depth foreground image (for labeling)
	Mat normalFore (Size(WIDTH, HEIGHT), CV_16UC1);		// final foreground mask
	Mat rawFore, normDepthMask, blurFore, blurThresh, threshForeContours, blobs, foreImMask;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	for (unsigned int img = 0; img < sourceList.size(); img++){

		sourceStr = sourcepath + "/" + sourceList[img];
		sourceIm = imread(sourceStr.data(),-1);				// read depth image as a 16-bit image

		// subtract background from image

		// Find foreground image.
		// Note that depth image is subtracted from background image because pixels further from the camera have a larger value.
		// Two issues must still be resolved with the foreground image:
		// 1. If the background image had any invalid pixels (value = zero), the raw foreground image will have a zero value as well
		// regardless of the pixel state in depth image.
		// 2. The raw foreground image has also overwritten any invalid pixels in the depth image that were invalid.
		subtract(bgIm, sourceIm, rawFore);

		// Deal with Issue 1: background image invalid pixels
		// Add the depth image to the raw foreground, masking for invalid background pixels only. This will overwrite any invalid
		// background image pixels that are valid in the depth image.
		add(rawFore, sourceIm, rawFore, bgImInvPixel8bit);

		// Deal with issue 2: depth image invalid pixels
		// Normalize the depth image valid pixel mask (valid = 1, invalid = 0); Multiply the foreground by normalized mask to set all invalid pixels to zero.
		divide(sourceIm, sourceIm, normDepthMask);
		multiply(rawFore, normDepthMask, rawFore);

		// blur salt and pepper noise
		medianBlur(rawFore,blurFore,1);

		// Entering the 8-bit world to be able to use cv::threshold and cv::findContours, because both require 8-bit images

		// Note that we do the full conversion from 0 to 4m here because of the threshold operation
		blurFore.convertTo(blurFore, CV_8UC1, OPENNI_SCALE);

		// threshold blurred image to remove pixels with a small amount of difference from the background image (attributable to sensor noise) or too close to sensor
		// Note that this creates a foreground "blob" with anything in the foreground set to 255
		threshold(blurFore,blurThresh,3,255, THRESH_BINARY);

		// copy to new Mat because findContours destroys original Mat
		blurThresh.copyTo(threshForeContours);

		// Find contours
		findContours( threshForeContours, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

		// initialize a new image for blobs
		blobs = Mat::zeros( Size(WIDTH, HEIGHT) , CV_8UC1 );

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

		// remove all small contours from foreground image (noise), creating the final foreground "blob" (8-bit)
		subtract(blurThresh, blobs, foreImMask);

		// Back to the 16-bit world

		// convert the foreground mask to 16-bit
		foreImMask.convertTo(normalFore, CV_16UC1);

		// normalize foreground image
		divide(normalFore,normalFore,normalFore);

		// mask the depth image by normalized foreground mask to create the final, 16-bit depth foreground image
		multiply(normalFore, sourceIm, depthFore16bit);

		// 8-bit conversion between MIN_X and MAX_X to maximize 8-bit depth resolution
		depthFore16bit.convertTo(depthFore8bit, CV_8UC1, OPENNI_ALPHA, OPENNI_BETA);

		// create a three channel foreground output image
		Mat channels[] = {depthFore8bit, depthFore8bit, depthFore8bit};
		merge(channels,3,depthFore8bitFull);

		// save the new image
		string targetName = sourceList[img];		// take the name of the source image
		targetName.replace(targetName.find("depth16bit"), 10, "label");
		targetStr = targetpath + "/" + targetName;
		imwrite(targetStr.data(), depthFore8bitFull);

	}
	return 1;
}
/******************************************************************************/

int makeLabelImages( string rootpath, string sourcepath)			// create a set of 3-channel depth images for labeling
{

	string dirDest;
	string destStr, sourceStr;
	vector<string> depthlist, labellist;

	dirDest = rootpath + "/label";
	if(!listFiles(sourcepath, depthlist, "png")){
		return 0;
	}

	if(!listFiles(dirDest, labellist, "png")){
		cout << "Couldn't find directory 'label' in the target directory. Creating it..." << endl;
		if(mkdir(dirDest.data(),0777) != 0){
			cout << "Error making /label folder" << endl;
			return 0;
		}
	}

	Mat sourceIm;
	Mat destIm (cv::Size(640,480),CV_8UC3);

	for (unsigned int img = 0; img < depthlist.size(); img++){

		sourceStr = sourcepath + "/" + depthlist[img];
		sourceIm = imread(sourceStr.data(),0);				// Load as 1 channel 8-bit grayscale image
		Mat in[] = {sourceIm, sourceIm, sourceIm};
		merge(in ,3, destIm);

		destStr = dirDest + "/" + depthlist[img];

		imwrite(destStr.data(), destIm);

	}
	cout << "Successfully created " << depthlist.size() << " 3-channel images for manual labeling from depth images." << endl;

	return 1;
}
/******************************************************************************/

// Find the centre of each labeled body part in world space using centre of moments for labeled data
int groundTruthPartCentresFromLabel(string path, vector<Mat>& labelimages, vector<Mat>& depthimages, vector<string>& labellist){

	cout << "Finding ground truth part centres from fully labeled holdback images using centre of moments." << endl;

	for(int part = 0; part < trackModel->bodyParts.count; part++){
		trackModel->gtPartCentres[part].clear();
	}

	Mat depthim = Mat::zeros(Size(WIDTH,HEIGHT), CV_16UC1);
	Mat labelim = Mat::zeros(Size(WIDTH,HEIGHT), CV_8UC3);

	// loop through each image and find the centres of each ground truth body part
	for(unsigned int img = 0; img < labelimages.size(); img++){

		depthim = depthimages[img];
		labelim = labelimages[img];

		// loop through each body part to find the ground truth center of mass
		for(int part = 0; part < trackModel->bodyParts.count; part++){
			Mat3b label = labelim.clone();

			// isolate the part from the rest of the ground truth background image, setting all part pixels to 255 and all other pixels to 0
			for (Mat3b::iterator it = label.begin(); it != label.end(); it++) {
				if (*it != Vec3b(trackModel->bodyParts.colours[part].x, trackModel->bodyParts.colours[part].y, trackModel->bodyParts.colours[part].z)) {
					*it = Vec3b(0, 0, 0);
				}
				if (*it == Vec3b(trackModel->bodyParts.colours[part].x, trackModel->bodyParts.colours[part].y, trackModel->bodyParts.colours[part].z)) {
					*it = Vec3b(255,255,255);
				}
			}

			// split the image channels (which are all identical now) for processing
			vector<Mat> bodypart(3);
			split(label, bodypart);


			Point3_<float> curGTPartCentre = Point3_<float>(0,0,-1);

			if (countNonZero(bodypart[0]) != 0){		// this body part is labeled in the current image

				float dI;
				Point3_<float> proj;

				// find the moments of the ground truth body part
				Moments mu = moments(bodypart[0], true);

				// find the centre of the moment(0)
				Point2f mc = Point2f(mu.m10/mu.m00,mu.m01/mu.m00);

				// find the depth of the centre of moment
				dI = depthim.at<ushort>(Point(mc.x,mc.y))/10.0;	// find depth of pixel in mm

				// check if the depth of the pixel identified by the centre of mass is valid. If not, try taking the average of the valid pixels in the region around it.
				if(dI == 0){
					cout << "Ground truth part centre for part " << part << ", image " << labellist[img] << " at point " << mc << " has a depth of 0 (invalid). Trying to compute average of pixels around it."<< endl;

					// can only use findNonZero on 8-bit 1-channel Mat
					Mat depthim8bit;
					depthim.convertTo(depthim8bit, CV_8UC1, OPENNI_SCALE);

					// Isolate a small ROI around the centre of mass
					Mat roi(depthim8bit, Rect(mc.x-5,mc.y-5,10,10));
					Mat nonZeroCoords;

					// check if any elements are non-zero
					if(countNonZero(roi) != 0){
						// find coordinates of all non-zero elements
						findNonZero(roi, nonZeroCoords);

						float averageDepth = 0;

						for (unsigned int i = 0; i < nonZeroCoords.total(); i++ ) {
							averageDepth = averageDepth + depthim.at<ushort>(Point(mc.x - 10 + nonZeroCoords.at<Point>(i).x, mc.y - 10 + nonZeroCoords.at<Point>(i).y))/10.0;
						}
						dI = averageDepth/nonZeroCoords.total();

						cout << "Found the average depth of the surrounding pixels to be " << dI << "mm using " << nonZeroCoords.total() << " surrounding pixels." << endl;
					}
					else{
						cout << "Could not calculate ground truth part centre for part " << dI << ", image " << labellist[img] << endl;
						exit(0);
					}
				}

				proj.x = mc.x;
				proj.y = mc.y;
				proj.z = dI;									// Note that the depth here must be in mm, not m, for proper conversion to world space

				// convert the ground truth centre into world space
				grabber->convertProjToReal(1, &proj, &curGTPartCentre);

			}
			trackModel->gtPartCentres[part].push_back(curGTPartCentre);
		}
	}

	string title = "created by groundTruthPartCentresFromLabel using centre of moments from labeled images.";

	// save ground truth parts to file

	saveGroundTruthPartCentres(path, title, labellist);
	cout << "\tGround truth part centre file created." << endl;
	return (1);
}
/******************************************************************************/

// initialize the ground truth part centres using the classifier
void initGroundTruthPartCentres(){

	for(int part = 0; part < trackModel->bodyParts.count; part++){
		trackModel->gtPartCentres[part].clear();
	}

	// loop through for the entire number of images
	for(unsigned int img = 0; img < grabber->depthList.size(); img++){

		if(grabber->background){	// if the background exists, classify the parts as a starting estimate of the ground truth part positions
			string depthFile = grabber->path + "/depth16bit/" + grabber->depthList[img];

			depthIm = imread(depthFile.data(),-1);	// Load as 1 channel 16-bit image

			// find the foreground image from the background image
			grabber->subtractBG(depthIm, foreIm);

			// create a new random set of pixels
			createSamples(trackModel->samples, RUN_PIXELS);

			// update the tracking model to obtain intermediate tracking data
			classifyParts(runConf.num_trees, foreIm, depthIm);

			// update the tracking model to obtain the modes
			modeFind(depthIm);

			// loop through each body part
			for(int part = 0; part < trackModel->bodyParts.count; part++){

				Point3_<float> realCoords = Point3_<float>(0,0,-1);

				if(trackModel->partConf[part] != 0.0){		// a valid part proposal was made for this part
					Point3_<float> projCoords = Point3_<float>(trackModel->partCentres[part].x, trackModel->partCentres[part].y, trackModel->partCentres[part].z);		// coordinates are x,y in image window with depth of pixel (x,y) in mm
					grabber->convertProjToReal(1, &projCoords, &realCoords);
				}

				trackModel->gtPartCentres[part].push_back(realCoords);
			}
			trackModel->gtAction.push_back(0);		// initialize the position to UNDEFINED
			trackModel->gtHandPos.push_back(Point(0,0));		// initialize the task step to UNDEFINED
		}
		else{						// the depth images aren't available. Initialize ground truth part positions
			// loop through each body part
			for(int part = 0; part < trackModel->bodyParts.count; part++)
				trackModel->gtPartCentres[part].push_back(Point3_<float>(0,0,-1));				// set centre to (0,0,-1)

		}
	}
}

// generate a part centre file and hand washing task hand location for a validation trial using the classifier
void validationPartPropMetrics(vector<string>& output, vector<vector<Point3_<float> > >& partCentres){

	//vector<Point> handPos;

	// set up metrics
	vector<vector<float> > truePos(trackModel->bodyParts.count);			// parts in ground truth image that are classified, within PROP_DIST, as the part
	vector<vector<float> > trueNeg(trackModel->bodyParts.count);			// parts not in ground truth image that are not classified as the part
	vector<vector<float> > falsePos(trackModel->bodyParts.count);			// parts in ground truth image that are classified, outside PROP_DIST or within PROP_DIST when another more than once, as the part
	vector<vector<float> > falseNeg(trackModel->bodyParts.count);			// parts in ground truth image that are not classified as the part
	//vector<vector<Point3_<float> > > PRstats(trackModel->bodyParts.count);		// precision (x), recall (y) and threshold (z) stored as x, y and z of Point3_<float>


	// clear participatn positions and set up confusion matrices. Note that runConf.positions holds one extra position (UNDEF)
	for (unsigned int position = 0; position < runConf.positions.size(); position ++){
		for (int part = 0; part < trackModel->bodyParts.count; part++){
			truePos[part].push_back(0.0);
			trueNeg[part].push_back(0.0);
			falsePos[part].push_back(0.0);
			falseNeg[part].push_back(0.0);
		}
	}

	// loop through for the entire number of images
	for(unsigned int img = 0; img < grabber->depthList.size(); img++){

		if(grabber->background){	// if the background exists, classify the parts as a starting estimate of the ground truth part positions
			string depthFile = grabber->path + "/depth16bit/" + grabber->depthList[img];

			depthIm = imread(depthFile.data(),-1);	// Load as 1 channel 16-bit image

			// find the foreground image from the background image
			grabber->subtractBG(depthIm, foreIm);

			// create a new random set of pixels
			createSamples(trackModel->samples, RUN_PIXELS);

			// update the tracking model to obtain intermediate tracking data
			classifyParts(runConf.num_trees, foreIm, depthIm);

			// update the tracking model to obtain the modes
			modeFind(depthIm);

			// loop through each body part
			for(int part = 0; part < trackModel->bodyParts.count; part++){
				if(trackModel->partConf[part] != 0.0){  // Part centre was found for this part

					// convert the classified part centre to float
					Point3_<float> ptClassCentreProj = Point3_<float>(trackModel->partCentres[part].x, trackModel->partCentres[part].y, trackModel->partCentres[part].z);

					// push value onto vector
					partCentres[part].push_back(ptClassCentreProj);
				}
				else
					partCentres[part].push_back(Point3_<float>(0,0,-1));
			}

			// -------------------------------------------------------------------------------------------------
			// Calculate the part proposal error metrics
			// -------------------------------------------------------------------------------------------------

			for(int part = 0; part < trackModel->bodyParts.count; part++){

				// compare the ground truth part centre to the classified part centre
				// update the  confusion matrix stats for the matrix associated with the participant's action, as well as the overall matrix

				if(trackModel->gtPartCentres[part][img] == Point3_<float>(0,0,-1)){							// part doesn't exist in ground truth image
					if (trackModel->modes[part].size() == 0){
						trueNeg[part][trackModel->gtAction[img] - 1] +=1.0;									// classifier didn't provide proposal for this part
						trueNeg[part][runConf.positions.size() - 1] +=1.0;
					}
					else{
						falsePos[part][trackModel->gtAction[img] - 1] += 1.0;								// every part proposed by the classifier is a false positive
						falsePos[part][runConf.positions.size() - 1] +=1.0;
					}
				}
				else{																						// part exists in ground truth image
					if(trackModel->modes[part].size() == 0){												// classifier didn't provide any part proposals
						falseNeg[part][trackModel->gtAction[img] - 1] +=1.0;
						falseNeg[part][runConf.positions.size() - 1] +=1.0;
					}
					else{																					// classifier provided at least one part proposal

						// convert the ground truth part centre to projective space
						Point3_<float> ptGTCentreProj;
						grabber->convertRealToProj(1,&trackModel->gtPartCentres[part][img],&ptGTCentreProj);

						// convert the classified part centre to float
						Point3_<float> ptClassCentreProj = Point3_<float>(trackModel->partCentres[part].x, trackModel->partCentres[part].y, trackModel->partCentres[part].z);

						// find the distance between the ground truth and classified points
						float dist = distance3D(ptGTCentreProj,ptClassCentreProj);

						if(dist < VAL_DIST){
							truePos[part][trackModel->gtAction[img] - 1] +=1.0;								// part proposal is close to the ground truth proposal
							truePos[part][runConf.positions.size() - 1] +=1.0;
						}
						else{
							falsePos[part][trackModel->gtAction[img] - 1] +=1.0;							// Any mode outside VAL_DIST is a false positive
							falsePos[part][runConf.positions.size() - 1] +=1.0;
						}
					}
				}
			} // body part loop
		}

	}

	// stream for results
	ostringstream out;

	// -------------------------------------------------------------------------------------------------
	// Report the part proposal error metrics
	// -------------------------------------------------------------------------------------------------
	for (unsigned int position = 0; position < runConf.positions.size(); position ++){
		for (int part = 0; part < trackModel->bodyParts.count; part++){
			if(trackModel->bodyParts.classify[part]){

				float precision;												// Proportion of correctly classified parts out of all the instances where the algorithm classified that part
				float recall;													// Proportion of actual positives that are correctly identified as such (also called sensitivity)
				if(truePos[part][position] + falsePos[part][position] != 0)
					precision = truePos[part][position]/(truePos[part][position] + falsePos[part][position]);
				else
					precision = -1;

				if (truePos[part][position] + falseNeg[part][position] != 0)
					recall = truePos[part][position]/(truePos[part][position] + falseNeg[part][position]);
				else
					recall = -1;

				// calculate the F(BETA) score
				float Fscore;
				if(precision == 0 && recall == 0)
					Fscore = 0;
				else if (precision == -1 && recall == -1)
					Fscore = -1;
				else
					Fscore =  (1 + pow(BETA,2)) * ( (precision*recall ) / ( ( pow(BETA,2) * precision ) + recall ) );

				out << "\t" << Fscore;
			}
		}
	}
	output.push_back(out.str());
}

void changeAnnotationMode()
{
	if(!annotating){					// start annotating

		cout << endl << "****Entering validation image annotation mode.****" << endl << endl;
		string sourceDir = gtkGetDirPath("Select validation folder");

		// switch to DISK
		runConf.source = DISK;
		initGrabber(DISK, 640, 480, roi, BUFFER_SIZE);

		// get a list of subdirectories within parent directory
		listDirectories(sourceDir, grabber->validateDirs);

		int numImages = 0;

		// read each directory to report the number of images
		for(unsigned int i = 0; i < grabber->validateDirs.size(); i++){
			string subDir = sourceDir + "/" + grabber->validateDirs[i] + "/depth16bit";
			vector<string> subDirFileList;
			listFiles(subDir, subDirFileList, "png");
			numImages = numImages + subDirFileList.size();
		}

		cout << "\tFound a total of " << numImages << " images in " << grabber->validateDirs.size() << " directories." << endl;

		// create a list of body parts set to be annotated based on "classify" status in the configuration file
		gtkBodyPart.clear();

		if(VERBOSE)
			cout << "Annotating for:" << endl;

		for(int part = 0; part < trackModel->bodyParts.count; part++){
			if (trackModel->bodyParts.classify[part]){
				gtkBodyPart.push_back(part);
				if(VERBOSE)
					cout << "\t" << trackModel->bodyParts.label[part] << endl;
			}
		}

		// set the active diskpath
		runConf.diskpath = sourceDir + "/" + grabber->validateDirs[annotateDir];
		updateGrabber(1, sourceDir);

		gtk_button_set_label(GTK_BUTTON(sourceButton), "Source: DISK");
		gtk_button_set_label(GTK_BUTTON(annotateButton), "Stop annotate");

		gtkAction = 0;
		gtkStep = 0;

		annotating = 1;

	}
	else{
		runConf.source = KIN;

		updateGrabber(0);

		gtk_button_set_label(GTK_BUTTON(sourceButton), "Source: KIN");
		gtk_button_set_label(GTK_BUTTON(annotateButton), "Start annotate");

		annotating = 0;
	}
}

void changeAnnotationDir(int index){

	if(annotating && !setRegs){

		if(annotateDir == 0 && index < 0){					// rollover backward
			annotateDir = grabber->validateDirs.size() - 1;
		}
		else if (annotateDir == (grabber->validateDirs.size() - 1) && index > 0){	// rollover forward
			annotateDir = 0;
		}
		else{
			annotateDir = annotateDir + index;
		}

		// update GUI
		stringstream lblStrm;
		lblStrm << "Dir:  " << annotateDir;
		string annString = lblStrm.str();

		gtk_label_set_label(GTK_LABEL(annotateDirNum), annString.c_str());

		cout << "Switching to " << grabber->validateDirs[annotateDir] << endl;

		// set the active diskpath
		runConf.diskpath = grabber->validateParentDir + "/" + grabber->validateDirs[annotateDir];
		updateGrabber(1);
	}
}

// Train a new tree. This always trains the tree into forest[0]
int trainTree(string dirTrain, string dirCheck){

	string trainDir, checkDir;							// path to training images and nmetric check images
	string trainInfo;									// training parameters
	trainInfo = buildTrainMessage();
	cout << "Training info: " << trainInfo.c_str() << endl;

	// Display training parameters when not training from spearmint
	if(spearmint == 0){

		GtkWidget *dialog, *dialogLabel, *dialogContentArea;

		dialog = gtk_dialog_new_with_buttons ("New tree configuration.", window, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);

		dialogContentArea = gtk_dialog_get_content_area ( GTK_DIALOG( dialog ) );

		dialogLabel = gtk_label_new (trainInfo.c_str());

		gtk_container_add (GTK_CONTAINER (dialogContentArea), dialogLabel);

		gtk_widget_show (dialogContentArea);
		gtk_widget_show (dialogLabel);

		int response = gtk_dialog_run( GTK_DIALOG( dialog ) );

		gtk_widget_destroy (dialog);

		if (response != GTK_RESPONSE_OK){
			cout << "Training cancelled." << endl;
			return 1;
		}

		trainDir = gtkGetDirPath("Please select directory containing training images");
	}
	else if (spearmint == 1){
		trainDir = dirTrain;
	}


	string dirLabel = trainDir + "/label";								// path to labeled foreground training images
	string dirDepth = trainDir + "/depth16bit";								// path to depth training images

	vector<string> labelList,depthList, maskList;						// vectors to hold list of rgb and depth images
	struct timespec startIter, stopIter;								// elements for iteration time
	double accIter;															// time elapsed for current iteration
	struct timespec startTrain, stopTrain;								// elements for training time
	double accTrain;													// time elapsed for training

	string labelFile, depthFile;											// strings for rgb and depth image files to be loaded
	vector<Mat> labelImages;											// vector to hold preloaded labelled training images
	vector<Mat> depthImages;											// vector to hold preloaded depth images

	cout << "Creating new tree." << endl;

	// TODO see if this should be done anew on every new node
	// load offset points from file, or create if the file cannot be found/does not match the desired number of candidates or thresholds
	createCandidates(candidates);

	// create thresholds
	createThresholds(thresholds);

	clock_gettime( CLOCK_REALTIME, &startTrain);			// start the clock for this iteration

	setTreeConfig();

	// load sorted list of labeled (training) and depth images
	listFiles(dirLabel, labelList, "png");
	listFiles(dirDepth, depthList, "png");

	int numImages;

	// check the size of the label and depth image lists
	if ( ( labelList.size() != depthList.size() ) ){
		cout << "Label or depth training images don't have the same count." << endl;
		return 1;
	}
	else{
		numImages = labelList.size();				// number of images that will be used to train tree
		cout << "Training on " << numImages << " images, gain threshold = " << cutoffGain << endl;
	}

	cout << "Preloading training images." << endl;

	// preload images for training
	Mat labelRead, depthRead;

	for (int num_im=0 ; num_im < numImages; num_im++){
		// compare each image pair to make sure they have the same five digit identifier

		if (labelList[num_im].compare(labelList[num_im].size()-9,5,depthList[num_im],depthList[num_im].size()-9,5) != 0){
			cout << "Training image mismatch between label and depth images on images " << labelList[num_im] << " and " << depthList[num_im] << endl;
			//return (void*)1;
			return 1;
		}

		// construct image file names
		labelFile = dirLabel + "/" + labelList[num_im];
		depthFile = dirDepth + "/" + depthList[num_im];

		// push images onto image vectors
		labelRead = imread(labelFile.data(),1);  	// Load as 3 channel colour image
		depthRead = imread(depthFile.data(),-1);	// Load as 1 channel 16-bit image

		labelImages.push_back(labelRead);
		depthImages.push_back(depthRead);

		if (!labelImages[num_im].data){
			cout << "Label image data not loaded properly" << endl;
			//return (void*)1;
			return 1;
		}
		if (!depthImages[num_im].data){
			cout << "Depth image data not loaded properly" << endl;
			//return (void*)1;
			return 1;
		}
	}
	cout << "\tImages loaded." << endl;

	treeNode* next_node;															// pointer to the next node to be trained

	// classification vars
	vector<int> feature;															// depth features of current samples

	// entropy vars
	Mat labelsI = Mat::zeros(1,forest[0]->parts.count,CV_32FC1);					// PDF for whole set
	Mat labelsL = Mat::zeros(1,forest[0]->parts.count,CV_32FC1);					// PDF for left subset
	Mat labelsR = Mat::zeros(1,forest[0]->parts.count,CV_32FC1);					// PDF for right subset
	float HQ, HQl, HQr;																// entropy values for total set, and left and right subsets
	float valI, valL, valR;															// temporary storage for entropy terms


	// vars for the split candidates/threshold pair with the highest gain
	vector<imgSample> pixelsL, pixelsR;												// set of samples partitioned to the left/right subset for the split_candidate/threshold pair with the largest gain
	Mat labels(1,forest[0]->parts.count,CV_32FC1);									// label histogram for storage in the node - based on the split_candidates/threshold pair with the largest gain
	float maxGain;																	// largest gain of all split candidates/threshold pairs
	int candPos = 0;																// position of the split candidate pairs with the highest gain
	int threshPos = 0;																// position of the threshold with the highest gain
	vector<imgSample> curSamplesL, curSamplesR;										// vectors for holding the current samples assigned to the left and right subset

	// misc vars
	int iteration=0;																// current iteration in main loop. Used for terminal display
	int done = 0;																	// done flag
	int depth = 0;
	long int cursamples; 														// current samples in node
	int numSamples;																// total number of samples for training
	long int bgPixels;

	vector<imgSample> samples;												// set of pixels to be processed by the next node. Initial loop will process all images

	// Create a set of samples (training data) based on a randomly generated point within each image. This is the data set used for training the decision tree
	bgPixels = 0;
	cout << "Generating random samples based on " << TRAIN_PIXELS << " pixels per image." << endl;
	for (int i = 0; i < numImages ; i++){
		// Load depth and label image to check if point is invalid (depth == 0) or on the background of the image (R=B=G on label image)
		Mat dMap = depthImages[i];
		Mat labImage = labelImages[i];

		imgSample nextPoint;
		for(int j = 0 ; j < TRAIN_PIXELS ; j++){
			nextPoint.x = rand()%((WIDTH-1)-0)+0;
			nextPoint.y = rand()%((HEIGHT-1)-0)+0;
			nextPoint.img = i;				// image number
			nextPoint.gt = colourFeature(0, Point(nextPoint.x, nextPoint.y), labelList[i], labImage);

			unsigned short dI = dMap.at<ushort>(Point(nextPoint.x,nextPoint.y));				// find depth of pixel (x,y) in mm*10
			if(dI != 0 &&  nextPoint.gt != BG_PIXEL){
				samples.push_back(nextPoint);
			}
			else{
				bgPixels ++;
			}
		}
	}

	numSamples = samples.size();

	cout << "\tSet of " << numSamples << " random samples generated out of a possible " << TRAIN_PIXELS*numImages << " samples." << endl;
	cout << "\tRemoved "  << bgPixels << " pixels that were on the background of the images." << endl;

	forest[0]->destroy_tree();	// destroy tree if existing

	/*
	 * Main training loop for the binary decision tree (based on a randomized decision tree/forest)
	 * Steps are as follows (based on "Real-time human pose recognition in parts from single depth images (Shotton et al., 2011))
	 * 1. Split the set of images into left and right subsets for each pair of split candidates/threshold
	 * 2. Determine the feature with the largest gain based on the Shannon entropy of the normalized histogram of body part labels
	 * 3. If the gain is sufficient, and the maximum depth of the tree has not been reached, recurse through the left and right subsets
	 */

	// main training loop
	cout << endl << "Entering main training loop." << endl << endl;
	while (!done){
		clock_gettime( CLOCK_REALTIME, &startIter);			// start the clock for this iteration
		cursamples = samples.size();

		maxGain = -1.0;
		// TODO replace this loop with one that goes through the u,v candidates
		// iterate through split candidate pairs
		for (unsigned int num_cand=0 ; num_cand < candidates.size(); num_cand++){

			feature.resize(cursamples);

			// iterate through thresholds
			for (int num_thresh=0 ; num_thresh < NUM_THRESHOLDS; num_thresh++){

				labelsL = Scalar::all(0.0);
				labelsR = Scalar::all(0.0);
				curSamplesL.clear();
				curSamplesR.clear();

				for (int pix = 0; pix < cursamples; pix++){

					// Find the depth feature for the given sample only on the first threshold for each split candidate pair. This feature will not change for subsequent thresholds so don't find again until next split candidate pair
					if(num_thresh == 0)
						feature[pix] = depthFeature(0, Point(samples[pix].x,samples[pix].y), candidates[num_cand], labelImages[samples[pix].img], depthImages[samples[pix].img]);

					// split into left and right subsets based on current threshold value
					if (feature[pix] < thresholds[num_thresh]){
						labelsL.at<float>(0,samples[pix].gt) +=1.0f;								// update left split PDF
						curSamplesL.push_back(samples[pix]);						// include this sample in the left split node
					} else{
						labelsR.at<float>(0,samples[pix].gt) +=1.0f;								// update right split PDF
						curSamplesR.push_back(samples[pix]);						// include this sample in the right split node
					}
				}

				// total samples, samples belonging to left/right subsets for parts that are prioritized in entropy calculation
				float cL = 0;
				float cR = 0;
				for (int part = 0; part < forest[0]->parts.count; part++){
					labelsI.at<float>(0,part) = labelsL.at<float>(0,part) + labelsR.at<float>(0,part);	// total PDF
					cL = cL + labelsL.at<float>(0,part);
					cR = cR + labelsR.at<float>(0,part);
				}

				float cI = cL + cR;

				HQ = 0.0;
				HQl = 0.0;
				HQr = 0.0;

				// calculate Shannon entropies for the whole set, and left and right subsets
				for (int part = 0; part<forest[0]->parts.count; part++){
					valI = labelsI.at<float>(0,part);
					valL = labelsL.at<float>(0,part);
					valR = labelsR.at<float>(0,part);
					if (valI > 0) HQ  = HQ  + (valI/cI)*log2(valI/cI)*(-1);
					if (valL > 0) HQl = HQl + (valL/cL)*log2(valL/cL)*(-1);
					if (valR > 0) HQr = HQr + (valR/cR)*log2(valR/cR)*(-1);
				}

				// calculate gain and store data for this split candidate/threshold if gain is greater than current maximum gain
				float curGain = HQ - (cL/cI)*HQl - (cR/cI)*HQr;

				if( curGain > maxGain ){
					maxGain = curGain;
					pixelsL.resize(curSamplesL.size());
					copy(curSamplesL.begin(),curSamplesL.end(),pixelsL.begin());
					pixelsR.resize(curSamplesR.size());
					copy(curSamplesR.begin(),curSamplesR.end(),pixelsR.begin());
					labelsI.copyTo(labels);
					candPos = num_cand;
					threshPos = num_thresh;
				}

				// if total entropy of set is less than cutoff gain then no split-candidate/threshold pair will make this a branch node. Force it to exit loops.
				if(HQ < cutoffGain){
					num_cand = candidates.size();
					num_thresh = NUM_THRESHOLDS;
				}
			} // threshold loop
		} // split candidates loop

		node_type type;

		// label the current node as either a branch or a leaf based on gain
		// TODO consider forcing a LEAF node if largest probability is above a certain threshold (almost pure node)
		if ( (maxGain > cutoffGain) ){
			type = BRANCH;
		}
		else{
			numSamples = numSamples - cursamples;		// if it's a leaf, subtract the samples stored in the leaf from the total samples for terminal display
			type = LEAF;
		}

		// Store u,v and threshold that generate the maximum gain in the current node, then get the image set for the next node
		// This insert always returns the parent of the node that was inserted, or the root in the case of the first insertion
		next_node = forest[0]->insertIDDFS(candidates[candPos], thresholds[threshPos], pixelsL, pixelsR, type, labels, maxGain, maxDepth, cursamples);

		if (next_node->left == NULL && next_node->type == BRANCH){	// This will only execute one time, after the root is inserted
			samples.resize(next_node->pixelsL.size());
			depth = 1;
			copy(next_node->pixelsL.begin(),next_node->pixelsL.end(),samples.begin());
		}
		else if (next_node->right == NULL && next_node->type == BRANCH){	// This will execute every other iteration
			samples.resize(next_node->pixelsR.size());
			copy(next_node->pixelsR.begin(),next_node->pixelsR.end(),samples.begin());

		}
		else{	// This will execute every other iteration, and find the next node that is a branch node
			next_node = forest[0]->searchIDDFS(maxDepth);
			if (next_node != NULL){
				depth = next_node->depth+1;
				samples.resize(next_node->pixelsL.size());
				copy(next_node->pixelsL.begin(),next_node->pixelsL.end(),samples.begin());
			}
		}

		// calculate the iteration time for terminal
		clock_gettime( CLOCK_REALTIME, &stopIter);
		accIter = ( stopIter.tv_sec - startIter.tv_sec ) + (double)( stopIter.tv_nsec - startIter.tv_nsec ) / (double)BILLION;
		cout << "Iteration " << iteration +1 << setw(3) << ", depth " << depth << ", " << setw(13) << cursamples << " samples in node." <<
				"\t" << setw(10) << numSamples << " samples remaining, gain: " << setw(10) << maxGain <<", time: " << accIter << "s (" << accIter/60 << "m). Next node: " <<
				samples.size() << " samples." << endl;

		iteration ++;
		// stop iterating if a branch node with an available child does not exist in the tree
		if (next_node == NULL){
			cout << "Training done. " << numSamples << " samples assigned to leaf nodes at maximum tree depth." << endl;
			done = 1;
		}
	} // main training loop

	cout << "Tree trained." << endl;

	clock_gettime( CLOCK_REALTIME, &stopTrain);
	accTrain = ( stopTrain.tv_sec - startTrain.tv_sec ) + (double)( stopTrain.tv_nsec - startTrain.tv_nsec ) / (double)BILLION;

	forest[0]->saveIDDFS(accTrain, numImages, iteration, candidates, thresholds, cutoffGain, offset, maxDepth, threshMagnitude, projectPath, trainDir);			// save tree to file by depth
	cout << "Tree saved to file." << endl;

	// release the memory held by training images
	vector<Mat>().swap(labelImages);
	vector<Mat>().swap(depthImages);

	cout << "Checking tree error metrics..." << endl;

	if(spearmint == 0)
		checkDir = gtkGetDirPath("Checking tree error metrics. Please select directory containing validation images");
	else
		checkDir = dirCheck;

	if(forestPerClassMetrics(checkDir, 1, 0, 1))
		cout << "Couldn't determine error metrics..." << endl;

	forest[0]->destroy_tree();

	cout << "Training done!" << endl;

	// reload configured forest
	if(!loadForest())
		cout << "Problem loading decision trees." << endl;

	return 0;
}

/******************************************************************************/
// Where all the magic happens
gint trackerIdle(void* data) {
	if(grabber->isCamera){
		// Update image buffers with latest images/mask from camera
		grabber->getImages(rgbIm, depthIm, depthMask);

		// If not running, update background buffers with latest frames
		if(!running)
			grabber->updateBackground(depthIm, depthMask);

		// Find the foreground image from depth image, with background and invalid pixels identified
		if(grabber->background)
			grabber->subtractBG(depthIm, foreIm);
	}

	// process it if new depth data is available
	if(grabber->newDepthData && forestLoaded && grabber->background){
		// create a new random set of pixels
		createSamples(trackModel->samples, RUN_PIXELS);

		// update the tracking model to obtain intermediate tracking data
		classifyParts(runConf.num_trees, foreIm, depthIm);

		// update the tracking model to obtain the modes
		modeFind(depthIm);

		// implement temporal filters
		filterPartProposals();

	// capture the position of the left hands as float
	Point3_<float> LHandPosition = Point3_<float>(trackModel->partCentres[0].x, trackModel->partCentres[0].y, trackModel->partCentres[0].z);
	printf("=========in getLHandPos tracy: LHandPosition = < %f, %f, %f>\n", LHandPosition.x, LHandPosition.y, LHandPosition.z);
	
		// use hand locations to find the task action each hand is completing
		// findAction(LHandPosition, RHandPosition, trackModel->handAction);

		// we're done with it so wait for more
		grabber->newDepthData = FALSE;
	}

	// Update image windows with latest images
	refreshImageWindows();

	// trial options
	if (running){

		// save images
		if(saveRGB)
			savePng("rgb", grabber->cur_image , rgbIm);
		if(saveDepth)
			savePng("depth", grabber->cur_image , depthIm);
		if(saveDMask)
			savePng("mask", grabber->cur_image , depthMask);
		if(saveFore)
			savePng("fore", grabber->cur_image , foreIm);
		if(saveSkel){
			Mat skeleton = Mat::zeros(Size(WIDTH,HEIGHT), CV_8UC3);

			// Display the contours of the foreground image
			drawForegroundOutline(foreIm, skeleton);

			// overlay part proposals
			drawPartProposals(skeleton);

			cvtColor(skeleton,skeleton, CV_BGR2RGB);
			savePng("skel", grabber->cur_image , skeleton);
		}

		// if running from disk, increment to the next image
		if(grabber->source == DISK)
			grabber->cur_image++;

	}

	// Minor GUI stuff
	acc_s = g_timer_elapsed(runTimer, &acc_us);

	stringstream frameDisp;
	if (!annotating && !setRegs){
		frameDisp << "Frame " << grabber->cur_image;
	}
	else if(annotating && !setRegs){
		frameDisp << "Frame " << grabber->cur_image <<
				",\t" << runConf.positions[trackModel->gtAction[grabber->cur_image]] <<
				",\tLH " << runConf.activities[trackModel->gtHandPos[grabber->cur_image].x] <<
				",\tRH " << runConf.activities[trackModel->gtHandPos[grabber->cur_image].y] <<
				",\t" << trackModel->bodyParts.label[gtkBodyPart[gtkBodyPartOffset]] << ", " << grabber->depthList[grabber->cur_image];
	}
	else if(setRegs){
		if(!regionSetMode)
			frameDisp << "Frame " << grabber->cur_image << "\t" << "Setting centre for region " << trackModel->regions[curRegionToSet].label << " (press (r) to set radius)";
		if(regionSetMode)
			frameDisp << "Frame " << grabber->cur_image << "\t" << "Setting radius for region " << trackModel->regions[curRegionToSet].label << " (press (e) to set centre)";
	}

	gtkPushStatus(statusBar, frameDisp.str(), context_id);

	return (TRUE);
}

void usage(){
	cout << "Usage: main.cpp" << endl;

	cout << "required arguments:" << endl;
	cout << "-projectpath <path to project>" << endl;
	cout << "-offset <(integer) offset in pixels for split candidate pairs>" << endl;
	cout << "-threshold <(integer) threshold for tree node split>" << endl;
	cout << "-gain <(float) minimum gain for decsion tree training>" << endl;
	cout << "-spearmint <(int) 0 (default) = normal run with GUI, 1 = train tree from command line args, 2 = optimize part bandwidths>" << endl;

}

/******************************************************************************/
int main( int argc, char** argv )
{

	char* argflag=NULL;
	vector<string> savedTrees;

	// Training setup - command line arguments
	float s_band = -1;												// spearmint bandwidth
	float s_probThresh = -1;									// spearmint probability threshold
	int s_part = -1;											// spearmint part to optimize

	// Initialize training parameters
	cutoffGain = MIN_GAIN;
	offset = MAX_OFFSET;
	maxDepth = MAX_DEPTH;
	threshMagnitude= MAX_THRESHOLD;

	// Parse input parameters

	if (argc < 3) {
		usage();
		exit(0);
	}
	cout <<  "Decision tree." << endl;
	while (--argc > 0 && (*++argv)[0] == '-') {
		argflag = ++argv[0];

		if (strcmp(argflag, "projectpath") == 0) {
			projectPath = *++argv;
		}
		else if (strcmp(argflag, "offset") == 0) {
			offset = atoi(*++argv);
		}
		else if (strcmp(argflag, "threshold") == 0) {
			threshMagnitude = atoi(*++argv);
		}
		else if (strcmp(argflag, "gain") == 0){
			cutoffGain = atof(*++argv);
		}
		else if (strcmp(argflag, "band") == 0){
			s_band = atof(*++argv);
		}
		else if (strcmp(argflag, "depth") == 0){
			maxDepth = atoi(*++argv);
		}
		else if (strcmp(argflag, "prob_thresh") == 0){
			s_probThresh = atof(*++argv);
		}
		else if (strcmp(argflag, "part") == 0){
			s_part = atoi(*++argv);
		}
		else if (strcmp(argflag, "spearmint" ) == 0) {
			spearmint = atoi(*++argv);
		}
		else{
			cout << "Unknown argument passed to command line: " << argflag << endl;
			exit(0);
		}
		argc--;
	}

	srand( time(NULL) );	// Initialize random seed

	// Load configuration from file
	loadConfig("/config.txt");
	loadRegions("/regions.txt");

	if(runConf.num_trees > 0){
		forest = new DTree*[runConf.num_trees]();
		for(int k = 0;k < runConf.num_trees; k++){
			forest[k] = new DTree();
		}
	}
	else
	{
		cout << "Attempt to create a forest of " << runConf.num_trees << " failed. Shutting down." << endl;
		exit(0);
	}

	if (spearmint == 1)			// optimizing tree training
	{
		cout << "Spearmint is optimizing tree training." << endl;
		string trainDir = projectPath + "/training/T3";
		string checkDir = projectPath + "/training/T1";
		trainTree(trainDir,checkDir);
		exit(0);
	}
	else if (spearmint == 2)	{		// if optimizing parameters, load images from disk
		if(s_part != -1 && s_probThresh != -1)				// spearmint parameters are sent for current body part
			trackModel->bodyParts.probThreshs[s_part] = s_probThresh;
		else{
			cout << "Spearmint didn't send body part or probability threshold. Exiting." << endl;
			exit(0);
		}

		cout << "Overriding camera source from config.txt file. Setting to DISK." << endl;
		runConf.source = DISK;
		if(!loadForest()){
			cout << "Problem loading decision trees." << endl;
			exit(0);
		}
	}

	// Initialize grabber
	initGrabber(runConf.source, 640, 480, roi, BUFFER_SIZE);

	updateGrabber(0);

	if (spearmint == 2)	// optimizing modefind parameters
	{
		cout << "Spearmint initiated this call." << endl << "\tSetting bandwidth from spearmint." << endl;
		if(s_part != -1 && s_band != -1)
			trackModel->bodyParts.bandwidths[s_part] = s_band;						// Gaussian estimator bandwidth for body part that is being optimized
		else{
			cout << "Spearmint didn't send body part or bandwidth. Exiting." << endl;
			exit(0);
		}

		cout << "\tDisabling classification of all parts except part " << s_part << endl;
		for (int part = 0; part < trackModel->bodyParts.count; part++){
			trackModel->bodyParts.classify[part] = 0;
		}

		cout << "\tForcing classification of part " << trackModel->bodyParts.label[s_part] << endl;
		trackModel->bodyParts.classify[s_part] = 1;

		forestPartPropMetrics(runConf.diskpath, runConf.num_trees, 1);
		exit(0);

	}

	// Initialize Kalman Filters fore each body part
	KFilters = new Filter*[trackModel->bodyParts.count];
	for (int i = 0; i < trackModel->bodyParts.count; i++){
		KFilters[i] = new Filter(i);
	}

	if(!loadForest())
		cout << "Problem loading decision trees." << endl;

	/***************************************************************************
	 *
	 * Set up the GUI
	 *
	 **************************************************************************/

	int windowWidth = WIDTH * 2;

	/* Secure glib */
	if( ! g_thread_supported() )
		g_thread_init( NULL );

	/* Secure gtk */
	gdk_threads_init();

	/* Obtain gtk's global lock */
	gdk_threads_enter();

	// init gtk
	gtk_init(&argc, &argv);

	/* create a new window */
	mainWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	gtk_window_set_title(GTK_WINDOW (mainWindow), "COACH");
	gtk_window_set_position(GTK_WINDOW(mainWindow), GTK_WIN_POS_CENTER);

	g_signal_connect (mainWindow, "delete-event", G_CALLBACK (gtkDeleteEvent), NULL);
	g_signal_connect (mainWindow, "destroy", G_CALLBACK (gtkDestroy), NULL);

	/* Sets the border width of the window. */
	gtk_container_set_border_width (GTK_CONTAINER (mainWindow), 0);

	//  We create the main vertical box (vbox) to pack the horizontal boxes into.
	mainvbox = gtk_vbox_new (FALSE, 0);


	// Set up images
	{
		// new horizontal box for images
		hbox = gtk_hbox_new (FALSE, 0);
		// Left Image window
		{
			vbox = gtk_vbox_new (FALSE, 0);
			gtk_widget_set_size_request(vbox, WIDTH, -1);

			gtkLImage = gtk_drawing_area_new();
			gtk_widget_set_size_request(gtkLImage, WIDTH, HEIGHT);
			gtk_box_pack_start (GTK_BOX (vbox), gtkLImage, TRUE, FALSE, 0);

			hbox2 = gtk_hbox_new(FALSE,0);

			button = gtk_radio_button_new_with_label (NULL, "RGB");
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
			gtk_box_pack_start (GTK_BOX (hbox2), button, FALSE, TRUE, 0);
			g_signal_connect (button, "pressed", G_CALLBACK (gtkImageRadioCallback), (gpointer)0 );

			group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
			button = gtk_radio_button_new_with_label (group, "Depth");
			gtk_box_pack_start (GTK_BOX (hbox2), button, FALSE, TRUE, 0);
			g_signal_connect (button, "pressed", G_CALLBACK (gtkImageRadioCallback), (gpointer)1);

			gtk_widget_set_size_request(hbox2, -1, 30);

			gtk_box_pack_start (GTK_BOX (vbox), hbox2, TRUE, FALSE, 0);

			gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, FALSE, 0);
		}


		// Right Image window
		{
			vbox = gtk_vbox_new (FALSE, 0);
			gtk_widget_set_size_request(vbox, WIDTH, -1);

			gtkRImage = gtk_drawing_area_new();
			gtk_signal_connect (GTK_OBJECT (gtkRImage), "button_press_event", (GtkSignalFunc) gtkButtonPressEvent, NULL);	// tie a button-press even to this window for annotation
			gtk_widget_set_events (gtkRImage, GDK_BUTTON_PRESS_MASK);														// set the types of events we are interested in

			gtk_widget_set_size_request(gtkRImage, WIDTH, HEIGHT);
			gtk_box_pack_start (GTK_BOX (vbox), gtkRImage, TRUE, FALSE, 0);

			// image selection radio buttons
			hbox2 = gtk_hbox_new(FALSE,0);

			button = gtk_radio_button_new_with_label (NULL, "Skeleton");
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
			gtk_box_pack_start (GTK_BOX (hbox2), button, FALSE, TRUE, 0);
			g_signal_connect (button, "pressed", G_CALLBACK (gtkImageRadioCallback), (gpointer)10 );

			group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
			button = gtk_radio_button_new_with_label (group, "Mask");
			gtk_box_pack_start (GTK_BOX (hbox2), button, FALSE, TRUE, 0);
			g_signal_connect (button, "pressed", G_CALLBACK (gtkImageRadioCallback), (gpointer)11 );

			group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
			button = gtk_radio_button_new_with_label (group, "Foreground");
			gtk_box_pack_start (GTK_BOX (hbox2), button, FALSE, TRUE, 0);
			g_signal_connect (button, "pressed", G_CALLBACK (gtkImageRadioCallback), (gpointer)12 );

			gtk_widget_set_size_request(hbox2, -1, 30);

			gtk_box_pack_start (GTK_BOX (vbox), hbox2, TRUE, FALSE, 0);

			gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, FALSE, 0);
		}
		// Add the image windows to the main box
		gtk_box_pack_start (GTK_BOX (mainvbox), hbox, FALSE, FALSE, 0);
	}


	// Set up the play buttons
	{
		// New horizontal box for the main buttons/info
		hbox = gtk_hbox_new (FALSE, 0);
		{
			vbox = gtk_vbox_new (FALSE, 0);
			gtk_widget_set_size_request(vbox, windowWidth/2, -1);

			// Set up misc buttons
			hbox2 = gtk_hbox_new (FALSE, 0);

			runButton = gtk_button_new_with_label ("RUN");
			g_signal_connect (runButton, "clicked", G_CALLBACK (gtkRun), NULL);
			gtk_box_pack_start (GTK_BOX (hbox2), runButton, FALSE, FALSE, 0);

			button = gtk_button_new_with_label ("First");
			g_signal_connect (button, "clicked", G_CALLBACK (gtkChangeImage), (gpointer)0);
			gtk_box_pack_start (GTK_BOX (hbox2), button, FALSE, FALSE, 0);

			button = gtk_button_new_with_label ("<-");
			g_signal_connect (button, "clicked", G_CALLBACK (gtkChangeImage), (gpointer)-1);
			gtk_box_pack_start (GTK_BOX (hbox2), button, FALSE, FALSE, 0);

			button = gtk_button_new_with_label ("->");
			g_signal_connect (button, "clicked", G_CALLBACK (gtkChangeImage), (gpointer)1);
			gtk_box_pack_start (GTK_BOX (hbox2), button, FALSE, FALSE, 0);

			sourceButton = gtk_button_new_with_label ("Source: KIN");
			g_signal_connect (sourceButton, "clicked", G_CALLBACK (gtkSwitchSource), NULL);
			gtk_box_pack_start (GTK_BOX (hbox2), sourceButton, FALSE, FALSE, 0);

			button = gtk_button_new_with_label ("Directory");
			g_signal_connect (button, "clicked", G_CALLBACK (gtkSetDiskPath), NULL);
			gtk_box_pack_start (GTK_BOX (hbox2), button, FALSE, FALSE, 0);

			gtk_box_pack_start (GTK_BOX (vbox), hbox2, FALSE, FALSE, 0);	// Add buttons to the vbox

			gtkframe = gtk_frame_new("Run controls");
			gtk_frame_set_label_align (GTK_FRAME (gtkframe), 0.5, 0.5);
			gtk_container_add(GTK_CONTAINER (gtkframe), vbox);				// Add vbox to frame


			gtk_box_pack_start (GTK_BOX (hbox), gtkframe, TRUE, FALSE, 0);	// Add framed hbox to hbox
		}

		{
			vbox = gtk_vbox_new (FALSE, 0);
			gtk_widget_set_size_request(vbox, windowWidth/2, -1);

			// Set up misc buttons
			hbox2 = gtk_hbox_new (FALSE, 0);
			// skeleton model options
			button = gtk_check_button_new_with_label ("Disp All Modes");
			gtk_box_pack_start (GTK_BOX (hbox2), button, FALSE, TRUE, 0);
			g_signal_connect (button, "toggled", G_CALLBACK (gtkAllModesCallback), NULL );

			button = gtk_check_button_new_with_label ("Disp All Samples");
			gtk_box_pack_start (GTK_BOX (hbox2), button, FALSE, TRUE, 0);
			g_signal_connect (button, "toggled", G_CALLBACK (gtkAllSamplesCallback), NULL );

			button = gtk_check_button_new_with_label ("Disp Regions");
			gtk_box_pack_start (GTK_BOX (hbox2), button, FALSE, TRUE, 0);
			g_signal_connect (button, "toggled", G_CALLBACK (gtkDispRegions), NULL );

			button = gtk_check_button_new_with_label ("K. Filters");
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
			gtk_box_pack_start (GTK_BOX (hbox2), button, FALSE, TRUE, 0);
			g_signal_connect (button, "toggled", G_CALLBACK (gtkKalmanFiltersCallback), NULL );

			partEntry = gtk_combo_box_new_text ();
			for(int part = 0; part < trackModel->bodyParts.count; part++){
				gtk_combo_box_append_text (GTK_COMBO_BOX (partEntry), trackModel->bodyParts.label[part].c_str());
			}
			gtk_combo_box_set_active (GTK_COMBO_BOX (partEntry), 0);
			gtk_box_pack_start (GTK_BOX (hbox2), partEntry, FALSE, TRUE, 0);
			g_signal_connect (GTK_COMBO_BOX (partEntry), "changed", G_CALLBACK (gtkPartSelectCallback), NULL);

			scalebutton = gtk_scale_button_new(GTK_ICON_SIZE_LARGE_TOOLBAR, 0.25, 100.0, 0.025 , NULL);
			gtk_box_pack_start (GTK_BOX (hbox2), scalebutton, FALSE, TRUE, 0);
			g_signal_connect(scalebutton, "value-changed", G_CALLBACK(gtkScaleChangeCallback), NULL);

			partClass = gtk_check_button_new_with_label ("Classified");
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), trackModel->bodyParts.classify[0]);
			gtk_box_pack_start (GTK_BOX (hbox2), partClass, FALSE, TRUE, 0);
			g_signal_connect (partClass, "toggled", G_CALLBACK (gtkPartClassifyCallback), NULL);

			gtk_box_pack_start (GTK_BOX (vbox), hbox2, FALSE, FALSE, 0);	// Add buttons to the vbox

			gtkframe = gtk_frame_new("Misc. controls");
			gtk_frame_set_label_align (GTK_FRAME (gtkframe), 0.5, 0.5);
			gtk_container_add(GTK_CONTAINER (gtkframe), vbox);				// Add vbox to frame

			gtk_box_pack_start (GTK_BOX (hbox), gtkframe, TRUE, FALSE, 0);	// Add framed hbox to hbox
		}

		gtk_box_pack_start (GTK_BOX (mainvbox), hbox, FALSE, FALSE, 0);
	}

	// Set up various info/selections
	{
		hbox = gtk_hbox_new (FALSE, 0);

		// Set up forest display
		{
			vbox = gtk_vbox_new (FALSE, 0);
			gtk_widget_set_size_request(vbox, windowWidth/3, -1);

			// Set up forest info
			treebuffer = new GtkEntryBuffer*[runConf.num_trees];

			for (int i = 0; i < runConf.num_trees; i++){
				hbox2 = gtk_hbox_new (FALSE, 0);
				stringstream treelabel;
				treelabel << "Tree " << i << ": ";
				string str(treelabel.str());
				string full = str + runConf.treePaths[i].data();
				label = gtk_label_new(full.data());
				gtk_label_set_line_wrap(GTK_LABEL(label),TRUE);

				gtk_box_pack_start(GTK_BOX (hbox2), label, FALSE, FALSE, 0);

				gtk_box_pack_start (GTK_BOX (vbox), hbox2, FALSE, FALSE, 0);
			}

			{
				hbox2 = gtk_hbox_new (FALSE, 0);
				string pthFull = "Disk path: " + runConf.diskpath;
				diskpath = gtk_label_new(pthFull.data());
				gtk_label_set_line_wrap(GTK_LABEL(diskpath),TRUE);
				gtk_box_pack_start(GTK_BOX (hbox2), diskpath, FALSE, FALSE, 0);

				gtk_box_pack_start (GTK_BOX (vbox), hbox2, FALSE, FALSE, 0);
			}

			{
				hbox2 = gtk_hbox_new (FALSE, 0);

				stringstream numParts;
				numParts << "Number of parts: " << trackModel->bodyParts.count;
				string prts(numParts.str());
				label = gtk_label_new(prts.data());
				gtk_box_pack_start(GTK_BOX (hbox2), label, FALSE, FALSE, 0);

				gtk_box_pack_start (GTK_BOX (vbox), hbox2, FALSE, FALSE, 0);

			}
			// Set up forest buttons
			//hbox2 = gtk_hbox_new (FALSE, 0);

			//button = gtk_button_new_with_label ("Load forest");
			//g_signal_connect (button, "clicked", G_CALLBACK (gtkLoadForest), NULl);
			//gtk_box_pack_start (GTK_BOX (hbox2), button, TRUE, FALSE, 0);

			// Add buttons to the vbox
			//gtk_box_pack_start (GTK_BOX (vbox), hbox2, FALSE, FALSE, 0);

			gtkframe = gtk_frame_new("Run configuration");
			gtk_frame_set_label_align (GTK_FRAME (gtkframe), 0.5, 0.5);
			gtk_container_add(GTK_CONTAINER (gtkframe), vbox);

			// Add vbox to hbox
			gtk_box_pack_start (GTK_BOX (hbox), gtkframe, TRUE, FALSE, 0);

		}// End forest display


		// Set up the image saving checkboxes
		{
			vbox = gtk_vbox_new (FALSE, 0);
			gtk_widget_set_size_request(vbox, windowWidth/6, -1);

			check = gtk_check_button_new_with_label( "RGB" );
			gtk_signal_connect (GTK_OBJECT (check), "toggled",	GTK_SIGNAL_FUNC (gtkSaveCallback), (gpointer)0);
			gtk_box_pack_start (GTK_BOX (vbox), check, FALSE, TRUE, 0);

			check = gtk_check_button_new_with_label( "Depth" );
			gtk_signal_connect (GTK_OBJECT (check), "toggled",	GTK_SIGNAL_FUNC (gtkSaveCallback), (gpointer)1);
			gtk_box_pack_start (GTK_BOX (vbox), check, FALSE, TRUE, 0);

			check = gtk_check_button_new_with_label( "Depth Mask" );
			gtk_signal_connect (GTK_OBJECT (check), "toggled",	GTK_SIGNAL_FUNC (gtkSaveCallback), (gpointer)2);
			gtk_box_pack_start (GTK_BOX (vbox), check, FALSE, TRUE, 0);

			check = gtk_check_button_new_with_label( "Foreground" );
			gtk_signal_connect (GTK_OBJECT (check), "toggled",	GTK_SIGNAL_FUNC (gtkSaveCallback), (gpointer)3);
			gtk_box_pack_start (GTK_BOX (vbox), check, FALSE, TRUE, 0);

			check = gtk_check_button_new_with_label( "Skeleton" );
			gtk_signal_connect (GTK_OBJECT (check), "toggled",	GTK_SIGNAL_FUNC (gtkSaveCallback), (gpointer)4);
			gtk_box_pack_start (GTK_BOX (vbox), check, FALSE, TRUE, 0);

			gtkframe = gtk_frame_new("Save Image Options");
			gtk_frame_set_label_align (GTK_FRAME (gtkframe), 0.5, 0.5);
			gtk_container_add(GTK_CONTAINER (gtkframe), vbox);

			// Add framed vbox to hbox
			gtk_box_pack_start (GTK_BOX (hbox), gtkframe, TRUE, FALSE, 0);

		}// End misc display

		// Set up image manipulation buttons
		{
			vbox = gtk_vbox_new (FALSE, 0);
			gtk_widget_set_size_request(vbox, windowWidth/6, -1);

			// buttons

			button = gtk_button_new_with_label ("Create label images");
			g_signal_connect (button, "clicked", G_CALLBACK (gtkMakeLabelImages), NULL); 				// Create a new set of 3-channel images for labeling (run the three steps below automatically)
			gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

			button = gtk_button_new_with_label ("1. Create BG images");
			g_signal_connect (button, "clicked", G_CALLBACK (gtkCreateBackground), NULL); 				// Create a new background image
			gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

			button = gtk_button_new_with_label ("2. Remove BG from depth");
			g_signal_connect (button, "clicked", G_CALLBACK (gtkRemoveBackground), NULL); 				// Remove background image from a set of depth images
			gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

			button = gtk_button_new_with_label ("3. Create 3 channel images");
			g_signal_connect (button, "clicked", G_CALLBACK (gtkMakeDepthImages), NULL); 				// Create a new set of 3-channel images for labeling from 1-ch depth images
			gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

			annotateButton = gtk_button_new_with_label ("Start Annotate");
			g_signal_connect (annotateButton, "clicked", G_CALLBACK (gtkAnnotationMode), NULL);
			gtk_box_pack_start (GTK_BOX (vbox), annotateButton, FALSE, FALSE, 0);

			// TODO pack these into an hbox
			// TODO update annotateDir with left and right
			// TODO convert this to a stringstream for display

			hbox2 = gtk_hbox_new (FALSE, 0);
			{
				button = gtk_button_new_with_label ("<");
				g_signal_connect (button, "clicked", G_CALLBACK (gtkChangeAnnotationDir), (gpointer)-1);
				gtk_box_pack_start (GTK_BOX (hbox2), button, FALSE, FALSE, 0);

				button = gtk_button_new_with_label (">");
				g_signal_connect (button, "clicked", G_CALLBACK (gtkChangeAnnotationDir), (gpointer)1);
				gtk_box_pack_start (GTK_BOX (hbox2), button, FALSE, FALSE, 0);

				stringstream lblStrm;
				lblStrm << "Dir:  " << annotateDir;
				string annString = lblStrm.str();
				annotateDirNum = gtk_label_new(annString.c_str());
				gtk_box_pack_start(GTK_BOX (hbox2), annotateDirNum, FALSE, FALSE, 0);
			}
			gtk_box_pack_start (GTK_BOX (vbox), hbox2, FALSE, FALSE, 0);

			gtkframe = gtk_frame_new("Image manipulation");
			gtk_frame_set_label_align (GTK_FRAME (gtkframe), 0.5, 0.5);
			gtk_container_add(GTK_CONTAINER (gtkframe), vbox);

			// Add buttons to the main box
			gtk_box_pack_start (GTK_BOX (hbox), gtkframe, TRUE, FALSE, 0);
		}

		// Set up misc buttons
		{
			vbox = gtk_vbox_new (FALSE, 0);
			gtk_widget_set_size_request(vbox, windowWidth/6, -1);

			// buttons
			button = gtk_button_new_with_label ("Train tree");
			g_signal_connect (button, "clicked", G_CALLBACK (gtkTrainTree), NULL);
			gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

			button = gtk_button_new_with_label ("Raw (Per-Class) Metrics");
			g_signal_connect (button, "clicked", G_CALLBACK (gtkForestPerClassMetrics), NULL);
			gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

			button = gtk_button_new_with_label ("Part Proposal Metrics");
			g_signal_connect (button, "clicked", G_CALLBACK (gtkForestPartPropMetrics), NULL);
			gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

			button = gtk_button_new_with_label ("Validation image Metrics");
			g_signal_connect (button, "clicked", G_CALLBACK (gtkValidationMetrics), NULL);
			gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

			setRegions = gtk_button_new_with_label ("Set Regions");
			g_signal_connect (setRegions, "clicked", G_CALLBACK (gtkSetRegions), NULL);
			gtk_box_pack_start (GTK_BOX (vbox), setRegions, FALSE, FALSE, 0);

			gtkframe = gtk_frame_new("Miscellaneous");
			gtk_frame_set_label_align (GTK_FRAME (gtkframe), 0.5, 0.5);
			gtk_container_add(GTK_CONTAINER (gtkframe), vbox);

			// Add buttons to the main box
			gtk_box_pack_start (GTK_BOX (hbox), gtkframe, TRUE, FALSE, 0);
		}

		gtk_box_pack_start (GTK_BOX (mainvbox), hbox, FALSE, FALSE, 0);
	}


	// Set up utility buttons
	{
		// New horizontal box for utility buttons
		hbox = gtk_hbox_new (FALSE, 0);

		button = gtk_button_new_with_label ("Quit");
		g_signal_connect (button, "clicked", G_CALLBACK (gtkQuit), NULL); 						// When the button receives the "clicked" signal, it will call the function quit() passing it NULL as its argument.
		gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, FALSE, 0);

		// Add buttons to the main box
		gtk_box_pack_start (GTK_BOX (mainvbox), hbox, TRUE, FALSE, 0);
	}

	// Status bar
	{
		statusBar = gtk_statusbar_new ();
		context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR (statusBar), "Status bar");
		gtk_box_pack_start (GTK_BOX (mainvbox), statusBar, TRUE, TRUE, 0);
	}

	// Add the main box to the main window
	gtk_container_add (GTK_CONTAINER (mainWindow), mainvbox);

	g_signal_connect (G_OBJECT (mainWindow), "key_press_event", G_CALLBACK (gtkOnKeyPress), NULL);



	// Show
	gtk_widget_show_all (mainWindow);

    // Connect as server, using zmq

	// Add idle processes:
	// 1) monitor hand positions at all times
	idleTag = g_idle_add( trackerIdle, NULL);
	// 2) listen to "client" requests for info
	int request_type = 0;
    g_idle_add(processRequestsIdle, &request_type);

	runTimer = g_timer_new();
	g_timer_start(runTimer);

	// Start
	gtk_main ();
	gdk_threads_leave ();


}

gint processRequestsIdle(void* data){
	printf("========= Processing requests %d!\n", *((int*)data));
	return TRUE;
}

void respondWithLeftHandPos() {
	// capture the position of the left hands as float
	Point3_<float> LHandPosition = Point3_<float>(trackModel->partCentres[0].x, trackModel->partCentres[0].y, trackModel->partCentres[0].z);
	printf("=========in getLHandPos tracy: LHandPosition = < %f, %f, %f>\n", LHandPosition.x, LHandPosition.y, LHandPosition.z);
}

void respondWithRightHandPos() {
	// capture the position of the right hands as float
	Point3_<float> RHandPosition = Point3_<float>(trackModel->partCentres[1].x, trackModel->partCentres[1].y, trackModel->partCentres[1].z);
	printf("=========in getRightHandPOs tracy: RHandPosition = < %f, %f, %f>\n", RHandPosition.x, RHandPosition.y, RHandPosition.z);
}	

void respondWithHandAction() {
	// TODO: call findAction()
}
/******************************************************************************/
