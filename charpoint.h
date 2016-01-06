
#ifndef CHARPOINT_H
#define CHARPOINT_H
#include "mapobject.h"
#include "def.h"
#include <string>
#include <vector>
#include <QCoreApplication>
#include <QGraphicsItem>
#include <opencv2/opencv.hpp>
#include <set>

/**
  * class CharPoint
  *
  */

class QPainter;
class Connection;
class ControlPoint;

class CharPoint : public QGraphicsItem, public MapObject
{
    Q_DECLARE_TR_FUNCTIONS(CharPoint)
public:
    enum { Type = UserType + t_CharPoint };

    int type() const
    {
        // Enable the use of qgraphicsitem_cast with this item.
        return Type;
    }

    CharPoint (unsigned int id, PointType t, QGraphicsItem * parent = 0);

    /**
   * Empty Destructor
   */
    virtual ~CharPoint ();

    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void addConn(Connection* c);
    void removeConn(Connection* c);
    void removeFromParent();

    char additionalProperty;

    /**
   * Set the value of additionalProperty
   * @param new_var the new value of additionalProperty
   */
    void setAdditionalProperty (char new_var)   {
        additionalProperty = new_var;
    }

    /**
   * Get the value of additionalProperty
   * @return the value of additionalProperty
   */
    char getAdditionalProperty ()   {
        return additionalProperty;
    }

    int getRadius() const
    {
        return _radius;
    }

    void setRadius(int radius)
    {
        _radius = radius;
    }

    double direction(){
        if(delta.x() != 0)
            return delta.y() / delta.x();
        else
            return 10000;
    }
    void setDirection(double x, double y){
        delta.rx()  =  x;
        delta.ry() = y;
    }

public /*variables*/:
    ControlPoint * inControl;
    ControlPoint * outControl;
    std::set<Connection *> conns;

    int getProp1() const;
    CharPoint* setProp1(int value);

    int getProp2() const;
    CharPoint* setProp2(int value);

    int getTrackFlag() const;
    CharPoint* setTrackFlag(int value);

    double getSpeed() const;
    CharPoint* setSpeed(double value);

    PointType getType() const;
    CharPoint* setType(const PointType &value);

    bool getIsRoutePoint() const;
    void setIsRoutePoint(bool value);

protected /*functions*/:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
    QVariant itemChange(GraphicsItemChange change, const QVariant & value);

private /*variables*/:
    PointType _type;
    int _radius;
    QPointF delta;
    //following variables are properties of CharPoint
    int prop1;
    int prop2;
    double speed;
    int trackFlag;
    bool isRoutePoint;
    std::map<int, QColor> colorWithSpeed;
};

#endif // CHARPOINT_H
