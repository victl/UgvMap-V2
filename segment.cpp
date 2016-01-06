#include "segment.h"
#include "mapscene.h"
#include "controlpoint.h"
#include <QGraphicsScene>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <glog/logging.h>
#include <QPainter>
#include <QPlainTextEdit>

// Constructors/Destructors
//  

Segment::Segment (QGraphicsPathItem *parent)
    : QGraphicsPathItem(parent)
    , isFullConstructed(false)
    , laneLineIndex(1)
    , laneIndex(1)
    , vertexIndex(1)
{
}

Segment::Segment(unsigned int id, QGraphicsPathItem *parent)
    : QGraphicsPathItem(parent)
    , isFullConstructed(false)
    , laneLineIndex(1)
    , laneIndex(1)
    , vertexIndex(1)
{
    setId(id);
    setLineColor(Qt::blue);
    setPen(getPen());
    setFlags(ItemIsSelectable);
}

Segment::~Segment () {
    for(auto p : vertices){
        if(p->getType() == SharedVertex){
            p->setType(SegmentVertex);
        }
    }
    for(auto i : assocIntersections){
        delete i;
    }
    for(auto s : assocSegments){
        s->removeIntersect(this);
    }
}

void Segment::updatePath()
{
    QPainterPath path = getNormalPath();
    path.connectPath(path.toReversed());
    setPath(path);
}

bool Segment::contains(const QPointF &point) const
{
    return getNormalPath().contains(point);
}

void Segment::addVertex(CharPoint *c, const QPointF &pos, PointType t)
{
    if(!c){
        c = new CharPoint(vertexIndex++, t);
        scene()->addItem(c);
        c->setPos(pos);
        c->setParentItem(this);
    }
    bool hadInserted = false;
    for(unsigned int i = 1; i < vertices.size(); ++i){
        if(amid(c, vertices[i-1], vertices[i])){
            vertices.insert(vertices.begin() + i, c);
            hadInserted = true;
            break;
        }
    }
    if(!hadInserted)
        vertices.push_back(c);
    updatePath();
}

void Segment::addLaneLinePoint(PointType t, const QPointF &pos)
{
    Lane *laneline;
    CharPoint * p = new CharPoint(0, t);
    if(0 == laneLines.size() || laneLines.back()->isFullConstructed){
        laneline = new Lane(laneLineIndex++);
        laneline->setParentItem(this);
        laneLines.push_back(laneline);
        getLogger()->appendPlainText(tr(">Drawing lane line %1.%2").arg(id()).arg(laneline->id()));
    }else{//size() > 0 && not full constructed
        laneline = laneLines.back();
    }
    scene()->addItem(p);
    p->setPos(pos);
    p->setParentItem(laneline);
    laneline->addKeyPoint(p);
}

void Segment::addLanePoint(PointType t, const QPointF &pos)
{
    Lane *lane;
    CharPoint * p = new CharPoint(0, t);
    if(0 == lanes.size() || lanes.back()->isFullConstructed){
        lane = new Lane(laneIndex++);
        lane->setParentItem(this);
        lanes.push_back(lane);
        getLogger()->appendPlainText(tr(">Drawing lane %1.%2").arg(id()).arg(lane->id()));
    }else{//size() > 0 && not full constructed
        lane = lanes.back();
    }
    scene()->addItem(p);
    p->setPos(pos);
    p->setParentItem(lane);
    lane->addKeyPoint(p);
}

void Segment::finishLane()
{
    if(0 != lanes.size() && !lanes.back()->isFullConstructed){
        Lane *lane = lanes.back();
        if(lane->keyPoints.size() >1){
            lane->isFullConstructed = true;
            getLogger()->appendPlainText(tr(">Finish lane %1.%2").arg(id()).arg(lane->id()));
        }else{
            getLogger()->appendPlainText(tr(">Cancel lane %1.%2").arg(id()).arg(lane->id()));
            delete lane;
            lanes.pop_back();
        }
    }
}

