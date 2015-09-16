#include "Ring.h"


Ring::Ring() :center(Point2f(0, 0)), radius(0), ring_mask()
{
}

Ring::Ring(const Point2f & cen, float radi, const Mat & mask) : center(cen), radius(radi), ring_mask(mask)
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

Mat Ring::getRing_mask()
{
	return this->ring_mask; 
}


