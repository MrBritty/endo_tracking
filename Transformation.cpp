#include "Transformation.hpp"

Transformation::Transformation():translation(0,0),angle(0.0){}


void Transformation::find(const cv::RotatedRect& oldRect, const cv::RotatedRect& newRect) 
{
  translation.x = (int)(newRect.center.x - oldRect.center.x);
  translation.y = (int)(newRect.center.y - oldRect.center.y);
  angle =  oldRect.angle - newRect.angle;
}

double Transformation::getAngle() const 
{
  return angle;
}

cv::Point* Transformation::getTranslation() const 
{
  return (cv::Point*)&translation;
}
