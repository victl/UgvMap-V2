
#ifndef LANE_H
#define LANE_H

#include "mapobject.h"
#include "charpoint.h"
#include "def.h"

#include <string>
#include <vector>
#include <list>
#include <QGraphicsPathItem>

class QPlainTextEdit;

/**
  * class Lane
  * 
  */

class Lane : public QGraphicsPathItem, public MapObject
{
    Q_DECLARE_TR_FUNCTIONS(CharPoint)
public:

    enum { Type = UserType + t_Lane };

       int type() const
       {
           // Enable the use of qgraphicsitem_cast with this item.
           return Type;
       }

  /**
   * Empty Constructor
   */
  Lane (unsigned int id, QGraphicsPathItem * parent = 0);

  /**
   * Empty Destructor
   */
  virtual ~Lane ();

  QPainterPath shape() const;
  QPlainTextEdit *getLogger();
  void addKeyPoint(CharPoint * c);
  void removeFromParent();
  void removePoint(CharPoint* c);
  void updatePath();
  void updateCharIndex(unsigned int indx = 0);
  unsigned int reArrangeIds();

  bool isFullConstructed;

  //index is only used for charactor points. Start and End points always have 1, 2 as index.
  unsigned int charPointIndex;
  // Key points (including start, end and charactor points. the first two points are start, end points repectively)
  //This structure is a vector of pair. pair.first is index, pair.second is actual key points.
  std::vector<CharPoint *> keyPoints;
  // The width of the lane
  double width;

protected /*functions*/:
  void mousePressEvent(QGraphicsSceneMouseEvent *event);
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private /*variables*/:
private /*functions*/:
    QPainterPath getNormalPath() const;
};

#endif // LANE_H
