
#ifndef WAYPOINT_H
#define WAYPOINT_H
#include <string>
#include <vector>

#include "point.h"



/**
  * class WayPoint
  * 
  */

class WayPoint : public cv::Point2d
{
public:
  /**
   * Empty Constructor
   */
  WayPoint ();

  /**
   * Empty Destructor
   */
  virtual ~WayPoint ();

};

#endif // WAYPOINT_H
