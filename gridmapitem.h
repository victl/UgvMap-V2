#ifndef GRIDMAPITEM_H
#define GRIDMAPITEM_H
#include <QGraphicsPixmapItem>
#include <QGraphicsView>
#include <QImage>
#include <QVector>
#include <QPointF>
#include <QPainter>
#include <queue>
#include <QPixmap>
#include <map>
#include "mapview.h"
#include "Point.h"
#include "mainwindow.h"
#include <QGraphicsSceneMouseEvent>

extern int cursorDiameter;
extern EditorType m_editorType;

class GridMapItem : public QGraphicsItem
{
public:
//public methods:
    GridMapItem(QImage *image, QGraphicsItem * parent = 0);
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;
    void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget) Q_DECL_OVERRIDE;
    void drawOnMap(QGraphicsSceneMouseEvent *event);
//public data members:
    //the following 3 data members are used for drawing and repaint. (inside paint())
    std::queue<QPointF> stuff;
    std::queue<QRectF> updateRect;
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;

private:
    inline void commitChange(const QRectF rect);
    QImage* m_image;
    QImage  m_screenImage;
    //define which set of types to keep when different eraser is used.
    std::map<int, std::set<PointType> > keepTable;
};

#endif // GRIDMAPITEM_H
