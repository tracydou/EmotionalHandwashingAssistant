/*
 *  File:				FrameAnalyzer.hpp
 *  Created by:			Luyuan Lin
 *  Created:			March 2014
 *  Last Modified:		March 2014
 *  Last Modified by:	Luyuan Lin
 *  
 */

#include "FrameAnalyzer.hpp"

namespace EHwA {
	
FrameAnalyzer::FrameAnalyzer(){
  // init p_actionTracker & p_faceRecoganizer
}

FrameAnalyzer::~FrameAnalyzer(){
  // do nothing	
}
  
void FrameAnalyzer::Analyze() {
  // p_actionTracker->update(frame.image1, frame.image2, frame.image3);
  // p_faceRecoganizer->update(frame);
}

// Randomly propose a set of numSamples pixels per image for training
int FrameAnalyzer::createSamples(int numSamples){
/*
 * 	p_actionTracker -> samples.clear();
	int x,y;
	for(int i=0; i < numSamples; i++){
		x = rand()%((WIDTH-1)-0)+0;
		y = rand()%((HEIGHT-1)-0)+0;
		p_actionTracker -> samples.push_back(Point(x,y));
	}
	* */
	return 1;
}

Position FrameAnalyzer::getLeftHandPos() { 
  // p_LHandPosition = new Position(trackModel->partCentres[0].x,
  //                          trackModel->partCentres[0].y,
  //                          trackModel->partCentres[0].z);
  return Position(0);
};

Position FrameAnalyzer::getRightHandPos() { 
  // p_RHandPosition = new Position(trackModel->partCentres[0].x,
  //                          trackModel->partCentres[0].y,
  //                          trackModel->partCentres[0].z);
  return Position(0);
};

int getHandAction() {
  // To implement
  return -1;
}

}  // namespace
