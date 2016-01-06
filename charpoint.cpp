#include "charpoint.h"
#include "connection.h"
#include "controlpoint.h"
#include "mapscene.h"
#include "segment.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QStyle>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
// Constructors/Destructors
//  

CharPoint::CharPoint (unsigned int id, PointType t, QGraphicsItem * parent)
    : QGraphicsItem(parent)
    , additionalProperty(0)
    , inControl(new ControlPoint(this))
    , outControl(new ControlPoint(this))
    , _type(t)
    , _radius(2)
    , delta(QPointF(0,0))
    , prop1(0)
    , prop2(0)
    , speed(0)
    , trackFlag(0)
    , isRoutePoint(false)
    , colorWithSpeed({
            {0, QColor(0,255,0)}
          , {1, QColor(89, 183,255)}
          , {2, QColor(0,0,255)}
          , {3, QColor(147, 7, 255)}
          , {4, QColor(255, 96, 0)}
          })
{
    setId(id)->setLineWidth(2);
    switch (t) {
    case SegmentVertex:
        setLineColor(Qt::darkGreen);
        break;
    case LaneCharPoint:
        setLineColor(Qt::darkMagenta);
        break;
    case ConnectionControlPoint:
    case LaneControlPoint:
        setLineColor(Qt::darkYellow);
        break;
    default:
        break;
    }
    setFlags(ItemIsSelectable);
}

CharPoint::~CharPoint () {
    for(auto c : conns){
        delete c;
    }
}

QRectF CharPoint::boundingRect() const
{
    return QRectF(-_radius, -_radius, 2 * _radius, 2* _radius);
}

QPainterPath CharPoint::shape() const
{
    QPainterPath path;
    path.addEllipse(QPointF(0,0), _radius, _radius); // circle
    return path;
}

void CharPoint::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget */*widget*/)
{
    switch (_type) {
    case StandAlone:
        setLineColor(Qt::red);
        _radius = 10;
        break;
    case SegmentVertex:
        setLineColor(Qt::darkGreen);
        break;
    case LaneCharPoint:
        if(prop1 == 0 || getSpeed() == 0)
            setLineColor(Qt::red);
        else
            setLineColor(colorWithSpeed[int(getSpeed()) / 10]);
        break;
    case ConnectionControlPoint:
    case LaneControlPoint:
        setLineColor(Qt::darkYellow);
        break;
    case SharedVertex:
        setLineColor(Qt::green);
        break;
    default:
        break;
    }
    QPen pen = getPen();
    if (option->state & QStyle::State_Selected) {
//        pen.setStyle(Qt::DotLine);
        pen.setWidth(pen.width() * 1.5);
        inControl->setVisible(true);
        outControl->setVisible(true);
    }else{
        inControl->setVisible(false);
        outControl->setVisible(false);
    }
    if(_type == StandAlone){
        pen.setWidth(pen.width() * 3);
    }
    painter->setPen(pen);
    painter->drawEllipse(QPointF(0,0), _radius, _radius);
    if(_type == StandAlone){
        painter->drawText(QPointF(-16,-15), QString::number(id()) + "." + QString::number(getProp1()) + "." + QString::number(getProp2()));
    }
}

void CharPoint::addConn(Connection *c)
{
    conns.insert(c);
}

void CharPoint::removeConn(Connection *c)
{
    auto ic = conns.find(c);
    if(ic != conns.end()){
        conns.erase(ic);
    }
}

void CharPoint::removeFromParent()
{
    switch (_type) {
    case SegmentVertex:
    case IntersectionVertex:
    {
        Segment *seg = qgraphicsitem_cast<Segment*>(parentItem());
        seg->removeItem(this);
    }
        break;
    case LaneCharPoint:
    case LaneControlPoint:
    case LaneLinePoint:
    {
        Lane *lane = qgraphicsitem_cast<Lane*>(parentItem());
        lane->removePoint(this);
        if(isRoutePoint){
            MapScene *s = qobject_cast<MapScene *>(scene());
            s->removeRoutePoint(this);
        }
    }
        break;
    case ConnectionControlPoint:
    {
        Connection *conn = qgraphicsitem_cast<Connection*>(parentItem());
        conn->removePoint(this);
    }
        break;
    default:
        break;
    }
}

