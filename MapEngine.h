/*
 * GridMap.h
 *
 *  Created on: 2015年7月28日
 *      Author: victor
 */

#ifndef MAPENGINE_H_
#define MAPENGINE_H_
#include "Point.h"
#include "Intersection.h"
#include "Segment.h"

#include <map>
#include <set>
#include <vector>
#include <string>
#include <set>
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <opencv/cv.h>
#include <opencv/cv.hpp>
#include <opencv2/opencv.hpp>
#include <glog/logging.h>
#include <QImage>
#include <QString>
#include <QHash>
#include <QColor>
#include <QPainter>
#include <QString>

class MapEngine {
public:

//constructors and destructor:

	//No default initializer
//	MapEngine();
    MapEngine();
	virtual ~MapEngine();

//public methods:
    //methods to initialize datum, those method should only invoke once!
    //loadPng() and loadGm() should not invoked both:
    QImage* loadGm(const std::string& gridMapFilename);
    bool loadPng(const std::string& gridMapFilename);
    bool loadCarposes(const std::string& carposesFilename);
    QImage* getImage(){
        return m_image;
    }

    void setImage(QImage* image){
        m_image = image;
        return;
    }

    bool tanslateImage(double scale){
        if(!m_image){
            DLOG(FATAL) << "m_image is empty.";
            return false;
        }
        if(scale == 1){
            return true;//scale == 1 is tedious
        }

        int newWidth = m_image->width()*scale;
        int newHeight = m_image->height()*scale;
        QImage * tmpImage = new QImage(newWidth, newHeight, QImage::Format_RGB32);
        //defines how many pixels to merge into a single pixel.
        //if scale > 1, pixels are divided instead of merged. But we don't need to scale up right now, scale down is good enough.
        double numOfPix = 1 / scale;
        for(double yy = 0; yy < m_image->height(); yy += numOfPix){//for each rows in the scaled image
            for(double xx = 0; xx < m_image->width(); xx += numOfPix){//for each cols in the scaled image
                if(scale < 1){
                    std::map<PointType, double> typesCount;
                    for(int j = std::floor(yy); j <= std::ceil(yy + numOfPix); ++j ){//for each row of affected pixels in old image
                        for(int i = std::floor(xx); i <= std::ceil(xx + numOfPix); ++i){//for each col of affected pixels in old image
                            double xCoefficient = 1, yCoefficient = 1;
                            if(xx - i > 0) //current (i,j) is on top most
                                xCoefficient = xx - i;
                            else if(i - xx - numOfPix > 0)//current (i,j) is on bottom
                                xCoefficient = xx + numOfPix - i;//end if(xCoefficient)
                            if(yy - j > 0)//current (i,j) is on left most
                                yCoefficient = yy -j;
                            else if(j - yy - numOfPix > 0)//current (i,j) is on right most
                                yCoefficient = yy + numOfPix - j;//end if(yCoefficient)
                            if(m_image->rect().contains(i,j)){//it is possible that (i,j) overflow the range of the image
                                typesCount[this->getPointType(i, m_image->height() - 1 - j)] += xCoefficient * yCoefficient;
                            }
                        }//end for(i)
                    }//end for(j)
                    PointType decidedType = BLANK;//firstly, initialize it to BLANK
                    double maxValue = 0;
                    for (auto p : typesCount){
                        if(p.second > maxValue){
                            maxValue = p.second;
                            decidedType = p.first;
                        }
                    }// end for(auto p ...)
                    //write the decided pixel type on the newly scaled image
                    ((QRgb*)(tmpImage->scanLine(int(yy/numOfPix))))[int(xx/numOfPix)]=COLORS[decidedType];
                }//end if(scale < 1)
                /*else{
                    //the case of scale > 1 is ommited, since we don't need it for the moment
                }*/
            }//end for(xx)
        }//end for(yy)
        //after computation, replace the old one with the new one
        delete m_image;
        m_image = tmpImage;
        return true;
    }//end func translateImage

