/*
 * GridMap.cpp
 *
 *  Created on: 2015年7月28日
 *      Author: victor
 */

#include "MapEngine.h"

//default initializer, IMPORTANT: don't use it, because c++ produce strange behavior
//GridMap::GridMap()
//:COLORS({
//				{ LANELINE, cv::Vec3b(252, 255, 254) },//WHITE
//				{ ZEBRA, cv::Vec3b(0, 255, 253) },//LEMON YELLOW
//				{ INTERSECTION, cv::Vec3b(250, 255, 3) },//SKY BLUE
//				{ CURB, cv::Vec3b(0, 255, 4) },//GREEN
//				{ TREE, cv::Vec3b(30, 105, 210) },//PINK
//				{ TRUNK, cv::Vec3b(255, 0, 6) },//DEEP BLUE
//				{ PIT, cv::Vec3b(255, 0, 252) },//PURPLE
//				{ LANECENTER, cv::Vec3b(100, 100, 8) },//DARK GREEN
//				{ CARTRACK, cv::Vec3b(220, 192, 128) },//GRAY BLUE
//				{ TRAFFICSIGN, cv::Vec3b(0, 0, 255) },//RED
//				{ BLANK, cv::Vec3b(127, 127, 127) },//MID GRAY
//		}),
//	occurRateThreshold({
//		{ DOTTEDLANELINE, 0.1 },
//		{ SOLIDLANELINE, 0.2 },
//		{ CURB, 0.3 }
//	}),
//	maxX(0),
//	maxY(0),
//	minX(0),
//	minY(0),
//	gridMapFilename(""),
//	carposesFilename(""),
//	checkInterval(15),
//	distanceInterval(100),
//	eulrChangeThreshold(0.03f)
//{
//	DLOG(INFO)<<"ERROR: Grid map filename and carposes filename not specified."
//			"\nProgram will exit...\n";
//	exit(-1);
//}

//Note: the value of COLOR is chosen very carefully, especially the 'R' value, which exactly reflect the actual point type,
//see enum PointType definition in Point.h
MapEngine::MapEngine()
    :COLORS({
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
        }),
	occurRateThreshold({
		{ DOTTEDLANELINE, 0.1 },
		{ SOLIDLANELINE, 0.2 },
		{ CURB, 0.3 }
    }),
	checkInterval(15),
	distanceInterval(100),
    eulrChangeThreshold(0.03f),
    m_image(0),
    imagePainter(0)

{
}

MapEngine::~MapEngine() {
	for(auto seg: segments){
		delete seg;
	}
	//google::ShutdownGoogleLogging();
}

