/*
 *  File:				main.cpp
 *  Created by:			Luyuan Lin
 *  Created:			March 2014
 *  Last Modified:		March 2014
 *  Last Modified by:	Luyuan Lin
 * 
 *  Defined the main function for the whole EHwA system.
 */

#include <stdlib.h> //rand()
#include "../lib/Grabber_TrackingModel/defines.h"
#include "../lib/Grabber_TrackingModel/grab.h"
#include "FrameAnalyzer.hpp"

int loop_GrabAndAnalyze(Grabber* grabber, FrameAnalyzer* frameAnalyzer, void* data){

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
		frameAnalyzer -> createSamples(FrameAnalyzer::RUN_PIXELS);

		// update the tracking model to obtain intermediate tracking data
		classifyParts(runConf.num_trees, foreIm, depthIm);

		// update the tracking model to obtain the modes
		modeFind(depthIm);

		// implement temporal filters
		filterPartProposals();

		// capture the position of the left and right hands as float
		// TODO find the left and right hand positions by parsing the list of parts
		//Point3_<float> LHandPosition = Point3_<float>(trackModel->partCentres[0].x, trackModel->partCentres[0].y, trackModel->partCentres[0].z);
		//Point3_<float> RHandPosition = Point3_<float>(trackModel->partCentres[1].x, trackModel->partCentres[1].y, trackModel->partCentres[1].z);
		cout << "trackModel->partCentres[0].x, trackModel->partCentres[0].y, trackModel->partCentres[0].z = (" 
		     << trackModel->partCentres[0].x << ", " 
		     << trackModel->partCentres[0].y << ", "
		     << trackModel->partCentres[0].z << ")" << endl;

		// use hand locations to find the task action each hand is completing
		//findAction(LHandPosition, RHandPosition, trackModel->handAction);

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

int main() {
  // define and initianlize them
  KinGrabber kinGrabber;
  FrameAnalyzer frameAnalyzer;
  EPACalculator epaCalculator;
  StageAnalyzer stageAnalyzer;
  
  // linux system pipe, fake object here, i don't remember how to use it
  Pipe pipe;
  
  for (;;) {
      //Frame frame = kinGrabber.Capture();
      kinGrabber.update();
      Image image1 = kinGrabber.foreImFull;
      Image image2 = 
      Image image3 = blah;
      
      Frame frame(image1, 2, 3);
      frameAnalyzer.analyzer(frame);
      Point3 leftHandPos = frameAnalyzer.getLeftHandPos();
      Point3 rightHandPos = frameAnalyzer.getRightHandPos();
      
      DoubleTuple3 epa = epaCalculator.Calculate(left, right);
      Stage currentStage = StageAnalyzer.analyzer(frameAnalyzer.getHandAction());
      
      // connect to BayesAct
      pipe.write(epa, currentStage);
      // the program will block here, until BayesAct engine returns
      pipe.read(respondedEPA);
      
      // scan table
      
      // play video
      // we do not always play new video here, do it at previous one finished.
      
  }
}
