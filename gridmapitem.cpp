#include "gridmapitem.h"
#include <glog/logging.h>


GridMapItem::GridMapItem(QImage *image, QGraphicsItem * parent)
    : QGraphicsItem(parent)
    , m_image(image)
    , keepTable({
        {ROADCLEANER, { LANELINE, ZEBRA, INTERSECTION, LANECENTER, CARTRACK, DOTTEDLANELINE, SOLIDLANELINE, CURB, TREE, TRUNK, PIT, TRAFFICSIGN, OCCUPIED }}
      , {FINEERASER, { LANELINE, ZEBRA, INTERSECTION, LANECENTER, CARTRACK, DOTTEDLANELINE, SOLIDLANELINE , PIT, OCCUPIED}}
      , {AGGRESSIVEERASER, { }}
                })
{
    setCacheMode(QGraphicsItem::NoCache);
    m_screenImage = m_image->copy(m_image->rect());
//    m_screenImagePainter = new QPainter(&m_screenImage);
}

QRectF GridMapItem::boundingRect() const
{
    return m_screenImage.rect();
}

QPainterPath GridMapItem::shape() const
{
    QPainterPath path;
    path.addRect(m_screenImage.rect());
    return path;
}

void GridMapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);
    if(!updateRect.size())
        painter->drawImage(0,0,m_screenImage);

//    QPaintDevice *device = painter->device();
//    if(dynamic_cast<QPixmap*>(device) !=NULL) DLOG(INFO)<<"QPixmap";
//    else if (dynamic_cast<QGraphicsPixmapItem*>(device) !=NULL) DLOG(INFO)<<"QGraphicsPixmapItem";
//    else if (dynamic_cast<GridMapItem*>(device) !=NULL) DLOG(INFO)<<"GridMapItem";
//    else DLOG(INFO)<<"Something else";

    if(stuff.size()>=1){
        QPainter m_screenImagePainter(&m_screenImage);
        painter->save();
        QPen pen(QColor(COLORS[(m_editorType==CURBDRAWER || m_editorType==PENDRAWER)?CURB:CLEAR])
                , (cursorDiameter*2/scaleRatio)>=1? (int)(cursorDiameter*2/scaleRatio):1 , Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter->setPen(pen);
        m_screenImagePainter.setPen(pen);
        painter->setBrush(Qt::NoBrush);
        m_screenImagePainter.setBrush(Qt::NoBrush);
        painter->drawPoint(stuff.front());
        m_screenImagePainter.drawPoint(stuff.front());
        while (stuff.size()>1){
            QPainterPath path;
            path.moveTo(stuff.front());
            stuff.pop();
            path.lineTo(stuff.front());
            if(m_editorType == CURBDRAWER){
                stuff.pop();
            }
            painter->drawPath(path);
            m_screenImagePainter.drawPath(path);
            commitChange(path.boundingRect().normalized().adjusted(-cursorDiameter,-cursorDiameter,cursorDiameter,cursorDiameter));
        }
        painter->restore();

    }
    if(updateRect.size()){
        painter->drawImage(updateRect.front(), *m_image, updateRect.front());
        updateRect.pop();
    }
}

void GridMapItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(m_editorType!=HAND){
        drawOnMap(event);
    }else{
        //note: calling parent class method will reult mouseMoveEvent & mouseReleaseEvent propagated to upper level
        //so a "else" clouse is required!
    QGraphicsItem::mousePressEvent(event);
    }
//    QGraphicsView::mousePressEvent(event);
}

void GridMapItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(m_editorType!=HAND&& m_editorType != CURBDRAWER&&event->buttons()==Qt::LeftButton){
        drawOnMap(event);
    }
    QGraphicsItem::mouseMoveEvent(event);
}

void GridMapItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    if(m_editorType!=HAND&& m_editorType != CURBDRAWER && event->button()==Qt::LeftButton){
        while(!stuff.empty())
            stuff.pop();
    }
}

void GridMapItem::commitChange(const QRectF rect)
{
    int left = rect.left(), top = rect.top(), width = rect.right()-rect.left(), height = rect.bottom()-rect.top();
    if(m_editorType == CURBDRAWER || m_editorType == PENDRAWER){
        for(int y = top+height-1; y >=top; --y){
            for(int x = left + width - 1; x >=left; --x){
                if( m_screenImage.rect().contains(x,y) && qRed(((QRgb*)(m_screenImage.scanLine(y)))[x]) == CURB ){
                        ((QRgb*)(m_image->scanLine(y)))[x] = COLORS[CURB];
                }
            }
        }
        updateRect.push(rect);
        return;
    }

    bool isNeedUpdate = false;
//    QImage *p_image = (SvgView *)(parentWidget())->m_image;
    for(int y = top+height-1; y >=top; --y){
        for(int x = left + width - 1; x >=left; --x){
//                    DLOG(INFO)<<"("<<left+x<<","<<top+y<<"):"<<(int)qRed(((QRgb*)(m_image->scanLine(top+y)))[left+x]);
            if( m_screenImage.rect().contains(x,y) && qRed(((QRgb*)(m_screenImage.scanLine(y)))[x]) == CLEAR ){
//                        DLOG(INFO)<<"QImage region size: "<<x<<'\t'<<y<<'\t'<<"region clear";
                if( keepTable[(int)m_editorType].count((PointType)qRed(((QRgb*)(m_image->scanLine(y)))[x])) ){
                    isNeedUpdate = true;
                    ((QRgb*)(m_screenImage.scanLine(y)))[x] = ((QRgb*)(m_image->scanLine(y)))[x];
                }
                else{
                    ((QRgb*)(m_image->scanLine(y)))[x] = COLORS[CLEAR];
                }
            }
        }
    }
    if(isNeedUpdate){
        updateRect.push(rect);
//        update();
    }
}

void GridMapItem::drawOnMap(QGraphicsSceneMouseEvent *event)
{
    QPointF pos = mapToScene(event->pos());
//    switch(m_editorType){
//    case ROADCLEANER:

//        ((MainWindow*)parentWidget())->m_mapEngine->replacePointsInCircle(
//                    pos.x(),
//                    m_pixMapItem->boundingRect().height() - 1 - (int)pos.y(),
//                    m_radius,
//                    std::set<PointType>({BLANK}),
//                    CLEAR
//                    );
//    }
//    repaint();
    stuff.push(pos);
    update();
    //((MainWindow*)parentWidget())->statusBar()->showMessage(tr("image position: (%1,%2). Radius: %3").arg(pos.x()).arg(boundingRect().height() - 1 - (int)pos.y()).arg(m_radius));
}