QImage* MapEngine::loadGm(const std::string& gridMapFilename) {
    //STEP 1: read grip map file
	std::ifstream mapin(gridMapFilename, std::ios::binary);
    //note: sizeOfMap should be of type 'size_t', only use 'int' with older .gm files
    size_t sizeOfMap;
	mapin.read((char*) &sizeOfMap, sizeof(sizeOfMap));
	this->totalPointNum = sizeOfMap;
	DLOG(INFO) << "size of record:" << sizeOfMap << std::endl;
	//Note: input data should predefine minX,minY,maxX,maxY, so they needn't re-calculate.
    int minX=0,minY=0,maxX=0,maxY=0;
    mapin.read((char*)&minX, sizeof(minX));mapin.read((char*)&minY, sizeof(minY));
    mapin.read((char*)&maxX, sizeof(maxX));mapin.read((char*)&maxY, sizeof(maxY));
    //create the image.
    if(m_image)
        delete m_image;
    m_image = new QImage(QSize(maxX-minX, maxY-minY),QImage::Format_RGB32);
	//set all background color to GRAY:
    imagePainter = new QPainter(m_image);
    imagePainter->setBrush(QBrush(QColor(COLORS[BLANK])));
    imagePainter->drawRect(m_image->rect());
//    int minxx=-1,minyy=-1,maxxx=-1,maxyy=-1;
	for (size_t i = 0; i < sizeOfMap; ++i) {
		pointT pt;
		unsigned char value;
		mapin.read((char*) &pt, sizeof(pt));
		mapin.read((char*) &value, sizeof(value));
//        minxx = minxx > pt.x? pt.x:minxx;
//        maxxx = maxxx < pt.x? pt.x:maxxx;
//        minyy = minyy > pt.y? pt.y:minyy;
//        maxyy = maxyy < pt.y? pt.y:maxyy;
        switch (value){
        case LANELINE:
            ((QRgb*)(m_image->scanLine(m_image->height()-pt.y)))[pt.x]=COLORS[LANELINE];
            break;
        case ZEBRA:
            ((QRgb*)(m_image->scanLine(m_image->height()-pt.y)))[pt.x]=COLORS[ZEBRA];
            break;
        case INTERSECTION:
            ((QRgb*)(m_image->scanLine(m_image->height()-pt.y)))[pt.x]=COLORS[INTERSECTION];
            break;
        case CURB:
            ((QRgb*)(m_image->scanLine(m_image->height()-pt.y)))[pt.x]=COLORS[CURB];
            break;
        case TREE:
            ((QRgb*)(m_image->scanLine(m_image->height()-pt.y)))[pt.x]=COLORS[TREE];
            break;
        case TRUNK:
            ((QRgb*)(m_image->scanLine(m_image->height()-pt.y)))[pt.x]=COLORS[TRUNK];
            break;
        case PIT:
            ((QRgb*)(m_image->scanLine(m_image->height()-pt.y)))[pt.x]=COLORS[PIT];
            break;
        case LANECENTER:
            ((QRgb*)(m_image->scanLine(m_image->height()-pt.y)))[pt.x]=COLORS[LANECENTER];
            break;
        case TRAFFICSIGN:
            ((QRgb*)(m_image->scanLine(m_image->height()-pt.y)))[pt.x]=COLORS[TRAFFICSIGN];
            break;
        //NOTE: IMPORTANT!!!!!!!!!!!!
        //the following cases are for older datum, new system will generate proper value
        case 1://OLD LANELINE
            ((QRgb*)(m_image->scanLine(m_image->height()-pt.y)))[pt.x]=COLORS[LANELINE];
            break;
        case 2://OLD ZEBRA
            ((QRgb*)(m_image->scanLine(m_image->height()-pt.y)))[pt.x]=COLORS[ZEBRA];
            break;
        case 5://OLD TREE
            ((QRgb*)(m_image->scanLine(m_image->height()-pt.y)))[pt.x]=COLORS[TREE];
            break;
        case 7://OLD PIT
            ((QRgb*)(m_image->scanLine(m_image->height()-pt.y)))[pt.x]=COLORS[PIT];
            break;
        default:
//            DLOG(INFO)<<"value not recognized: "<<(int)value;
            break;
        }
//		DLOG(INFO)<<"image value at current point("<<pt.x<<','<<pt.y<<"):\n"<<(int)image.at<cv::Vec3b>(pt.x, pt.y)[2]<<std::endl;//for debug
	}
//    DLOG(INFO)<<minxx<<'\t'<<maxxx<<'\t'<<minyy<<'\t'<<maxyy;//for debug
	mapin.close();
    return m_image;
}

