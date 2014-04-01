/*
 *  File:				frame_analyzer.hpp
 *  Created by:			Luyuan Lin
 *  Created:			March 2014
 *  Last Modified:		March 2014
 *  Last Modified by:	Luyuan Lin
 *  
 */

#include "frame_analyzer.hpp"

namespace EHwA {
	
FrameAnalyzer::FrameAnalyzer(){
  // init p_action_tracker_ & p_face_recoganizer_
}

FrameAnalyzer::~FrameAnalyzer(){
  // do nothing	
}
  
void FrameAnalyzer::Analyze() {
  // p_action_tracker_->update(frame.image1, frame.image2, frame.image3);
  // p_face_recoganizer_->update(frame);
}

// Randomly propose a set of numSamples pixels per image for training
int FrameAnalyzer::CreateSamples(int numSamples){
/*
 * 	p_action_tracker_ -> samples.clear();
	int x,y;
	for(int i=0; i < numSamples; i++){
		x = rand()%((WIDTH-1)-0)+0;
		y = rand()%((HEIGHT-1)-0)+0;
		p_action_tracker_ -> samples.push_back(Point(x,y));
	}
	* */
	return 1;
}

Position FrameAnalyzer::get_left_hand_position() { 
  // p_LHandPosition = new Position(trackModel->partCentres[0].x,
  //                          trackModel->partCentres[0].y,
  //                          trackModel->partCentres[0].z);
  return Position(0);
};

Position FrameAnalyzer::get_right_hand_position() { 
  // p_RHandPosition = new Position(trackModel->partCentres[0].x,
  //                          trackModel->partCentres[0].y,
  //                          trackModel->partCentres[0].z);
  return Position(0);
};

int FrameAnalyzer::get_hand_action() {
  // To implement
  return -1;
}

}  // namespace
