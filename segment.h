
#ifndef SEGMENT_H
#define SEGMENT_H
#include "lane.h"
#include "def.h"
#include "mapobject.h"

#include <opencv2/opencv.hpp>
#include <QVector>
#include <QPointF>
#include <QMouseEvent>
#include <QCoreApplication>

/**
  * class Segment
  *
  */

class QGraphicsSceneMouseEvent;
class QPlainTextEdit;

class Segment: public QGraphicsPathItem, public MapObject
{
    Q_DECLARE_TR_FUNCTIONS(Segment)
public /*functions*/:

    enum { Type = UserType + t_Segment };

       int type() const
       {
           // Enable the use of qgraphicsitem_cast with this item.
           return Type;
       }

    Segment (QGraphicsPathItem *parent = 0);
    Segment(unsigned int id, QGraphicsPathItem *parent = 0);

    /**
   * Empty Destructor
   */
    ~Segment ();

    void updatePath();

    /*reimpliment*/bool contains(const QPointF & point) const;
    void addVertex(CharPoint * c = 0, const QPointF& pos = QPointF(0,0), PointType t = SegmentVertex);
    void addLaneLinePoint(PointType t, const QPointF& pos);
    void addLanePoint(PointType t, const QPointF& pos);
    void finishLane();
    void finishLaneLine();
    void addIntersect(Segment *s);
    void removeIntersect(Segment *s);
    void intAddSeg(Segment *s);
    void removeItem(CharPoint * c);
    void removeItem(Lane* l);
    void removeFromParent();
    void updateIndices(unsigned int verIndx = 0, unsigned int laneIndx = 0, unsigned int laneLineIndx = 0);
    unsigned int reArrangeIds();
    QPainterPath getNormalPath() const;
    inline QPlainTextEdit * getLogger();

protected /*overrided functions*/:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

public /*variables*/:
    //because Segment's construction procedure currently require user to click at least 3 points
    //so we need a public flag to indicate this.
    bool isFullConstructed;

    //Here, I used QVector instead of std::vector, because QPolygonF could be constructed very easily
    //by a QVector<QPointF>. The interface is very similar between the two vectors, you could replace
    //it with std::vector any time you like. (if you don't need Qt libraries)
   std::vector<CharPoint *> vertices;

   std::vector<Lane *> lanes;

   //lane lines are treated internally as type 'Lane', because they all have a line and two end points
   std::vector<Lane *> laneLines;

   unsigned int laneLineIndex;
   unsigned int laneIndex;
   unsigned int vertexIndex;

private /*variables*/:
public:
   std::set<Segment*> assocIntersections;
   std::set<Segment*> assocSegments;
private /*functions*/:
};

#endif // SEGMENT_H