void MapEngine::detectTurningPoints() {
	DLOG(INFO)<<"Detecting turning points...\n";//for debug
	//firstly, clear (possibly) former detected results
	this->carTrackTurningPoints.clear();
	this->suspectedTurningPoints.clear();
	//set new eulrChangeThreshold and checkInterval
	this->checkInterval = checkInterval;
	this->eulrChangeThreshold = eulrChangeThreshold;
	for(int i = 0; i < carTrack.size()-checkInterval; ++i){
		//if the point is near intersection cross (range < 6 meter) , then
		//this point must be TURNING POINT!
		if(isPointNearby(carTrack[i], INTERSECTION, 30)
				){
			this->carTrackTurningPoints.insert(carTrack[i]);
		}else{
			float nextEulrChange = fabs(carTrack[i].eulr - carTrack[i+1].eulr);
			float farEulrChange = fabs(carTrack[i].eulr - carTrack[i+4].eulr);
			//Strategies to detect turning point:
			if(nextEulrChange > eulrChangeThreshold && farEulrChange > 2*eulrChangeThreshold){
				//if there are intersections or zebra crosses nearby, then
				//this point must be TURNING POINT!
//				if(isCarposeNearIntersectionOrZebra(carTrack[i])){
					this->carTrackTurningPoints.insert(carTrack[i]);
//				}else{
					//else, this turning point is suspected, to be further
					//processed in near future
//					this->suspectedTurningPoints.insert(carTrack[i]);
//				}
			}
		}
	}
//	DLOG(INFO)<<"carTrackTurningPoints size: "<<carTrackTurningPoints.size()<<std::endl;//for debug
}

void MapEngine::divideCarTrackIntoSegments() {
	DLOG(INFO)<<"dividing car tracks into segments...\n";//for debug
	//firstly, clear (possibly) former detect result
	for(auto seg: segments){
		delete seg;
	}
	segments.clear();
	//initially, put one segment into segments.
	segments.push_back(new Segment());
	for(int i = 0; i < carTrack.size()-checkInterval; ++i){
		//if current point is no a turning point
		if(!this->carTrackTurningPoints.count(carTrack[i])
				&& !this->suspectedTurningPoints.count(carTrack[i])){
			segments.back()->carTrack.push_back(carTrack[i]);
		}else{
			//if encountered turning point
			//only segment length greater than 20 do we admit it as a real one
			if(segments.back()->carTrack.size() > 20){
				//this means one segement finished, we could calculate its direction
				segments.back()->setCarDirection();
				segments.push_back(new Segment());
			}else{
				//if number of points in this segment is less than 20, then discard it
				segments.back()->carTrack.clear();
			}
		}
	}
}

void MapEngine::test(const float dottedLaneLineThreshold, const float solidLaneLineThreshold,
		const float curbThreshold) {
	this->detectTurningPoints();
	this->divideCarTrackIntoSegments();
	this->setSegmentScanBaseline();
	occurRateThreshold[DOTTEDLANELINE]=dottedLaneLineThreshold;
	occurRateThreshold[SOLIDLANELINE]=solidLaneLineThreshold;
	occurRateThreshold[CURB]=curbThreshold;
	DLOG(INFO)<<"before entering scanSegment()...\n";//for debug
	this->scanSegment();
	DLOG(INFO)<<"scan segment finished...\n";//for debug
	this->drawSegments();
	cv::imwrite("lanelinedetect.png", image);
}

void MapEngine::drawSegments() {
	int nn=0;
	for(auto seg : segments){
		cv::Scalar color = getRandColor();
		for(auto pt: seg->carTrack){
			drawCircle(pt,color);
//			cvSet2D(canvas, pt.y, pt.x, color);
		}
		color = getRandColor();
		for(auto pt: seg->leftSegmentTerminalPoints){
			drawCircle(pt,color);
//			cvSet2D(canvas, pt.y, pt.x, color);
		}
		color = getRandColor();
		for(auto pt: seg->rightSegmentTerminalPoints){
			drawCircle(pt,color);
//			cvSet2D(canvas, pt.y, pt.x, color);
		}
	}
}

