#ifndef CONNECTION_H
#define CONNECTION_H
#include "mapobject.h"
#include "def.h"
#include <QGraphicsPathItem>
#include <QCoreApplication>

class CharPoint;

class Connection : public QGraphicsPathItem, public MapObject
{
    Q_DECLARE_TR_FUNCTIONS(Connection)
public:
    enum { Type = UserType + t_Connection };

       int type() const
       {
           // Enable the use of qgraphicsitem_cast with this item.
           return Type;
       }

    explicit Connection(unsigned int id, QGraphicsPathItem *parent = 0);
    ~Connection();
    bool isValid();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void updatePath();
    void updateCharIndex(unsigned int indx = 0);
    unsigned int reArrangeIds();

    void addKeyPoint(CharPoint* c);
    void removePoint(CharPoint* c);

signals:

public slots:

protected /*functions*/:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

public /*variables*/:
    CharPoint *from;
    CharPoint *to;
    std::vector<CharPoint *> keyPoints;
    unsigned int charPointIndex;
    double getSpeed() const;
    void setSpeed(double speed);

    double getProp2() const;
    void setProp2(double prop2);

private /*variables*/:
    double _speed;
    double _prop2;
private /*functions*/:
    QPainterPath getNormalPath() const;
};

#endif // CONNECTION_H
