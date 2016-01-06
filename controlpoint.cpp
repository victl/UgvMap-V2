#include "controlpoint.h"

#include <QPainter>
#include <QPen>
#include <QStyleOptionGraphicsItem>

ControlPoint::ControlPoint(QGraphicsItem *parent)
    : QGraphicsItem(parent)
{
}

ControlPoint::~ControlPoint()
{

}

QRectF ControlPoint::boundingRect() const
{
    return QRectF(-2, -2, 4, 4 );
}

QPainterPath ControlPoint::shape() const
{
    QPainterPath path;
    path.addEllipse(QPointF(0,0), 2, 2); // circle
    return path;
}

void ControlPoint::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget */*widget*/)
{
    QPen pen(Qt::lightGray);
    if (option->state & QStyle::State_Selected) {
        pen.setWidth(pen.width() * 1.5);
    }
    painter->setPen(pen);
    painter->drawEllipse(QPointF(0,0), 2, 2);
    painter->setPen(Qt::lightGray);
    painter->drawLine(QPointF(0,0), mapFromScene(parentItem()->scenePos()));
}

