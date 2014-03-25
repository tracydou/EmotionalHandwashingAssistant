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

// Randomly propose a set of numSamples pixels per image for training
int FrameAnalyzer::createSamples(int numSamples){
	p_actionTracker -> samples.clear();
	int x,y;
	for(int i=0; i < numSamples; i++){
		x = rand()%((WIDTH-1)-0)+0;
		y = rand()%((HEIGHT-1)-0)+0;
		p_actionTracker -> samples.push_back(Point(x,y));
	}
	return 1;
}

getLeftHandPos() { return leftHandPos };
getRightHandPos() { return ..};
getHandAction() {return ..}
