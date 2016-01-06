#include "point.h"

// Constructors/Destructors
//  

UgvPoint::UgvPoint()
: eulr(0)
{
	x=0;
	y=0;
}

UgvPoint::~UgvPoint() {
}

//  
// Methods
//  

// Accessor methods
//  

// Other methods
//  

UgvPoint::UgvPoint(const pointT& pt)
: eulr(0)
{
	x=pt.x;
	y=pt.y;
}

UgvPoint::UgvPoint(int _x, int _y)
: eulr(0)
{
	x=_x;
	y=_y;
}

UgvPoint::UgvPoint(const carpos& cp)
: eulr(cp.eulr)
{
	x=cp.x;
	y=cp.y;
}

UgvPoint::UgvPoint(int _x, int _y, float _eulr)
: eulr(_eulr)
{
	x = _x;
	y = _y;
}

int UgvPoint::distance(const UgvPoint& second) const {
	return (int)sqrt(pow(this->x-second.x,2)+pow(this->y-second.y,2));
}

UgvPoint::UgvPoint(const UgvPoint& pt)
: eulr(pt.eulr)
{
	x=pt.x;
	y=pt.y;
}

std::string UgvPoint::toStr() {
	//TODO: output rndf compatible string
	return "";
}

UgvPoint::UgvPoint(const cv::Point& pt)
:eulr(0)
{
	x=pt.x;
	y=pt.y;
}

int UgvPoint::distance(const int xx, const int yy) const {
	return (int)sqrt(pow(this->x-xx,2)+pow(this->y-yy,2));
}
