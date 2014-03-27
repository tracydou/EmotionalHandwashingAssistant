/*
 *  File:				defines.hpp
 *  Created by:			Luyuan Lin
 *  Created:			March 2014
 *  Last Modified:		March 2014
 *  Last Modified by:	Luyuan Lin
 *  Shared definitions among all EHwA src files
 */

#ifndef DEFINES_
#define DEFINES_

//OpenCV libraries
#include "opencv2/opencv.hpp"

using cv::Point3_;

namespace EHwA {

typedef Point3_<float> Position;
  
const int MAX_RESPOND_BUFFER_SIZE = 2048;

}  // namespace EHwA

#endif  // DEFINES_