void Segment::finishLaneLine()
{
    if(0 != laneLines.size() && !laneLines.back()->isFullConstructed){
        Lane *laneLine = laneLines.back();
        if(laneLine->keyPoints.size() >1){
            laneLine->isFullConstructed = true;
            getLogger()->appendPlainText(tr(">Finish lane line %1.%2").arg(id()).arg(laneLine->id()));
        }else{
            getLogger()->appendPlainText(tr(">Cancel lane line %1.%2").arg(id()).arg(laneLine->id()));
            delete laneLine;
            laneLines.pop_back();
        }
    }
}

void Segment::addIntersect(Segment *s)
{
    assocIntersections.insert(s);
}

void Segment::removeIntersect(Segment *s)
{
    auto is = assocIntersections.find(s);
    if(is != assocIntersections.end()){
        assocIntersections.erase(is);
    }
}

void Segment::intAddSeg(Segment *s)
{
    assocSegments.insert(s);
}

void Segment::removeItem(CharPoint *c)
{
    auto ic = std::find(vertices.begin(), vertices.end(), c);
    if(ic != vertices.end()){
        vertices.erase(ic);
    }
}

void Segment::removeItem(Lane *l)
{
    auto ic = std::find(lanes.begin(), lanes.end(), l);
    if(ic != lanes.end()){
        lanes.erase(ic);
    }else{
        ic = std::find(laneLines.begin(), laneLines.end(), l);
        if(ic != laneLines.end()){
            laneLines.erase(ic);
        }
    }
}

void Segment::updateIndices(unsigned int laneLineIndx, unsigned int verIndx, unsigned int laneIndx)
{
    if(verIndx){
        vertexIndex = verIndx;
    }
    for(auto p : vertices){
        if(vertexIndex <= p->id())
            vertexIndex = p->id() + 1;
    }
    if(laneIndx){
        laneIndex = laneIndx;
    }
    for(auto l : lanes){
        if(laneIndex <= l->id())
            laneIndex = l->id() + 1;
        l->updateCharIndex();
    }
    if(laneLineIndx){
        laneLineIndex = laneLineIndx;
    }
    for(auto l : laneLines){
        if(laneLineIndex <= l->id())
            laneLineIndex = l->id() + 1;
        l->updateCharIndex();
    }
}

unsigned int Segment::reArrangeIds()
{
    vertexIndex = 1;
    for(auto p : vertices){
        p->setId(vertexIndex++);
    }
    laneIndex = 1;
    for(auto l : lanes){
        l->setId(laneIndex++);
        l->reArrangeIds();
    }
    laneLineIndex = 1;
    for(auto l : laneLines){
        l->setId(laneLineIndex++);
        l->reArrangeIds();
    }
    return 0;
}

QPlainTextEdit *Segment::getLogger()
{
    return qobject_cast<MapScene*>(scene())->getLogger();
}

void Segment::mousePressEvent(QGraphicsSceneMouseEvent *event)
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
            PointType t = assocSegments.size() ? IntersectionVertex : SegmentVertex;
            addVertex(0, event->scenePos(), t);
            break;
        }
        default:
            break;
        }
    QGraphicsPathItem::mousePressEvent(event);
}

void Segment::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    setPen(getPen());
    updatePath();
    QGraphicsPathItem::paint(painter, option, widget);
}

QPainterPath Segment::getNormalPath() const
{
    QPainterPath path(vertices.front()->scenePos());
    for(unsigned int i = 1; i < vertices.size(); ++i){
        path.cubicTo(vertices[i-1]->outControl->scenePos(), vertices[i]->inControl->scenePos(), vertices[i]->pos());
    }
    if(isFullConstructed)
        path.cubicTo(vertices.back()->outControl->scenePos(), vertices.front()->inControl->scenePos(), vertices.front()->scenePos());
    return path;
}

