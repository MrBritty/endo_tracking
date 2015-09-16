#ifndef TRANSFORMATION_HPP
#define TRANSFORMATION_HPP

#include <opencv2/core/core.hpp>

class Transformation 
{

public:
  //Default constructor (translation=(0,0), angle=0)
  Transformation();
  
  //Find transformation (translation+rotation only) between two given rectangles 
  void find(const cv::RotatedRect&, const cv::RotatedRect&);
  
  //Returns rotation angle
  double getAngle() const;
  
  //Returns translation vector
  cv::Point* getTranslation() const;

protected:
  cv::Point translation;
  double angle;
};

#endif
