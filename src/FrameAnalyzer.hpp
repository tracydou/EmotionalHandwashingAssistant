/*
 *  File:				FrameAnalyzer.hpp
 *  Created by:			Luyuan Lin
 *  Created:			March 2014
 *  Last Modified:		March 2014
 *  Last Modified by:	Luyuan Lin
 *  
 */

#ifndef FRAME_ANALYZER_
#def FRAME_ANALYZER_

//include file defining trackingModel
//include file defining FacialExpressionRecoganizer

class FrameAnalyzer {
  public:
    FrameAnalyzer();
    ~FrameAnalyzer();
  
    void Analyze(Frame frame);
  
    // we can either return p_actionTracker->getLeftPos() here directly if we do not need to interprete it
    getLeftHandPos();
    getRightHandPos();
    getHandAction();
 
  protected:
    trackingModel* p_actionTracker;
    FacialExpressionRecoganizer* p_facialExpressionRecoganizer;
};

#endif  // FRAME_ANALYZER_
