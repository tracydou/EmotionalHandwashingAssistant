/*
 *  File:				frame_analyzer.hpp
 *  Created by:			Luyuan Lin
 *  Created:			March 2014
 *  Last Modified:		March 2014
 *  Last Modified by:	Luyuan Lin
 *  
 */

#ifndef FRAME_ANALYZER_
#define FRAME_ANALYZER_

#include "defines.hpp"

namespace EHwA {

// TODO: add "facial expressions" as an influencer as well
class FrameAnalyzer {
  public:
    FrameAnalyzer();
    ~FrameAnalyzer();
  
    void Analyze();

    Position get_left_hand_position();
    Position get_right_hand_position();
    int get_hand_action();
 
  protected:
    int CreateSamples(int numSamples);
  
    // trackingModel* p_action_tracker_;
    
    // constants copied from lib/Grabber_TrackingModel/defines.h
    static const int WIDTH = 640; // for cropped image
    static const int HEIGHT = 480; // for cropped image
    static const int RUN_PIXELS = 3000; // random pixels per image for classification for classification
};

}  // namespace EHwA

#endif  // FRAME_ANALYZER_
