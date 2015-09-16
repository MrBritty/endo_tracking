#ifndef TOOL_HPP
#define TOOL_HPP

#include "ContourFeature.hpp"
#include "Transformation.hpp"
#include <cmath>

class Tool {
public:
  Tool();
  Tool(const ContourFeature&, const cv::RotatedRect&, const std::vector<cv::Point>&);
  void update(cv::RotatedRect&, std::vector<cv::Point>&);
  double compare(const ContourFeature&);
  void draw(cv::Mat&) const;
  std::vector<cv::Point>& getPoints() const;

  //    void lastTransformation(Transformation&, const cv::RotatedRect&);
  void lastTranslation(cv::Point&, const cv::Point&);
  void hasBeenSelected();
  int  getSelections() const;
  void setID(int a);
  int getID();
  float getConfidence();
  ContourFeature& getFeatures() const;
  void info() const;

protected:
  ContourFeature features;
  std::vector<cv::Point> points;
  cv::RotatedRect boundingBox;
  unsigned int hits;
  int id;
  float confidence;
};
#endif
