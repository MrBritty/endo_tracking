#include "Ring.h"


Ring::Ring() :center(Point2f(0, 0)), radius(0), roi()
{
}

Ring::Ring(const Point2f & cen, float radi, const Rect & region) : center(cen), radius(radi),  roi(region)
{
}


Point2f Ring::getCenter()
{
	return this->center;
}

float Ring::getRadii()
{
	return this->radius;
}
//
//Mat Ring::getRing_mask()
//{
//	return this->ring_mask; 
//}

Rect Ring::getRoi()
{
	return this->roi;
}


