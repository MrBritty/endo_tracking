#ifndef RING_HPP
#define RING_HPP

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

class Ring
{
public:
	Ring();
	Ring(const Point2f&, float, const Rect&);
	Point2f getCenter();
	float getRadii();
	//Mat getRing_mask();
	Rect getRoi();
protected:
	Point2f center;
	float  radius;
	//Mat ring_mask;
	Rect roi;
};

#endif

