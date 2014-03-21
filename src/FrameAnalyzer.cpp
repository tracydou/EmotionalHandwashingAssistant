/*
 *  File:				FrameAnalyzer.hpp
 *  Created by:			Luyuan Lin
 *  Created:			March 2014
 *  Last Modified:		March 2014
 *  Last Modified by:	Luyuan Lin
 *  
 */

FrameAnalyzer::FrameAnalyzer(){
  // init p_actionTracker & p_faceRecoganizer
}

FrameAnalyzer::~FrameAnalyzer(){
  // do nothing	
}
  
void FrameAnalyzer::Analyze(Frame frame) {
  p_actionTracker->update(frame.image1, frame.image2, frame.image3);
  p_faceRecoganizer->update(frame);
}

getLeftHandPos() { return leftHandPos };
getRightHandPos() { return ..};
getHandAction() {return ..}
