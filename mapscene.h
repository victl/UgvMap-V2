#ifndef MAPSCENE_H
#define MAPSCENE_H
#include "def.h"

#include <QGraphicsScene>
#include <vector>
#include <utility>
#include <list>
#include <QDebug>
#include <QPlainTextEdit>
#include <HdlEngine.h>
#include <defines.h>

class QGraphicsSceneMouseEvent;
class Segment;
class CharPoint;
class Connection;
class QKeyEvent;

typedef struct MdfPoint_t{
    int id;
    double first;
    double second;
    short flag1;
    short flag2;
}MdfPoint;

class MapScene : public QGraphicsScene
{
    Q_OBJECT
public:
    MapScene(QObject * parent = 0);
    MapScene(const QRectF & sceneRect, QObject * parent = 0);
    MapScene(qreal x, qreal y, qreal width, qreal height, QObject * parent = 0);

    virtual ~MapScene(){}
    void setGeometry(double x, double y, int w, int h){
        xCenter = x; yCenter = y; width = w; height = h;
        setSceneRect(0, 0, width, height);
    }
    QPointF mapToLocal(const QPointF& p){
        return QPointF((p.x() - xCenter) * 5 + (double)width / 2, (yCenter - p.y()) * 5 + (double)height / 2);
    }
    QPointF mapToLocal(double x, double y){return mapToLocal(QPointF(x, y));}
    QPointF mapToGlobal(const QPointF& p){
        return QPointF((p.x() - (double)width / 2) / 5 + xCenter, ((double)height / 2 - p.y()) / 5 + yCenter);
    }
    QPointF mapToGlobal(double x, double y){return mapToGlobal(QPointF(x, y));}

    void updateCursor();
    void addSegmentVertex(const QPointF& pos);
    void pushConn(Connection* c){
        conns.push_back(c);
    }
    void addRoutePoint(CharPoint * p);
    void removeRoutePoint(CharPoint* p);
    void clearRoutePoints();

    std::string getCompleteId(CharPoint* p);
    void removeSeg(Segment* s);
    void removeConn(Connection* conn);

    void finishSegment();
    void finishIntersection();
    void finishLane();
    void finishLaneLine();

    bool exportRndf(const QString& fileName);
    bool exportRouteTrack(const QString& fileName);
    bool fromRndfx(const QString& fileName);
    bool getMdfPts(const QString& fileName);
    void updateIndices(unsigned int segindx = 0, unsigned int intIndx = 0, unsigned int zoneIndx = 0, unsigned int connIndx = 0);
    void reArrangeIds();
    void clearAll();
    bool findBestPath();
    bool findPathBetween(CharPoint *startPoint, CharPoint* target);
    std::string getFullId(CharPoint *c);
    void processHdl(victl::HdlEngine* h);
    void writeBoundaryBit(const QString& fileName);

protected /*functions*/:
    void mousePressEvent(QGraphicsSceneMouseEvent * event);
    void keyPressEvent(QKeyEvent * event);

signals:

public slots:
public /*variables*/:
    EditorType editorType;
    Connection* curConn;
private /*variables*/:
    EditorType typeToRetore;
    unsigned int segIndex;
    unsigned int intersectIndex;
public:
    unsigned int connIndex;
private /*functions*/:
    inline unsigned int numBehindDot(std::string v);
    std::string pointInfoEmitor(CharPoint* w);
    CharPoint* findPointById(const std::string& s);
    void AddSinglePoint(const MdfPoint &p);
    bool readConf(const std::string &fileName);
    inline void findMissionPoint(int n = 0);
private:
    unsigned int zoneIndex;
   std::list<Segment *> segments;
   std::list<Segment *> intersections;
   std::list<Segment *> zones;
   std::list<Connection *> conns;
   std::list<CharPoint *> routeKeyPoints;
   double xCenter;
   double yCenter;
   int width;
   int height;
   QPlainTextEdit* logger;
   std::vector<CharPoint *> bestPath;
   std::vector<Connection *> bestPathArrow;
   bool isProcessingHdl;
   bool isProcessingHdlPaused;
   bool saveALocalMap = false;
   QGraphicsPixmapItem *carImgItem;
   QPixmap* probabilityImg;
   QGraphicsPixmapItem * probabilityImgItem;
   std::vector<MdfPoint> mdfPts;
   QPixmap oneTrackPoint;
   QGraphicsItemGroup * trackitems;
   std::vector<std::pair<double, double> > trackpoints;
   std::vector<QGraphicsPixmapItem*> tracks;
   std::vector<CharPoint*> standalonePts;
   unsigned int standaloneIndex;
   double xBias, yBias; // the bias of two coordinate: self & the one from judges
public:// method to support adding intersections in charpoint.cpp
   Segment* curIntersect();
   Segment *addIntersect();
   QPlainTextEdit *getLogger() const;
   void setLogger(QPlainTextEdit *value);
   bool loadCarTrack(const QString &fileName, bool hasId = false);
   template<class T>
   bool addCarTrack(const std::vector<T >& points);
   void clearTrack();
   bool saveKeyPoints(const QString& fileName);
   bool restoreKeyPoints(const QString& fileName, bool createIfNotExist = false);
   int currentMissionPt;
};

#endif // MAPSCENE_H
