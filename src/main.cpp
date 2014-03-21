/*
 *  File:				main.cpp
 *  Created by:			Luyuan Lin
 *  Created:			March 2014
 *  Last Modified:		March 2014
 *  Last Modified by:	Luyuan Lin
 * 
 *  Defined the main function for the whole EHwA system.
 */

enum Stage {
  INIT,
  TURN_ON_SHUI_LONG_TOU,
  INIT_WASHING,
  SOAP,
  RUBBING,
  WASHING_SOAP,
  TURN_OFF,
  END
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
