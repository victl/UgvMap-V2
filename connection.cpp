#include "connection.h"
#include "charpoint.h"
#include "controlpoint.h"
#include "mapscene.h"

#include <glog/logging.h>
#include <list>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionGraphicsItem>

Connection::Connection(unsigned int id, QGraphicsPathItem *parent)
    : QGraphicsPathItem(parent)
    , from(0)
    , to(0)
    , charPointIndex(1)
    , _speed(0)
    , _prop2(0)
{
    setId(id);
    setLineColor(Qt::green);
//    setPen(getPen());
}

Connection::~Connection()
{
    from->removeConn(this);
    to->removeConn(this);
}

bool Connection::isValid()
{
    return from && to;
}

void Connection::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    setPen(getPen());
    updatePath();
    QGraphicsPathItem::paint(painter, option, widget);
}

void Connection::updatePath()
{
    QPainterPath path = getNormalPath();
    //add an arrow
    if(to){
        QPointF lastPos = to->scenePos();
        QPointF semiLastPos = path.pointAtPercent(0.99);
        QPointF sign = lastPos - semiLastPos;
        QPointF delta1(0, 0) , delta2(0, 0);
        double slope = 0;
        CharPoint* prev = keyPoints.size() ? keyPoints.back() : from;
        if(prev->outControl->pos() == QPointF(0, 0)){
            slope = - sign.y() / sign.x();
        }else{
            slope = - path.slopeAtPercent(1);
        }
        const static double arrowLength = 10;
        delta1.rx() = arrowLength / sqrt(1 + pow(slope, 2));
        delta1.ry() = delta1.rx() * std::fabs(slope);
        delta2.ry() = delta1.rx() / 2;
        delta2.rx() = delta2.ry() * std::fabs(slope);
        if(sign.x() < 0)
            delta1.rx() *= -1;
        if(sign.y() < 0 )
            delta1.ry() *= -1;
        if(sign.x() * sign.y() > 0)
            delta2.rx() *= -1;
        QPointF arrowPoint1(lastPos - delta1+ delta2), arrowPoint2(lastPos - delta1 - delta2);
        path.lineTo(arrowPoint1);
        path.lineTo(lastPos);
        path.lineTo(arrowPoint2);
        path.lineTo(lastPos);
    }
    path.connectPath(path.toReversed());
    setPath(path);
}

void Connection::updateCharIndex(unsigned int indx)
{
    if(indx){
        charPointIndex = indx;
        return;
    }
    for(auto p : keyPoints){
        if(indx <= p->id())
            charPointIndex = p->id() + 1;
    }
}

unsigned int Connection::reArrangeIds()
{
    charPointIndex = 1;
    for(auto p: keyPoints){
        p->setId(charPointIndex++);
    }
    return charPointIndex;
}

void Connection::addKeyPoint(CharPoint *c)
{
    //to simplify processing, a temp std::list is constructed
    std::vector<CharPoint* > tmpvec;
    tmpvec.push_back(from);
    tmpvec.insert(tmpvec.end(), keyPoints.begin(), keyPoints.end());
    tmpvec.push_back(to);
    c->setId(charPointIndex++);
    bool hadInserted = false;
    for(unsigned int i = 1; i < tmpvec.size(); ++i){
        if(amid(c, tmpvec[i-1], tmpvec[i])){
            keyPoints.insert(keyPoints.begin() + i - 1, c);
            hadInserted = true;
            break;
        }
    }
    if(!hadInserted)
        keyPoints.push_back(c);
    updatePath();
}

void Connection::removePoint(CharPoint *c)
{
    auto ic = std::find(keyPoints.begin(), keyPoints.end(), c);
    if(ic != keyPoints.end()){
        keyPoints.erase(ic);
    }
}

void Connection::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    MapScene * s = qobject_cast<MapScene *>(scene());
    if(Editor == s->editorType){
        setFlags(ItemIsSelectable);
    }else{
        setFlag(ItemIsSelectable, false);
    }
    if (event->buttons()==Qt::LeftButton)
        switch (s->editorType) {
        case AddPoint:
        {
            CharPoint * p = new CharPoint(0, ConnectionControlPoint);
            scene()->addItem(p);
            p->setPos(event->scenePos());
            p->setParentItem(this);
            addKeyPoint(p);
            break;
        }
        default:
            break;
        }
    QGraphicsPathItem::mousePressEvent(event);
}

double Connection::getSpeed() const
{
    return _speed;
}

void Connection::setSpeed(double speed)
{
    _speed = speed;
}

double Connection::getProp2() const
{
    return _prop2;
}

void Connection::setProp2(double prop2)
{
    _prop2 = prop2;
}

QPainterPath Connection::getNormalPath() const
{
    QPainterPath path(from->scenePos());
    std::vector<CharPoint* > tmpvec;
    tmpvec.push_back(from);
    tmpvec.insert(tmpvec.end(), keyPoints.begin(), keyPoints.end());
    tmpvec.push_back(to);
    for(unsigned int i = 1; i < tmpvec.size(); ++i){
        path.cubicTo(tmpvec[i-1]->outControl->scenePos(), tmpvec[i]->inControl->scenePos(), tmpvec[i]->pos());
    }
    return path;
}