void MapEngine::terminalPoints(const cv::Point& startPoint,
		const cv::Vec4f& direction, std::vector<cv::Point>& leftContainer,std::vector<cv::Point>& rightContainer) {
	int xx=startPoint.x;
	//if direction is horizontal, then its perpendicular line is vertical,
	//the slope is positive infinite. So cann't use normal bresenham algorithm
	if(direction[1]==0){
		//search should not exceed 50 meters (250 points)
        for(int yy=startPoint.y; yy < startPoint.y+250 && yy < m_image->height(); ++yy){
			cv::Point pt(xx,yy);
			//if direction[0]>0, then increasing y means search left side
			if (direction[0]>0){
				leftContainer.push_back(pt);
			}else{
				rightContainer.push_back(pt);
			}
			if(this->isPointNearBoundary(pt)){
				break;
			}
		}
		for(int yy=startPoint.y-1; yy > startPoint.y-250 && yy >= 0; --yy){
			cv::Point pt(xx,yy);
			//if direction[0]<0, then decreasing y means search left side
			if (direction[0]<0){
				leftContainer.push_back(pt);
			}else{
				rightContainer.push_back(pt);
			}
			if(this->isPointNearBoundary(pt)){
				break;
			}
		}
	}else{
		//using normal bresenham algorithm
		double error = 0;
		double deltaError = fabs(double(direction[0])/double(direction[1]));
		int sign=1;
		//if car direction is 0~90 or 180~270, then the perpendicular direction would be 90~180 or 270~360
		//so the sign should change to '-1':
		if (direction[0]*direction[1]>0) {sign = -1;}
		int yy = startPoint.y;
        for(xx=startPoint.x; xx < startPoint.x+250 && xx < m_image->width(); ++xx ){
			cv::Point pt(xx,yy);
			if(this->isPointNearBoundary(pt)){
				break;
			}
			//if direction[1]<0, then increasing x means search left side
			if (direction[1]<0){
				leftContainer.push_back(pt);
			}else{
				rightContainer.push_back(pt);
			}
			error+=deltaError;
			while(error>=0.5){
				yy+=sign;
				cv::Point pt(xx,yy);
				if(this->isPointNearBoundary(pt)){
					break;
				}
				//if direction[1]<0, then increasing x means search left side
				if (direction[1]<0){
					leftContainer.push_back(pt);
				}else{
					rightContainer.push_back(pt);
				}
				error-=1.0;
			}
		}
		yy = startPoint.y;
		for(xx=startPoint.x-1; xx > startPoint.x-250 && xx >= 0; --xx ){
			cv::Point pt(xx,yy);
			if(this->isPointNearBoundary(pt)){
				break;
			}
			//if direction[1]>0, then decreasing x means search left side
			if (direction[1]>0){
				leftContainer.push_back(pt);
			}else{
				rightContainer.push_back(pt);
			}
			error+=deltaError;
			while(error>=0.5){
				yy-=sign;
				cv::Point pt(xx,yy);
				if(this->isPointNearBoundary(pt)){
					break;
				}
				//if direction[1]>0, then decreasing x means search left side
				if (direction[1]<0){
					leftContainer.push_back(pt);
				}else{
					rightContainer.push_back(pt);
				}
				error-=1.0;
			}
		}
	}
}

void MapEngine::bresenhamPoints(const cv::Point& point1,
		const cv::Point& point2, std::vector<cv::Point>& container) {

	const cv::Point& leftPoint = (point1-point2).x<0?point1:point2;
	const cv::Point& rightPoint = (point1-point2).x<0?point2:point1;
	//if direction is vertical, the slope is positive infinite. So cann't use normal bresenham algorithm
	if(point1.x==point2.x){
		//x is fixed, we only need to increment y
		for(int yy=point1.y<point2.y?point1.y:point2.y; yy <= abs(point1.y-point2.y); ++yy){
			cv::Point pt(point1.x,yy);
			container.push_back(pt);
		}
	}else{
		//using normal bresenham algorithm
		double error = 0;
		double deltaError = fabs(double(leftPoint.y-rightPoint.y)/(leftPoint.x-rightPoint.x));
		int sign=1;
		//if slope is going down with x increase.
		if (rightPoint.y<leftPoint.y) {sign = -1;}
		int yy = leftPoint.y;
		for(int xx=leftPoint.x; xx <= rightPoint.x; ++xx ){
			container.push_back(cv::Point(xx,yy));
			error+=deltaError;
			while(error>=0.5){
				yy+=sign;
				container.push_back(cv::Point(xx,yy));
				error-=1.0;
			}
		}
	}
}

