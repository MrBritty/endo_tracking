#ifndef TOOLS_TRACKING_APP_HPP
#define TOOLS_TRACKING_APP_HPP

#include <opencv2/video/tracking.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/background_segm.hpp>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <set>
#include <functional>
#include <numeric>
#include <sstream>
#include <cstdlib>
#include <time.h>
#include <fstream>
#include "ContourFeature.hpp"
#include "Transformation.hpp"
#include "Tool.hpp"
#include "ToolBox.hpp"
#include "BoundingBox.hpp"
#include "Ring.h"
#include "State_Machine.h"

#define SSTR( x ) dynamic_cast< std::ostringstream & >( \
	( std::ostringstream() << std::dec << x ) ).str()

using namespace cv;
using namespace std;

class ToolsTrackingApp {
public:
  ToolsTrackingApp();
  ~ToolsTrackingApp();

  int init(const char*);    
  void run();
  

private:
  //Apply background substraction and denoising
	bool segmentation_tool(const cv::Mat &, vector<Point> &, int *);
	void segmentation_ring(const vector<Point> &contour_tool, Mat& mask_ring);
  
  //True is contour is noise (too small...)
  bool isNotAContour(ContourFeature&, int);
  bool isNotARing(ContourFeature&, int);
  
  //floats<->integers array converters
  void convertToFloat(const vector<Point>&, vector<Point2f>&);
  void convertToInt(const vector<Point2f>&, vector<Point>&);
  
  //Handles keyboard events
  void keyboardEvent(char);
  
  //Contains detected tools
  ToolBox toolbox;

  fstream logger;
  

  std::vector<cv::Point2f> trackedPoints;
  cv::VideoCapture capture;
  cv::VideoWriter outputVideo;
  cv::BackgroundSubtractor* pMOG;
    
  cv::Mat element[4];

  cv::Mat frame;
  cv::Mat fgMask;
  cv::Mat gray;
  cv::Mat prevGray;
  cv::Mat fgMaskRing_R;
  cv::Mat fgMaskRing_B;
  cv::Mat fgMaskRing_O;
  cv::Rect roi;
  cv::Mat temp_mat;
};
#endif