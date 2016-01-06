#include "mapview.h"

#include <QWheelEvent>
#include <qmath.h>

MapView::MapView(QWidget *parent) : QGraphicsView(parent)
  , scaleRatio(1)
{
    setDragMode(QGraphicsView::ScrollHandDrag);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);//if not Full..., the background will become ugly after scrolling

    // Prepare background check-board pattern
    QPixmap tilePixmap(64, 64);
    tilePixmap.fill(Qt::white);
    QPainter tilePainter(&tilePixmap);
    QColor color(220, 220, 220);
    tilePainter.fillRect(0, 0, 32, 32, color);
    tilePainter.fillRect(32, 32, 32, 32, color);
    tilePainter.end();

    setBackgroundBrush(tilePixmap);
}

void MapView::wheelEvent(QWheelEvent *event)
{
    qreal factor = qPow(1.2, event->delta() / 240.0);
    scaleRatio *= factor;
    scale(factor, factor);
    event->accept();
}

void MapView::drawBackground(QPainter *p, const QRectF &/*rect*/)
{
    p->save();
    p->resetTransform();
    p->drawTiledPixmap(viewport()->rect(), backgroundBrush().texture());
    p->restore();
}