cv::Vec4f MapEngine::midLine(const cv::Vec4f& line, const cv::Point& pt) {
	cv::Vec4f perpendicularLine(line[1],-line[0],pt.x,pt.y);
	cv::Point intersect = getLineIntersectPoint(line, perpendicularLine);
	//the mid line is parallel to 'line', and pass through mid point of 'pt' and 'intersect'.
	return cv::Vec4f(line[0],line[1],(pt.x+intersect.x)/2, (pt.y+intersect.y)/2);
}

void MapEngine::setSegmentScanBaseline() {
	DLOG(INFO)<<"setting segment scan baseline...\n";//for debug
	for(auto seg: segments){
		seg->setCarDirection();
		//****SegmentTerminalPoints will be used as scan baseline.
		this->terminalPoints(seg->carTrack.front(),seg->getCarDirection(),seg->leftSegmentTerminalPoints,seg->rightSegmentTerminalPoints);
	}
}

void MapEngine::scanSegment() {
	int count =0;//for debug
	for(auto seg: segments){
		++count;//for debug
		DLOG(INFO)<<"processing segment("<<count <<" of " << segments.size()<<")...\n";//for debug
		//first, scan left of the car track
		//NOTE: we have to watch out for the occurrence of solid line.
		//since after a solid line has reached, the lanes on the other side have opposite direction.
		bool solidLineReached = false;
		for(int i = 0; i< seg->leftSegmentTerminalPoints.size();++i){
			cv::Vec4f scanLine = seg->getCarDirection();
			//set scanLine's point value to current scanning point
			scanLine[2]=seg->leftSegmentTerminalPoints[i].x;
			scanLine[3]=seg->leftSegmentTerminalPoints[i].y;
			//find the end point by finding the intersect point of scanLine and endLine,
			//the end line passes through the last point of car track, and is perpendicular to scanLine
			cv::Vec4f endLine(scanLine[1],-scanLine[0],seg->carTrack.back().x,seg->carTrack.back().y);
			cv::Point endPoint = getLineIntersectPoint(scanLine,endLine);
			//get all the points between begin point (on scan baseline) and end point.
			std::vector<cv::Point> points;
			bresenhamPoints(seg->leftSegmentTerminalPoints[i], endPoint, points);
//			this->viewPoints(points);//for debug
			std::map<PointType, int> statistics;
			pointsStatistics(points,statistics);
//			DLOG(INFO)<<"lane line statistics:" <<statistics[LANELINE]<<std::endl;//for debug
			//if curb line found, besides to push it into container, don't check for lane line
			if(float(statistics[CURB])/points.size()>occurRateThreshold[CURB]){
				cv::Vec4f line(seg->carDirection[0],seg->carDirection[1],seg->leftSegmentTerminalPoints[i].x,seg->leftSegmentTerminalPoints[i].y);
				seg->curbLeftScanResult.push_back(line);
				this->drawPoints(points, cv::Scalar(0,0,0));//for debug
				continue;
			}
			if(float(statistics[LANELINE])/points.size()>occurRateThreshold[SOLIDLANELINE]){
				solidLineReached = true;
				cv::Vec4f line(seg->carDirection[0],seg->carDirection[1],seg->leftSegmentTerminalPoints[i].x,seg->leftSegmentTerminalPoints[i].y);
				seg->laneLineLeftScanResult.push_back(line);
				this->drawPoints(points);//for debug
			}else if(float(statistics[LANELINE])/points.size()>occurRateThreshold[DOTTEDLANELINE]){
				if(!solidLineReached){
					cv::Vec4f line(seg->carDirection[0],seg->carDirection[1],seg->leftSegmentTerminalPoints[i].x,seg->leftSegmentTerminalPoints[i].y);
					seg->laneLineLeftScanResult.push_back(line);
					this->drawPoints(points);//for debug
				}else{
					cv::Vec4f line(-seg->carDirection[0],seg->carDirection[1],-seg->leftSegmentTerminalPoints[i].x,seg->leftSegmentTerminalPoints[i].y);
					seg->laneLineLeftScanResult.push_back(line);
					this->drawPoints(points);//for debug
				}
			}
		}
		DLOG(INFO) << "left scan result: "<<seg->laneLineLeftScanResult.size()
				<<'\t'<<seg->curbLeftScanResult.size()<<std::endl;
		//left scan finished, now scan right
		solidLineReached = false;
		for(int i = 0; i< seg->rightSegmentTerminalPoints.size();++i){
			cv::Vec4f scanLine = seg->getCarDirection();
			//set scanLine's point value to current scanning point
			scanLine[2]=seg->rightSegmentTerminalPoints[i].x;
			scanLine[3]=seg->rightSegmentTerminalPoints[i].y;
			//find the end point by finding the intersect point of scanLine and endLine,
			//the end line passes through the last point of car track, and is perpendicular to scanLine
			cv::Vec4f endLine(scanLine[1],-scanLine[0],seg->carTrack.back().x,seg->carTrack.back().y);
			cv::Point endPoint = getLineIntersectPoint(scanLine,endLine);
			//get all the points between begin point (on scan baseline) and end point.
			std::vector<cv::Point> points;
			bresenhamPoints(seg->rightSegmentTerminalPoints[i], endPoint, points);
//			this->viewPoints(points);//for debug
			std::map<PointType, int> statistics;
			pointsStatistics(points,statistics);
			//if curb line found, besides to push it into container, don't check for lane line
			if(float(statistics[CURB])/points.size()>occurRateThreshold[CURB]){
				cv::Vec4f line(seg->carDirection[0],seg->carDirection[1],seg->leftSegmentTerminalPoints[i].x,seg->leftSegmentTerminalPoints[i].y);
				seg->curbLeftScanResult.push_back(line);
				this->drawPoints(points, cv::Scalar(0,0,0));//for debug
				continue;
			}
			if(float(statistics[LANELINE])/points.size()>occurRateThreshold[SOLIDLANELINE]){
				solidLineReached = true;
				cv::Vec4f line(seg->carDirection[0],seg->carDirection[1],seg->leftSegmentTerminalPoints[i].x,seg->leftSegmentTerminalPoints[i].y);
				seg->laneLineLeftScanResult.push_back(line);
				this->drawPoints(points);//for debug
			}else if(float(statistics[LANELINE])/points.size()>occurRateThreshold[DOTTEDLANELINE]){
				if(!solidLineReached){
					cv::Vec4f line(seg->carDirection[0],seg->carDirection[1],seg->leftSegmentTerminalPoints[i].x,seg->leftSegmentTerminalPoints[i].y);
					seg->laneLineLeftScanResult.push_back(line);
					this->drawPoints(points);//for debug
				}else{
					cv::Vec4f line(-seg->carDirection[0],seg->carDirection[1],-seg->leftSegmentTerminalPoints[i].x,seg->leftSegmentTerminalPoints[i].y);
					seg->laneLineLeftScanResult.push_back(line);
					this->drawPoints(points);//for debug
				}
			}
		}
		DLOG(INFO) << "right scan result: "<<seg->laneLineLeftScanResult.size()
				<<'\t'<<seg->curbLeftScanResult.size()<<std::endl;
    }
}

