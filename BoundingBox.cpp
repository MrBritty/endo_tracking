#include "BoundingBox.hpp"

BoundingBox::BoundingBox():box()
{
  for(int i=0; i<4; i++)
  {
    vertices[i] = cv::Point(0,0);
  }
}

void BoundingBox::find(std::vector<cv::Point>& contour) 
{
  box = minAreaRect(contour);
  box.points(vertices);
}

void BoundingBox::get(cv::RotatedRect& r) const 
{
  r = box;
}

void BoundingBox::info() const 
{
  std::cout << "Box position:" << box.center << std::endl;
  std::cout << "Box angle:" << box.angle << std::endl;
}


void BoundingBox::draw(cv::Mat& frame) const 
{
  for(int k=1; k<4; k++){
    line(frame, vertices[k-1], vertices[k], cv::Scalar(255,255,255), 1);
  }
  line(frame, vertices[3], vertices[0], cv::Scalar(255,255,255), 1);     
}