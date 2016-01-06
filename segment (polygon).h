
#ifndef SEGMENT_H
#define SEGMENT_H
#include "lane.h"

#include <QGraphicsPolygonItem>
#include <opencv2/opencv.hpp>
#include <QVector>
#include <QPointF>
#include <QMouseEvent>

/**
  * class Segment
  *
  */

class QGraphicsPathItem;
class QGraphicsSceneMouseEvent;

class Segment: public cv::Point2d, public QGraphicsPolygonItem
{
public /*functions*/:
    Segment (QGraphicsItem * parent = 0);
    Segment(unsigned int id, QGraphicsItem * parent = 0);

    /**
   * Empty Destructor
   */
    ~Segment ();

protected /*overrided functions*/:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

public /*variables*/:
    //because Segment's construction procedure currently require user to click at least 3 points
    //so we need a public flag to indicate this.
    bool isFullConsturcted;

    //Here, I used QVector instead of std::vector, because QPolygonF could be constructed very easily
    //by a QVector<QPointF>. The interface is very similar between the two vectors, you could replace
    //it with std::vector any time you like. (if you don't need Qt libraries)
   QVector<QPointF> vertexes;

   std::vector<Lane *> lanes;

   //lane lines are treated internally as type 'Lane', because they all have a line and two end points
   std::vector<Lane *> laneLines;

   unsigned int laneIndex;

private /*variables*/:
   unsigned int _id;
};

#endif // SEGMENT_H