//note: the 'yy' coordinate here is relative to the map's coordinate, the orignin is on bottom-left corner
//while the origin of m_image is on top-left corner.
Point3B MapEngine::get3b(int xx, int yy)
{
    Point3B point;
    int value = qRed(((QRgb*)(m_image->scanLine(m_image->height()-1-yy)))[xx]);
    switch (value) {
    case LANELINE:
    case 1://for old file capability
        point.base |= (ROADEDGE_CLEAR | LANELINE_DOTTED);
        break;
    case ZEBRA:
    case 2://for old file capability
        point.base |= ROADEDGE_CLEAR;
        break;
    case INTERSECTION:
        point.base |= ROADEDGE_CLEAR;
        point.road |= REGION_INTERSECTION;
        break;
    case CURB:
        point.base |= ROADEDGE_OCCUPIED;
        point.road |= CURB_YES;
        break;
    case TREE:
    case 5://for old file capability
        point.base |= ROADEDGE_OCCUPIED;
        break;
    case TRUNK:
        point.base |= ROADEDGE_OCCUPIED;
        break;
    case PIT:
    case 7://for old file capability
        //NOTE: Pits should be treated as OCCUPIED, but I'm not sure for the moment
//        point.base |= ROADEDGE_OCCUPIED;
        break;
    case LANECENTER:
        point.base |= ROADEDGE_CLEAR;
        break;
    case TRAFFICSIGN:
        point.base |= ROADEDGE_OCCUPIED;
        //type of traffic signs will be added in the future
        break;
    case BLANK:
        point.base |= ROADEDGE_UNKNOWN;
        break;
    case CLEAR:
        point.base |= ROADEDGE_CLEAR;
        break;
    default:
        point.base |= ROADEDGE_UNKNOWN;
        break;
    }
//    if(point.base&ROADEDGE_CLEAR == ROADEDGE_CLEAR){
//        DLOG(INFO)<<"CLEAR POINT: ("<<xx<<", "<<yy<<")";
//    }
    return point;
}