	//draw detected segments on grid map. this method is just for viusalization, don't use it in production environment
	void drawSegments();
    void detectTurningPoints() ;
	void divideCarTrackIntoSegments();
	//given a line and a point outside the line. we want to find another line parallel to current one, amid the point and the line.
	//this method could be used to find LANE CENTER line.
	cv::Vec4f midLine(const cv::Vec4f& line, const cv::Point& pt);
	//for each segment, find their scan baseline. get prepared for further scanning.
	void setSegmentScanBaseline();

	//start scanning for LANE LINE and ROAD BOUNDARY
	void scanSegment();
    Point3B get3b(int xx, int yy);
    bool write3b(const std::string &fileName);
    bool toPng(const std::string &fileName);
    QImage* load3b(const std::string &fileName, int xDimension = 1000, int yDimension = 1000);
    //The replacePointsInCircle() is mainly used by map editors.
    //NOTE: the third param is a std::set, so when calling this function, you should wrap your PointTypes in '{}' to get a std::set
    //The reason for using std::set is that you could replace more than one type with a single call of the function
    //In addition, all the functions involved were inlined, so it should behave very efficiently
    void replacePointsInCircle(int centerX, int centerY, int radius, std::set<PointType> currentTypes, PointType replaceType)
    {
        for(int x = -radius; x<=radius; ++x){
            for(int y = -radius; y<=radius; ++y){
                if(pointInImage(centerX+x,centerY+y) && cv::norm(cv::Point(x,y))<=radius &&  currentTypes.count(getPointType(centerX+x,centerY+y))){
                    drawPoint(centerX+x,centerY+y,COLORS[replaceType]);
                }
            }
        }
    }

	static cv::Scalar getRandColor(){
		unsigned char r, g, b;
		r = rand() % 256;
		g = rand() % 256;
		b = rand() % 256;
		//cout << '(' << (short) r << ',' << (short) g << ',' << (short) b << ")\n";
		return cv::Scalar(b, g, r);
	}

	//note: because this class made heavy use of this function, so it should be inlined, defined in header, not in source file
	bool isPointNearby(const cv::Point& pt, const PointType type, rangetype range) const{
		for(rangetype i = -range; i <= range; ++i){
			for(rangetype j = -range; j <=range; ++j){
				//the cv::norm function returns the norm (length between (0,0) and (i,j) )
				if(cv::norm(cv::Point(i,j))<=range){
					if (getPointType(pt.x+i, pt.y+j)==type){
//						DLOG(INFO)<<"Found:"<<(int)getPointType(pt.x+i, pt.y+j)<<std::endl;//for debug
						return true;
					}
				}
			}
		}
		return false;
	};
	bool isPointNearBoundary(const cv::Point& pt) const{
        return isPointNearby(pt, CURB, 5)
				||isPointNearby(pt, TREE, 5)
				||isPointNearby(pt, TRUNK, 5)
				||isPointNearby(pt, TRAFFICSIGN, 5);
	}
	//
	int howManyPointsNearby(const cv::Point& pt, const PointType type, rangetype range) const{
		int num = 0;
		for(rangetype i = -range; i <= range; ++i){
			for(rangetype j = -range; j <=range; ++j){
				//the cv::norm function returns the norm (length between (0,0) and (i,j) )
				if(cv::norm(cv::Point(i,j))<=range){
					if (getPointType(pt.x+i, pt.y+j)==type){
//						DLOG(INFO)<<"Found:"<<(int)getPointType(pt.x+i, pt.y+j)<<std::endl;//for debug
						++num;
					}
				}
			}
		}
		return num;
	};

	//The 'R' channel value of the image is exactly the same as PointType definition
    PointType getPointType(const int xx, const int yy) const{
        if(pointInImage(xx,yy)){
            return (PointType)qRed(m_image->pixel(xx,m_image->height()-1-yy));
		}else{
			return BLANK;
		}
	}

