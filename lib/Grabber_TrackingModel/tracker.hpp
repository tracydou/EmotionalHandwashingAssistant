
#ifndef TRACKER_
#define TRACKER_

//OpenCV libraries
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

// openni
#include "ni/XnCppWrapper.h"

// NITE
#include "nite/XnV3DVector.h"

// Open MP
#include <omp.h>

#include "gtk/gtk.h"
#include <gdk/gdkkeysyms.h>

#include <fstream>
#include <dirent.h>
#include <iomanip>
#include <limits>

#include "tree.h"
#include "defines.h"
#include "grab.h"
#include "kalmanfilter.h"

#define BILLION  1000000000L;

// functions got from changing signatures of original funcs
int main_start(int argc, char** argv);

// wrapper functions
Point3_<float> getLeftHandPos();
Point3_<float> getRightHandPos();
int getHandAction();

#endif  // TRACKER_