bool MapEngine::write3b(const std::string &fileName)
{
    std::ofstream fileout(fileName,std::ios::binary);
//    DLOG(INFO)<<"prepare writing .3b file...\nimage size:("<<m_image->width()<<','<<m_image->height()<<")";
    for(int yy = 0; yy < m_image->height(); ++yy){
        for(int xx = 0; xx < m_image->width(); ++xx ){
            Point3B pt = get3b(xx,yy);
//            DLOG(INFO)<<"Point3B: "<<(int)pt.base<<'\t'<<(int)pt.road<<'\t'<<(int)pt.sig << std::endl;
//            fileout.write((char *)&pt, sizeof(Point3B));
            ((QRgb*)(m_image->scanLine(m_image->height()-1-yy)))[xx]=qRgb(pt.sig,pt.road,pt.base);
            fileout << pt.base<<pt.road<<pt.sig;
        }
    }
    fileout.close();
    return true;
}

bool MapEngine::toPng(const std::string &fileName)
{
    if(m_image){
        m_image->save(fileName.c_str(),"PNG");
        return true;
    }
    return false;
}

QImage* MapEngine::load3b(const std::string &fileName, int xDimension, int yDimension)
{
    if(m_image)
        delete m_image;
    m_image = new QImage(QSize(xDimension, yDimension),QImage::Format_RGB32);

    std::ifstream map3b(fileName, std::ios::binary);
    Point3B pt;
    for(int x = 0; x < xDimension; ++x){
        for(int y = 0; y < yDimension; ++y){
            map3b.read((char*) &pt, sizeof(pt));
//            DLOG(INFO)<<(int)pt.base;
//            if((pt.base&ROADEDGE_CLEAR) == ROADEDGE_CLEAR){
//                ((QRgb*)(m_image->scanLine(yDimension-1-y)))[x]=COLORS[CLEAR];
//            } else
//            if((pt.base&ROADEDGE_UNKNOWN) == ROADEDGE_UNKNOWN){
//                ((QRgb*)(m_image->scanLine(yDimension-1-y)))[x]=COLORS[BLANK];
//            } else
//            if((pt.base&ROADEDGE_OCCUPIED) == ROADEDGE_OCCUPIED){
//                ((QRgb*)(m_image->scanLine(yDimension-1-y)))[x]=COLORS[OCCUPIED];
//            }
            ((QRgb*)(m_image->scanLine(yDimension-1-y)))[x]=qRgb(pt.sig,pt.road,pt.base);
        }
    }
    map3b.close();
    return m_image;
}

