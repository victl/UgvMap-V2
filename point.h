#ifndef POINT_H
#define POINT_H

#include <string>
#include <vector>
#include <opencv/cv.h>
#include <opencv/cv.hpp>
#include <opencv2/opencv.hpp>
#include <cmath>
#include <cstdlib>
#include <QHash>
#include <QRgb>

typedef short rangetype;
typedef int coordinate;
typedef unsigned short int shortCoordinate;


struct pointT {
	shortCoordinate x, y;
//	int x,y;

//the operators below is overloaded to support std::map indexing
	bool operator <(const pointT other) const {
		if (x != other.x)
			return x < other.x;
		return y < other.y;
	}
	bool operator ==(const pointT other) const {
		return abs(x - other.x) < 2 && abs(y - other.y) < 2;
	}
	bool operator !=(const pointT other) const {
		return x != other.x || y != other.y;
	}
};

struct carpos:public cv::Point {
	carpos():cv::Point(),eulr(0){}
	carpos(coordinate _x, coordinate _y)
	:cv::Point(_x,_y),eulr(0){}
	carpos(coordinate _x, coordinate _y, float _eulr)
	:cv::Point(_x,_y),eulr(_eulr){}
	float eulr;
	//the operators below is overloaded to support std::map indexing
		bool operator <(const carpos other) const {
			if (x != other.x)
				return x < other.x;
			return y < other.y;
		}
		bool operator ==(const carpos other) const {
			return abs(x - other.x) < 2 && abs(y - other.y) < 2;
		}
		bool operator !=(const carpos other) const {
			return x != other.x || y != other.y;
		}
};

struct Point3B {
public:
    //public member variable
    unsigned char base;
    unsigned char road;
    unsigned char sig;
    //public method:
    Point3B():
        base(0)
      , road(0)
      , sig(0)
    {
    }
};

//NOTE: "unsigned char" for enum is a new feature introduced in c++11 standard
//so you have to enable c++11 support of compilers
//enum class PointType: unsigned char {
enum PointType {
	LANELINE = 254,
	ZEBRA = 253,
	INTERSECTION = 3,
	CURB = 4,
	TREE = 210,
	TRUNK = 6,
	PIT = 252,
	LANECENTER = 8,
	CARTRACK = 128,
	TRAFFICSIGN = 255,
    BLANK = 127, //'BLANK' means no specific type. Blank point on road should turn into 'CLEAR' point before '3b map file' output
	DOTTEDLANELINE = 251,
    SOLIDLANELINE = 250,
    CLEAR = 249, //'CLEAR' means point could pass trough
    OCCUPIED = 0,
};

//enum EditorType{
//    HAND = 0,
//    ROADCLEANER = 1,
//    FINEERASER = 2,
//    AGGRESSIVEERASER = 3,
//    CURBDRAWER = 4,
//    PENDRAWER = 5,
//};

static QHash<PointType, QRgb> COLORS ={
                { LANELINE, qRgb(LANELINE, 255, 252) },//WHITE
                { ZEBRA, qRgb(ZEBRA, 255, 0) },//LEMON YELLOW
                { INTERSECTION, qRgb(INTERSECTION, 255, 250) },//SKY BLUE
                { CURB, qRgb(CURB, 255, 0) },//GREEN
                { TREE, qRgb(TREE, 105, 30) },//PINK
                { TRUNK, qRgb(TRUNK, 0, 255) },//DEEP BLUE
                { PIT, qRgb(PIT, 0, 255) },//PURPLE
                { LANECENTER, qRgb(LANECENTER, 100, 100) },//DARK GREEN
                { CARTRACK, qRgb(CARTRACK, 192, 220) },//GRAY BLUE
                { TRAFFICSIGN, qRgb(TRAFFICSIGN, 0, 0) },//RED
                { BLANK, qRgb(BLANK, 127, 127) },//MID GRAY
                { CLEAR, qRgb(CLEAR, 255, 255) }, //WHITE
                { OCCUPIED, qRgb(OCCUPIED, 0, 0) } //BLACK
        };