PointType CharPoint::getType() const
{
    return _type;
}

CharPoint *CharPoint::setType(const PointType &value)
{
    _type = value;
    return this;
}

void CharPoint::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    MapScene * s = qobject_cast<MapScene *>(scene());
    if(Editor == s->editorType){
        setFlags(ItemIsMovable);
    }else{
        setFlag(ItemIsMovable, false);
    }
    if(AddPoint == s->editorType){
        setFlag(ItemIsSelectable, false);
    }else{
        setFlag(ItemIsSelectable);
    }
    if (event->buttons()==Qt::LeftButton)
        switch (s->editorType) {
        case AddConnection:
        {
           if(0 == s->curConn)
           {
               s->curConn = new Connection(s->connIndex++);
               s->curConn->setSpeed(speed);
               s->curConn->setProp2(prop2);
               s->curConn->from = this;
           }else{
               if(s->curConn->from != this)
               {
                   s->curConn->to = this;
                   s->curConn->from->addConn(s->curConn);
                   s->curConn->to->addConn(s->curConn);
                   scene()->addItem(s->curConn);
                   s->pushConn(s->curConn);
                   s->curConn->updatePath();
                   s->curConn = 0;
               }
           }
            break;
        }
        case DrawIntersection:
        {
            Segment *seg = qgraphicsitem_cast<Segment *>(parentItem());
            if(seg->assocSegments.size())
                break;
            Segment *intersect = s->curIntersect();
            if(!intersect || intersect->isFullConstructed){
                intersect = s->addIntersect();
            }/*else{
                event->accept();
            }*/
            intersect->addVertex(this);
            setType(SharedVertex);
            //connect each other (intersect & seg)
            if(seg){
                intersect->intAddSeg(seg);
                seg->addIntersect(intersect);
            }
            event->accept();
        }
            break;
        case SelectRoute:
            s->addRoutePoint(this);
            break;
        default:
            break;
        }
    QGraphicsItem::mousePressEvent(event);
}

void CharPoint::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    MapScene * s = qobject_cast<MapScene *>(scene());
    if (event->buttons()==Qt::LeftButton){
        switch (s->editorType) {
        case Editor:
        case SelectRoute:
            QGraphicsItem::mouseMoveEvent(event);
            break;
        default:
            delta = event->scenePos() - scenePos();
            outControl->setPos(delta);
            inControl->setPos(-delta);
            break;
        }
    }
}

QVariant CharPoint::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange && scene()) {
        setZValue(CharPointLayer);
        inControl->setZValue(ControlPointLayer);
        outControl->setZValue(ControlPointLayer);
    }
    return QGraphicsItem::itemChange(change, value);
}

bool CharPoint::getIsRoutePoint() const
{
    return isRoutePoint;
}

void CharPoint::setIsRoutePoint(bool value)
{
    isRoutePoint = value;
}

double CharPoint::getSpeed() const
{
    return speed;
}

CharPoint *CharPoint::setSpeed(double value)
{
    speed = value;
    return this;
}

int CharPoint::getTrackFlag() const
{
    return trackFlag;
}

CharPoint *CharPoint::setTrackFlag(int value)
{
    trackFlag = value;
    return this;
}

int CharPoint::getProp2() const
{
    return prop2;
}

CharPoint *CharPoint::setProp2(int value)
{
    prop2 = value;
    return this;
}

int CharPoint::getProp1() const
{
    return prop1;
}

CharPoint *CharPoint::setProp1(int value)
{
    prop1 = value;
    return this;
}
