#ifndef CONTROLPOINT_H
#define CONTROLPOINT_H
#include "def.h"
#include <QGraphicsItem>

class ControlPoint: public QGraphicsItem
{
public:
    enum { Type = UserType + t_ControlPoint };

       int type() const
       {
           // Enable the use of qgraphicsitem_cast with this item.
           return Type;
       }

    ControlPoint(QGraphicsItem * parent = 0);
    virtual ~ControlPoint ();

    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

signals:

public slots:
};

#endif // CONTROLPOINT_H
