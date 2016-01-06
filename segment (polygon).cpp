#include "segment.h"
#include "def.h"
#include "mapview.h"
#include <QGraphicsScene>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>

// Constructors/Destructors
//  

Segment::Segment (QGraphicsItem *parent)
    : QGraphicsPolygonItem(parent)
    , isFullConsturcted(false)
    , laneIndex(1)
    , _id(0)
{
    setFlags(ItemIsSelectable);
}

Segment::Segment(unsigned int id, QGraphicsItem *parent)
    : QGraphicsPolygonItem(parent)
    , isFullConsturcted(false)
    , laneIndex(1)
    , _id(id)
{
    setFlags(ItemIsSelectable);
}

Segment::~Segment () { }

void Segment::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    scene()->clearSelection();
    MapView * view = qobject_cast<MapView *>(scene()->views().front());
    if (event->buttons()==Qt::LeftButton)
        switch (view->editorType) {
        case DrawLaneLine:
        {
            if(0 == laneLines.size() || laneLines.back()->isFullConstructed){
                Lane *laneline = new Lane(laneIndex++);
                laneline->setParentItem(this);
                laneLines.push_back(laneline);
                CharPoint * p = new CharPoint(1);
                scene()->addItem(p);
                p->setPos(event->scenePos());
                p->setParentItem(laneline);
                laneline->keyPoints.push_back(p);
                laneline->setPath(QPainterPath(event->scenePos()));
            }else{//size() > 0 && not full constructed
                Lane *laneline = laneLines.back();
                CharPoint * p = new CharPoint(2);
                scene()->addItem(p);
                p->setPos(event->scenePos());
                p->setParentItem(laneline);
                laneline->keyPoints.push_back(p);
                laneline->isFullConstructed = true;
                QPainterPath path = laneline->path();
                path.lineTo(event->scenePos());
                laneline->setPath(path);
            }
            break;
        }
        default:
            break;
        }
}