	void test(const float dottedLaneLineThreshold, const float solidLaneLineThreshold,
			const float curbThreshold);//for debug
	cv::Point getLineIntersectPoint(const cv::Vec4f& line1, const cv::Vec4f& line2);
	void bresenhamPoints(const cv::Point& point1, const cv::Point& point2, std::vector<cv::Point>& container);
		//the following bresenham method is not very straightforward, the intention is: given
		//a lane line (of type cv::Vec4f), the begin and end point of car track, find out points
		//of the lane line.
		void bresenhamPoints(const cv::Point& point1, const cv::Point& point2, cv::Vec4f line, std::vector<cv::Point>& container);
		//this is a method just for viewing a segment of points, debug using
		void viewPoints(const std::vector<cv::Point>& points){
			if(points.size()==0){
				DLOG(ERROR)<<"points vector is empty!\n";
				return;
			}
//				for(auto pt: points){
//					DLOG(INFO)<<pt<<std::endl;
//				}
			cv::Rect rect(points.front(),points.back());
			cv::Mat tmp(image, rect);
			cv::Mat part = tmp.clone();
			cv::Scalar color = getRandColor();
			int xOffset = points.front().x<points.front().x?points.front().x:points.front().x;
			int yOffset = points.front().y<points.front().y?points.front().y:points.front().y;
			for(auto pt: points){
				if(pt.x>=0&&pt.y>=0&&pt.x<=image.cols&&pt.y<=image.rows){
					cv::circle(part, cv::Point(pt.x-xOffset,pt.y-yOffset), 1, color,-1);
				}
			}
			cv::namedWindow( "Display window", cv::WINDOW_AUTOSIZE ); // Create a window for display.
			cv::imshow( "Display window", part );                // Show our image inside it.
//			cv::waitKey(0);
		}
		//this method removes CURB, INTERSECTION outliers
		void removeOutlier();

//public member variables:
	//define different color for each kind of point,
	//will be initialized in constructor
    QHash<PointType, QRgb> COLORS;

	//variables to store basic grip map info
	std::vector<carpos> carTrack;
    size_t totalPointNum;


	//variables to store topology map info:
	std::vector<Segment* > segments;


private:
//private member variables, they all get initialized in constructor:

	int checkInterval;
	int distanceInterval;
	float eulrChangeThreshold;
	std::map<PointType, float> occurRateThreshold;
    QPainter *imagePainter;

	//create a opencv image mat to store most of the info (except for car track)
    //IMPORTANT! When migrating to QT, QImage is preferred than cv::Mat,
    //I'm considering eliminate the use of cv::Mat
	cv::Mat image;
    QImage *m_image;
	//all turning point within any intersection
	std::set<carpos> carTrackTurningPoints;
	//since some turning point detected by method detectTurningPoints()
	//might not reside in any intersection, those were stored here
	std::set<carpos> suspectedTurningPoints;
	std::set<carpos> carTrackUturnPoints;

//private method:
	//find terminal points of certain segment
	void terminalPoints(const cv::Point& startPoint, const cv::Vec4f& direction, std::vector<cv::Point>& container,std::vector<cv::Point>& rightContainer);

	//Give this method a vector of points, it counts num of occurrence of each different point type.
	void pointsStatistics(const std::vector<cv::Point>& points, std::map<PointType, int>& statistics);
    void drawPoint(int xx, int yy, QRgb color){
        ((QRgb*)(m_image->scanLine(m_image->height()-1-yy)))[xx]=color;
    }
	void drawPoints(const std::vector<cv::Point>& points, const cv::Scalar& color=getRandColor());
	void drawCircle(const cv::Point& pt, const cv::Scalar& color, int radius = 1){
		if(pt.x>=0&&pt.y>=0&&pt.x<=image.cols&&pt.y<=image.rows){
			cv::circle(image, pt, radius, color,-1);
		}
	}
    bool pointInImage(int xx, int yy) const{
        return xx>=0&&xx<m_image->width()&&yy>=0&&yy<m_image->height();
    }

	void view(cv::Rect rect){
		cv::Mat part(image, rect);
		cv::namedWindow( "Display window", cv::WINDOW_AUTOSIZE ); // Create a window for display.
		cv::imshow( "Display window", part );                // Show our image inside it.
		cv::waitKey(0); // Wait for a keystroke in the window
	}


//private temp variables, just for debuging
};

#endif /* MAPENGINE_H_ */
