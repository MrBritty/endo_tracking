#include "Tool.hpp"


#define CRIT_PERIMETER 80
#define CRIT_DIST 200
#define CRIT_AREA 500
#define CRIT_ECCENTRICITY 0.115
#define CRIT_AREA_RATIO 0.015
#define CRIT_HULL_AREA 200
#define CRIT_SOLIDITY 0.015
#define CRIT_CONVEX_RATIO 0.015

#define COEFF_PERIMETER 4.0
#define COEFF_DIST 10.0
#define COEFF_AREA 4.0
#define COEFF_ECCENTRICITY 6.0
#define COEFF_AREA_RATIO 8.0
#define COEFF_HULL_AREA 4.0
#define COEFF_SOLIDITY 5.0
#define COEFF_CONVEX_RATIO 5.0

#define COEFF_TOTAL (COEFF_PERIMETER+COEFF_DIST+COEFF_AREA+COEFF_ECCENTRICITY+COEFF_AREA_RATIO+COEFF_HULL_AREA+COEFF_SOLIDITY+COEFF_CONVEX_RATIO)

#define VOTE(a,b,r) ((fabs(a-b))<r)?1:0
#define MIN_VOTES 4


Tool::Tool():features(),points(),boundingBox(),hits(0),confidence(0),id(4){}


Tool::Tool(const ContourFeature& cf, const cv::RotatedRect& r, const std::vector<cv::Point>& p):
  features(p), 
  points(),
  boundingBox(){}

void Tool::update(cv::RotatedRect& r, std::vector<cv::Point>& p) 
{
  points = p;
  boundingBox = r;
  features = ContourFeature(p);
}


void Tool::draw(cv::Mat& frame) const 
{
  for(int i=1; i<points.size(); i++)
  {
    line(frame, points[i-1], points[i], cv::Scalar(0,255,0), 1);
    circle(frame, points[i], 4, cv::Scalar(0,0,255), -1, 8);
  }
  //  line(frame, points[points.size()-1], points[0], cv::Scalar(255,255,255), 1);

  //std::ostringstream os;
  //os << "Tool ";
  //os << (size_t)id;
  //os << "(";
  //os << (double)confidence;
  //os << ")";
  //cv::Point org = features.getCentre()+cv::Point2d(20,20);
  //putText(frame, os.str(), org, 0, 0.4, cv::Scalar(255,255,255), 1, 8, false);
}


std::vector<cv::Point>& Tool::getPoints() const 
{
  return (std::vector<cv::Point>&)points;
}

// void Tool::lastTransformation(Transformation& t, const cv::RotatedRect& r) {
//   std::cout << "calc" << boundingBox.center<<std::endl;
//   std::cout << "calc" << r.center<<std::endl;  
//   t.find(boundingBox, r);
  
// }

void Tool::lastTranslation(cv::Point& result, const cv::Point& current) 
{
  cv::Point p = getFeatures().getCentre();
  result = current - p;
}

ContourFeature& Tool::getFeatures() const 
{
  return (ContourFeature&)features;
}

double Tool::compare(const ContourFeature& cf) 
{ 
  ContourFeature contour = getFeatures();
  int votes = 0;
  cv::Point2d p(contour.getCentre());

  int a=(cv::norm(p - cf.getCentre()) < CRIT_DIST)?1:0;

  votes += a * COEFF_DIST;
  votes += (VOTE(contour.getPerimeter(), cf.getPerimeter(), CRIT_PERIMETER) * COEFF_PERIMETER);
  votes += (VOTE(contour.getArea(), cf.getArea(), CRIT_AREA) * COEFF_AREA);
  votes += (VOTE(contour.getEccentricity(), cf.getEccentricity(), CRIT_ECCENTRICITY) * COEFF_ECCENTRICITY);
  votes += (VOTE(contour.getAreaRatio(), cf.getAreaRatio(), CRIT_AREA_RATIO) * COEFF_AREA_RATIO);
  votes += (VOTE(contour.getHullArea(), cf.getHullArea(), CRIT_HULL_AREA) * COEFF_HULL_AREA);
  votes += (VOTE(contour.getSolidity(), cf.getSolidity(), CRIT_SOLIDITY) * COEFF_SOLIDITY);
  votes += (VOTE(contour.getConvexRatio(), cf.getConvexRatio(), CRIT_CONVEX_RATIO) * COEFF_CONVEX_RATIO); 
  

  confidence = (double)votes/COEFF_TOTAL;

  return votes;
}

void Tool::hasBeenSelected() 
{
  hits++;
}

int Tool:: getSelections() const 
{
  return hits;
}

void Tool::setID(int a)
{
  id = a;
}  

int Tool::getID()
{
  return id;
}  

float Tool::getConfidence()
{
  return confidence;
}  


void Tool::info() const 
{
  features.info();
  std::cout << "BoundingBox position:" << boundingBox.center << std::endl;
  std::cout << "BoundingBox angle:" << boundingBox.angle << std::endl;

  for(std::vector<cv::Point>::const_iterator it = points.begin();
      it!=points.end(); 
      ++it) 
  {
    std::cout << *it << std::endl;
  }
}

  // std::cout << "***********" << std::endl;
  // std::cout << "perim " << contour.getPerimeter()-cf.getPerimeter() << std::endl;
  // std::cout << "aire " << contour.getArea()-cf.getArea() << std::endl;
  // std::cout << "excentri" << contour.getEccentricity()-cf.getEccentricity() << std::endl;
  // std::cout << "arearatio " << contour.getAreaRatio()-cf.getAreaRatio() << std::endl;
  // std::cout << "hullarea " << contour.getHullArea()-cf.getHullArea() << std::endl;
  // std::cout << "solid " << contour.getSolidity()-cf.getSolidity() << std::endl;
  // std::cout << " convexeration" << contour.getConvexRatio()-cf.getConvexRatio() << std::endl;
  // std::cout << std::endl;