//define binary values of Point3B.base
static const unsigned char ROADEDGE_UNKNOWN = 0;
static const unsigned char ROADEDGE_CLEAR = 64;
static const unsigned char ROADEDGE_OCCUPIED = 128;
static const unsigned char ROADEDGE_DYNAMIC = 192;
static const unsigned char OBSTACLE_NONE = 0;
static const unsigned char OBSTACLE_STATIC = 8;
static const unsigned char OBSTACLE_PEDESTRIAN = 16;
static const unsigned char OBSTACLE_BIKE = 24;
static const unsigned char OBSTACLE_MOTO = 32;
static const unsigned char OBSTACLE_CAR = 40;
static const unsigned char OBSTACLE_BUS = 48;
static const unsigned char OBSTACLE_TRUCK = 56;
static const unsigned char LANELINE_NONE = 0;
static const unsigned char LANELINE_DOTTED = 2;
static const unsigned char LANELINE_SOLID = 4;
static const unsigned char LANELINE_DOUBLE = 6;
static const unsigned char STOPLINE_NO = 0;
static const unsigned char STOPLINE_YES = 1;

//define binary values of Point3B.road
static const unsigned char CURB_NO = 0;
static const unsigned char CURB_YES = 128;
static const unsigned char FENCERAMP_NO = 0;
static const unsigned char FENCERAMP_CITY = 32;
static const unsigned char FENCERAMP_HIGHWAY = 64;
static const unsigned char FENCERAMP_RAMP = 96;
static const unsigned char REGION_STRUCTURED = 0;
static const unsigned char REGION_INTERSECTION = 8;
static const unsigned char REGION_UTURN = 16;
static const unsigned char REGION_RIM = 24;
static const unsigned char ARROW_NONE = 0;
static const unsigned char ARROW_STRAIGHT = 1;
static const unsigned char ARROW_LEFT = 2;
static const unsigned char ARROW_RIGHT = 3;
static const unsigned char ARROW_UTURN = 4;
static const unsigned char ARROW_STRAIGHTLEFT = 5;
static const unsigned char ARROW_STRAIGHTRIGHT = 6;

//define binary values of Point3B.sig
static const unsigned char LAMP_NONE = 0;
static const unsigned char LAMP_ROUND = 64;
static const unsigned char LAMP_ARROW = 128;
static const unsigned char LAMP_GROUND = 192;
//the definition of traffic sign is uncertain for the moment, to be added in the future


/**
 * class Point
 *
 */

class UgvPoint: public cv::Point2d {
public:

	// Constructors/Destructors
	//
	UgvPoint(const pointT& pt);
	UgvPoint(int _x, int _y);
	UgvPoint(const carpos& cp);
	UgvPoint(int _x, int _y, float _eulr);
	UgvPoint(const UgvPoint& pt);
	UgvPoint(const cv::Point& pt);

	/**
	 * Empty Constructor
	 */
	UgvPoint();

	/**
	 * Empty Destructor
	 */
	virtual ~UgvPoint();

	// Static Public attributes
	//

	// Public attributes
	//
	float eulr ;

	// Public attribute accessor methods
	//

	// Public attribute accessor methods
	//

	/**
	 * return the distance between self and the second point
	 * @return int
	 * @param  second
	 */
	int distance(const UgvPoint& second) const;
	int distance(const int xx, const int yy) const;

	/**
	 * output self according to RNDF format
	 * @return string
	 */
	std::string toStr();

	bool operator <(const UgvPoint other) const {
		if (x != other.x)
			return x < other.x;
		return y < other.y;
	}
	bool operator ==(const UgvPoint other) const {
		return abs(x - other.x) < 2 && abs(y - other.y) < 2;
	}
	bool operator !=(const UgvPoint other) const {
		return x != other.x || y != other.y;
	}

protected:

	// Static Protected attributes
	//

	// Protected attributes
	//

public:

	// Protected attribute accessor methods
	//

protected:

public:

	// Protected attribute accessor methods
	//

protected:

private:

	// Static Private attributes
	//

	// Private attributes
	//

public:

	// Private attribute accessor methods
	//

private:

public:

	// Private attribute accessor methods
	//

private:

	void initAttributes();

};

#endif // POINT_H
