#ifndef DEF_H
#define DEF_H

#include <QPointF>

enum EditorType{
      Hand
    , Editor
    , DrawSegment
    , DrawIntersection
    , DrawLane
    , DrawLaneLine
    , AddPoint
    , AddConnection
    , SelectRoute
    , CurveAdjustor
    , SpeedAdjustor
};

// Enable the use of qgraphicsitem_cast with items.
enum {
      t_CharPoint = 1
    , t_Lane = 2
    , t_Segment = 3
    , t_ControlPoint = 4
    , t_Connection = 5
};

//CharPoint can be used in many different places, we need this enum to indicate it's type
enum PointType{
      StandAlone
    , SegmentVertex
    , IntersectionVertex
    , SharedVertex
    , LaneCharPoint
    , LaneControlPoint
    , LaneLinePoint
    , ConnectionControlPoint //note: there's another "ConnectionControlPoints" in enum 'RndfElement'
};

enum Layer{
      GridmapLayer = -1
    , SegmentLayer = 0
    , LaneLayer = 50
    , ControlPointLayer = 100
    , CharPointLayer = 150
};

enum RndfElement{
      RndfName
    , NumSegments
    , NumZones
    , FormatVersion
    , CreationDate
    , Geometry
    , GridmapName
    , SegmentId
    , SegmentName
    , Vertices
    , NumLanes
    , LaneId
    , NumWaypoints
    , Waypoint
    , EndVertices
    , EndWaypoint
    , EndLane
    , LaneLineId
    , EndLaneLine
    , EndSegment
    , EndFile
    , Connections
    , ConnectionControlPoints
    , EndConnectionControlPoints
    , ControlPoints
    , EndControlPoints
    , IntersectionId
    , EndIntersection
};

class CharPoint;
bool amid(const CharPoint *current, const CharPoint *prev, const CharPoint *next);
template< class containerT, class T >
inline bool present(containerT c, T v){
    return std::find(c.begin(), c.end(), v) != c.end();
}

#endif // DEF_H

