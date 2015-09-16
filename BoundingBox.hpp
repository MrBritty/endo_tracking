#ifndef BOUNDING_BOX_HPP
#define BOUNDING_BOX_HPP

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class BoundingBox 
{

public:
  //Default constructor
  BoundingBox();

  //Find the bounding box of a given set of points
  void find(std::vector<cv::Point>&);
  
  //Asssign bounding box to the given parameter
  void get(cv::RotatedRect&) const;

  //Draw the box...
  void draw(cv::Mat&) const;
  
  //Display box caracteristics (center, angle)
  void info() const;

protected:
  cv::Point2f vertices[4];
  cv::RotatedRect box; //DON'T USE box.center
  
};

#endif