cv::Point MapEngine::getLineIntersectPoint(const cv::Vec4f& line1,
		const cv::Vec4f& line2) {
	//note: this line is tedious, but verified correct!(didn't deal with condition when line1[0]==0, to be added)
	//It's just a formula deducted from mathematics (intersect of two lines, both given 'point-slope' formula).
	int xx=(line2[0]*line1[0]*(line2[3]-line1[3])+line2[0]*line1[1]*line1[2]
				-line1[0]*line2[1]*line2[2])/(line2[0]*line1[1]-line1[0]*line2[1]);
	int yy=(line1[1]*(xx-line1[2])/line1[0])+line1[3];
	return cv::Point(xx,yy);
}

void MapEngine::bresenhamPoints(const cv::Point& point1, const cv::Point& point2,
		cv::Vec4f line, std::vector<cv::Point>& container) {
	cv::Vec4f beginLine(line[1],-line[0],point1.x,point1.y);
	cv::Vec4f endLine(line[1],-line[0],point2.x,point2.y);
	auto beginPoint = this->getLineIntersectPoint(line,beginLine);
	auto endPoint = this->getLineIntersectPoint(line,endLine);
	this->bresenhamPoints(beginPoint,endPoint,container);
}

void MapEngine::pointsStatistics(const std::vector<cv::Point>& points,
		std::map<PointType, int>& statistics) {
	for(auto pt:points){
		if(pt.x>=0&&pt.y>=0&&pt.x<=image.cols&&pt.y<=image.rows){
			if(this->isPointNearby(pt, LANELINE,3)){
				++statistics[LANELINE];
			}
			if(this->isPointNearby(pt, CURB,3)){
				++statistics[CURB];
			}
		}
	}
}

void MapEngine::removeOutlier() {
	DLOG(INFO)<<"removing outliers...\n";
	cv::Mat tmp = image.clone();
	cv::imwrite("before remove outlier.png", image);//for debug
	for(int i = 0; i < image.cols; ++i){
		for(int j = 0; j < image.rows; ++j){
			cv::Point pt(i,j);
			switch (getPointType(i,j)){
			case CURB:
				if(this->howManyPointsNearby(pt,CURB,2)<=2 ){
					tmp.at<cv::Vec3b>(pt.y, pt.x) = COLORS[BLANK];
				}
				break;
			case INTERSECTION:
				if(this->howManyPointsNearby(pt,INTERSECTION,2)<=2 ){
					tmp.at<cv::Vec3b>(pt.y, pt.x) = COLORS[BLANK];
				}
				break;
			}
		}
	}
	image = tmp;
	cv::imwrite("after remove outlier.png", image);//for debug
}

bool MapEngine::loadPng(const std::string& gridMapFilename) {
    if(m_image)
        delete m_image;
    m_image = new QImage(gridMapFilename.c_str());
    return true;
}

bool MapEngine::loadCarposes(const std::string& carposesFilename) {
    carTrack.clear();
	std::ifstream carPosesFile(carposesFilename);
	std::string line;
	std::stringstream ss;
	carpos curcar;
	while (getline(carPosesFile, line)) {
		ss.clear();
		ss << line;
		ss >> curcar.y >> curcar.x >> curcar.eulr;
		carTrack.push_back(curcar);
	}
	carPosesFile.close();
    return true;
}

void MapEngine::drawPoints(const std::vector<cv::Point>& points, const cv::Scalar& color) {
	for(auto pt: points){
		drawCircle(pt,color);
	}
}
