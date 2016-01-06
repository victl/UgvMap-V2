#include "lane.h"
#include "mapscene.h"
#include "mapview.h"
#include "controlpoint.h"
#include "segment.h"

#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QPainterPathStroker>
#include <glog/logging.h>
#include <QPlainTextEdit>

// Constructors/Destructors
//  

Lane::Lane (unsigned int id, QGraphicsPathItem *parent)
    : QGraphicsPathItem(parent)
    , isFullConstructed(false)
    , charPointIndex(1)
    , width(3.0)
{
    setId(id);
    setLineColor(Qt::magenta);
    setPen(getPen());
}

Lane::~Lane () { }

QPainterPath Lane::shape() const /*const*/
{
    QPainterPath p = path();
//    QPainterPath p1 = path().toReversed();
//    p.connectPath(p1);
//    p.closeSubpath();
    QPainterPathStroker ps;
    ps.setWidth(getPen().width());
    return ps.createStroke(p);
}

QPlainTextEdit *Lane::getLogger()
{
    return qobject_cast<MapScene*>(scene())->getLogger();
}

void Lane::addKeyPoint(CharPoint *c)
{
    c->setId(charPointIndex++);
    bool hadInserted = false;
    for(unsigned int i = 1; i < keyPoints.size(); ++i){
        if(amid(c, keyPoints[i-1], keyPoints[i])){
            keyPoints.insert(keyPoints.begin() + i, c);
            hadInserted = true;
            break;
        }
    }
    if(!hadInserted)
        keyPoints.push_back(c);
    updatePath();
}

void Lane::removeFromParent()
{
    Segment *seg = qgraphicsitem_cast<Segment*>(parentItem());
    seg->removeItem(this);
}

void Lane::removePoint(CharPoint *c)
{
    auto ic = std::find(keyPoints.begin(), keyPoints.end(), c);
    if(ic != keyPoints.end()){
        keyPoints.erase(ic);
    }
}

void Lane::updatePath()
{
    QPainterPath path = getNormalPath();
    //add an arrow
    if(keyPoints.size() > 1){
        QPointF lastPos = keyPoints.back()->scenePos();
        QPointF semiLastPos = path.pointAtPercent(0.9999);
        QPointF sign = lastPos - semiLastPos;
        QPointF delta1(0, 0) , delta2(0, 0);
        double slope = 0;
        if(keyPoints.back()->outControl->pos() == QPointF(0, 0)){
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

void Lane::updateCharIndex(unsigned int indx)
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

unsigned int Lane::reArrangeIds()
{
    charPointIndex = 1;
    for(auto p: keyPoints){
        p->setId(charPointIndex++);
    }
    return charPointIndex;
}

void Lane::mousePressEvent(QGraphicsSceneMouseEvent *event)
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
            PointType t = (event->modifiers() & Qt::ShiftModifier) ? LaneCharPoint : LaneControlPoint;
            CharPoint * p = new CharPoint(0, t);
            scene()->addItem(p);
            p->setPos(event->scenePos());
            p->setParentItem(this);
            addKeyPoint(p);
            break;
        }
        default:
            break;
        }
}

void Lane::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    setPen(getPen());
    updatePath();
    QGraphicsPathItem::paint(painter, option, widget);
}

QPainterPath Lane::getNormalPath() const
{
    QPainterPath path(keyPoints.front()->scenePos());
    for(unsigned int i = 1; i < keyPoints.size(); ++i){
        path.cubicTo(keyPoints[i-1]->outControl->scenePos(), keyPoints[i]->inControl->scenePos(), keyPoints[i]->pos());
    }
    return path;
}
