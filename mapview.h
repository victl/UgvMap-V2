#ifndef MAPVIEW_H
#define MAPVIEW_H

#include "def.h"
#include <QGraphicsView>

class MapView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit MapView(QWidget *parent = 0);

protected /*functions*/:
    void wheelEvent(QWheelEvent *event);

signals:

public slots:

private /*functions*/:
     void drawBackground(QPainter *p, const QRectF &);

private /*variables*/:
     double scaleRatio;     
};

#endif // MAPVIEW_H
