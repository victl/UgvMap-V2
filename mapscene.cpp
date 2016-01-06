#include "mapscene.h"
#include "charpoint.h"
#include "segment.h"
#include "connection.h"
#include "controlpoint.h"
#include "mapview.h"
#include "connection.h"
#include "CoordiTran.h"
#include <QPainterPath>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QKeyEvent>
#include <QDate>
#include <fstream>
#include <string>
#include <set>
#include <map>
#include <limits>
#include <cmath>
#include <algorithm>
#include <glog/logging.h>
#include <opencv2/opencv.hpp>
#include <array>
#include <QDebug>
#include <future>
#include <boost/program_options.hpp>
#include <QInputDialog>

MapScene::MapScene(QObject *parent)
    : QGraphicsScene(parent)
    , editorType(Hand)
    , curConn(0)
    , typeToRetore(Hand)
    , segIndex(1)
    , intersectIndex(1)
    , connIndex(1)
    , zoneIndex(1)
    , xCenter(0)
    , yCenter(0)
    , width(0)
    , height(0)
    , logger(0)
    , isProcessingHdl(false)
    , isProcessingHdlPaused(false)
    , carImgItem(0)
    , probabilityImg(0)
    , probabilityImgItem(0)
    , oneTrackPoint(":/images/cartrackpoint.png")
    , trackitems(new QGraphicsItemGroup)
    , standaloneIndex(1)
    , xBias(0)
    , yBias(0)
    , currentMissionPt(0)
{

}

MapScene::MapScene(const QRectF &sceneRect, QObject *parent)
    : QGraphicsScene(sceneRect, parent)
    , editorType(Hand)
    , curConn(0)
    , typeToRetore(Hand)
    , segIndex(1)
    , intersectIndex(1)
    , connIndex(1)
    , zoneIndex(1)
    , xCenter(0)
    , yCenter(0)
    , width(0)
    , height(0)
    , logger(0)
    , isProcessingHdl(false)
    , isProcessingHdlPaused(false)
    , carImgItem(0)
    , probabilityImg(0)
    , probabilityImgItem(0)
    , oneTrackPoint(":/images/cartrackpoint.png")
    , trackitems(new QGraphicsItemGroup)
    , standaloneIndex(1)
    , xBias(0)
    , yBias(0)
    , currentMissionPt(0)
{

}

MapScene::MapScene(qreal x, qreal y, qreal width, qreal height, QObject *parent)
    : QGraphicsScene(x, y, width, height, parent)
    , editorType(Hand)
    , curConn(0)
    , typeToRetore(Hand)
    , segIndex(1)
    , intersectIndex(1)
    , connIndex(1)
    , zoneIndex(1)
    , xCenter(0)
    , yCenter(0)
    , width(0)
    , height(0)
    , logger(0)
    , isProcessingHdl(false)
    , isProcessingHdlPaused(false)
    , carImgItem(0)
    , probabilityImg(0)
    , probabilityImgItem(0)
    , oneTrackPoint(":/images/cartrackpoint.png")
    , trackitems(new QGraphicsItemGroup)
    , standaloneIndex(1)
    , xBias(0)
    , yBias(0)
    , currentMissionPt(0)
{

}

void MapScene::updateCursor()
{
    MapView * v = qobject_cast<MapView *>(views().front());
    switch (editorType) {
    case Editor:
    case DrawSegment:
    case DrawLane:
    case DrawLaneLine:
    case AddPoint:
    case AddConnection:
    case DrawIntersection:
    case SelectRoute:
    case CurveAdjustor:
        v->setDragMode(QGraphicsView::NoDrag);
        v->setCursor(Qt::CrossCursor);
        break;
    case SpeedAdjustor:
        v->setDragMode(QGraphicsView::RubberBandDrag);
        v->setCursor(Qt::CrossCursor);
        break;
    case Hand:
    default:
        v->setDragMode(QGraphicsView::ScrollHandDrag);
        v->unsetCursor();
        break;
    }
}

void MapScene::addSegmentVertex(const QPointF &pos)
{
    if(0 == segments.size() || segments.back()->isFullConstructed){
        Segment * seg = new Segment(segIndex++);
        segments.push_back(seg);
        addItem(seg);
        logger->appendPlainText(tr(">Drawing segment %1").arg(seg->id()));
    }
    segments.back()->addVertex(0, pos, SegmentVertex);
}

void MapScene::addRoutePoint(CharPoint *p)
{
    p->setIsRoutePoint(true);
    routeKeyPoints.push_back(p);
    getLogger()->appendPlainText(tr(">Added a route point %1").arg(QString::fromStdString(getFullId(p))));
}

void MapScene::removeRoutePoint(CharPoint *p)
{
    auto iter = std::find(routeKeyPoints.begin(), routeKeyPoints.end(), p);
    if(iter != routeKeyPoints.end()){
        (*iter)->setIsRoutePoint(false);
        getLogger()->appendPlainText(tr(">Removed a route point %1").arg(QString::fromStdString(getFullId(p))));
        routeKeyPoints.erase(iter);
    }
}

void MapScene::clearRoutePoints()
{
    for(auto p : routeKeyPoints){
        p->setIsRoutePoint(false);
    }
    routeKeyPoints.clear();
    for(auto c : bestPathArrow){
        delete c;
    }
    bestPathArrow.clear();
    bestPath.clear();
    getLogger()->appendPlainText(">Removed all route points");
}

std::string MapScene::getCompleteId(CharPoint *p)
{
    Lane * lane = qgraphicsitem_cast<Lane *>(p->parentItem());
    Segment* seg = qgraphicsitem_cast<Segment *>(lane->parentItem());
    return  std::to_string(seg->id()) + '.' + std::to_string(lane->id()) + '.' + std::to_string(p->id());
}

void MapScene::removeSeg(Segment *s)
{
    auto iter = std::find(segments.begin(), segments.end(), s);
    if(iter != segments.end()){
        segments.erase(iter);
    }else{
        iter = std::find(intersections.begin(), intersections.end(), s);
        if(iter != intersections.end())
            intersections.erase(iter);
    }
}

void MapScene::removeConn(Connection *conn)
{
    auto iter = std::find(conns.begin(), conns.end(), conn);
    if(iter != conns.end()){
        conns.erase(iter);
    }
}

void MapScene::finishSegment()
{
    if(0 != segments.size() && !segments.back()->isFullConstructed){
        Segment *seg = segments.back();
        if(seg->vertices.size() >2){
            seg->isFullConstructed = true;
            seg->updatePath();
            getLogger()->appendPlainText(tr(">Finished segment %1").arg(seg->id()));
        }else{//vertexes num <=2
            getLogger()->appendPlainText(tr(">Cancel segement %1").arg(seg->id()));
            delete seg;
            segments.pop_back();
        }
    }
}

void MapScene::finishIntersection()
{
    if(0 != intersections.size() && !intersections.back()->isFullConstructed){
        Segment *intersect = intersections.back();
        if(intersect->vertices.size() >2){
            intersect->isFullConstructed = true;
            intersect->updatePath();
            getLogger()->appendPlainText(tr(">Finished intersection %1").arg(intersect->id()));
        }else{//vertexes num <=2
            delete intersect;
            intersections.pop_back();
            getLogger()->appendPlainText(tr(">Cancel intersection %1").arg(intersect->id()));
        }
    }
}

void MapScene::finishLane()
{
    for(auto seg : segments){
        seg->finishLane();
    }
}

void MapScene::finishLaneLine()
{
    for(auto seg : segments){
        seg->finishLaneLine();
    }
}

bool MapScene::exportRndf(const QString &fileName)
{
    using namespace std;

    ofstream ofs(fileName.toStdString());
    if(!ofs)
        return false;
    //output header
    ofs << "RNDF_name\t" << fileName.split("/").back().split(".").front().toStdString() << endl
        << "num_segments\t" << segments.size() << endl
        << "num_zones\t" << zones.size() << endl
        << "format_version\t" << "1.1" << endl
        << "creation_date\t" << QDate::currentDate().toString("yyyy-MM-dd").toStdString() << endl;
           //geometry of topology map should not bound with certain map center, so we don't output geometry here any more
//        << "geometry\t" << xCenter << '\t' << yCenter << '\t' << width << '\t' << height << endl;
    //output each segment
    for(auto seg : segments){
        ofs << "segment\t" << seg->id() << endl
            << "segment_name\t" << seg->getName().toStdString() << endl;
        //output each vertex
        ofs << "vertices" << endl;
        for(auto v : seg->vertices){
            ofs << pointInfoEmitor(v) << endl;
        }
        ofs << "end_vertices" << endl;
        ofs << "num_lanes\t" << seg->lanes.size() << endl;
        //output each lane
        for(auto lane : seg->lanes){
            ofs << "lane\t" << seg->id() << '.' << lane->id() << endl
                  << "num_waypoints\t" << lane->keyPoints.size() << endl
                  << "waypoint" << endl;
            //output waypoints
            for(auto w : lane->keyPoints){
                ofs << pointInfoEmitor(w) << endl;
            }
            ofs << "end_waypoint" << endl;
            //end lane
            ofs << "end_lane" << endl;
        }
        //output each laneLine
        for(auto laneline : seg->laneLines){
            ofs << "laneline\t" << seg->id() << '.' << laneline->id() << endl
                  << "num_controlpoints\t" << laneline->keyPoints.size() << endl
                  << "controlpoint" << endl;
            //output controlpoints
            for(auto p : laneline->keyPoints){
                ofs << pointInfoEmitor(p) << endl;
            }
            ofs << "end_controlpoint" << endl;
            //end lane
            ofs << "end_laneline" << endl;
        }
        //end segment
        ofs << "end_segment" << endl;
    }
    //output each zone
    //this part is deferred, will be added in future

    //output connections
    for(auto c : conns){
        Lane * fromLane = qgraphicsitem_cast<Lane *>(c->from->parentItem());
        Lane * toLane = qgraphicsitem_cast<Lane *>(c->to->parentItem());
        Segment* fromSeg = qgraphicsitem_cast<Segment *>(fromLane->parentItem());
        Segment* toSeg = qgraphicsitem_cast<Segment *>(toLane->parentItem());
        ofs << "connection\t" << fromSeg->id() << '.' << fromLane->id() << '.' << c->from->id()
            << '\t' << toSeg->id() << '.' << toLane->id() << '.' << c->to->id() << '\t' << c->getProp2() << '\t' << c->getSpeed() << endl;
        //output keypoints
        if (c->keyPoints.size()){
            ofs << "connection_control_points" << endl;
            for(auto p : c->keyPoints){
                ofs << pointInfoEmitor(p) << endl;
            }
            ofs << "end_connection_control_points" << endl;
        }
    }

    //output intersections
    if(intersections.size()){
        for(auto i : intersections){
            ofs << "intersection\t" << i->id() << endl
                << "intersection_name\t" << i->getName().toStdString() << endl
                << "vertices" << endl;
            //output each vertex
            for(auto v : i->vertices){
                ofs << pointInfoEmitor(v) << endl;
            }
            ofs << "end_vertices" << endl;
        }
        ofs << "end_intersection" << endl;
    }
    ofs << "end_file" << endl;
    ofs.close();
    return true;
}

bool MapScene::exportRouteTrack(const QString &fileName)
{
    std::ofstream ofs(fileName.toStdString());
    if(!ofs)
        return false;
    using namespace std;
    //output route.thu first
    DLOG(INFO) << bestPath.size();
    int i = 1;
    for(auto p : bestPath){
        if(p->getType() == LaneCharPoint ){
            Lane * lane = qgraphicsitem_cast<Lane *>(p->parentItem());
            Segment* seg = qgraphicsitem_cast<Segment *>(lane->parentItem());
            QPointF pos = mapToGlobal(p->scenePos());
            ofs << i++ << '\t' << seg->id() << '\t' << lane->id() << '\t' << p->id() << '\t' << pos.x() << '\t' << pos.y() << '\t'
                   /*continued*/ << p->getProp1() << '\t' << p->getProp2()  << '\t'<< p->getSpeed() << '\t' << p->getTrackFlag() << endl;
        }
    }
    ofs.close();

    /*********************************this ends the route.thu file ****************************************/
    ofs.open(fileName.toStdString().append(".gps.txt"));
    if(!ofs)
        return false;
    QPainterPath route;
    route.moveTo(bestPath.front()->scenePos());
    for(auto curr = bestPath.begin(), next = bestPath.begin() + 1; next != bestPath.end(); ++curr, ++ next){
        route.cubicTo((*curr)->outControl->scenePos(), (*next)->inControl->scenePos(), (*next)->scenePos());
    }
    //add a tail
    QPointF delta = bestPath[bestPath.size() - 1]->scenePos() - bestPath[bestPath.size() - 2]->scenePos();
    delta.rx() *= 10;
    delta.ry() *=  10;
    route.lineTo(bestPath.back()->scenePos() + delta);
    //output first point
    QPointF startPos = mapToGlobal(routeKeyPoints.front()->scenePos());
    ofs << startPos.x() << '\t' << startPos.y() << '\t' << 1 << '\t' << getFullId(routeKeyPoints.front()) << endl;
    //output rest points
    for(auto curr = bestPath.begin(), next = bestPath.begin() + 1; next != bestPath.end(); ++curr, ++ next){
        QPainterPath p((*curr)->scenePos());
        p.cubicTo((*curr)->outControl->scenePos(), (*next)->inControl->scenePos(), (*next)->scenePos());
        double pointsInThisPart = p.length();
        double step = 1 / pointsInThisPart;
        for(double pos = step; pos < 1; pos += step){
            QPointF apoint = mapToGlobal(p.pointAtPercent(pos));
            ofs << apoint.x() << '\t' << apoint.y() << '\t' << 0 << '\t' << "0.0.0" << endl;
        }
        QPointF next_g = mapToGlobal((*next)->scenePos());
        if((*next)->getType() == LaneCharPoint)
            ofs << next_g.x() << '\t' << next_g.y() << '\t' << 1 << '\t' << getFullId(*next) << endl;
        else
            ofs << next_g.x() << '\t' << next_g.y() << '\t' << 0 << '\t' << "0.0.0" << endl;
    }
    //output the tail
    QPainterPath p(bestPath.back()->scenePos());
    p.lineTo(bestPath.back()->scenePos() + delta);
    double pointsInThisPart = p.length();
    double step = 1 / pointsInThisPart;
    for(double pos = step; pos < 1; pos += step){
        QPointF apoint = mapToGlobal(p.pointAtPercent(pos));
        ofs << apoint.x() << '\t' << apoint.y() << '\t' << 0 << '\t' << "0.0.0" << endl;
    }
    ofs.close();

    return true;
}

bool MapScene::fromRndfx(const QString &fileName)
{
    using namespace std;
    ifstream ifs(fileName.toStdString());
    if(!ifs){
        DLOG(INFO) << "error reading " << fileName.toStdString();
        return false;
    }
    if(segments.size()){
        clearAll();
    }
    static map<string, RndfElement> elemByName{
        {"RNDF_name", RndfName}, {"num_segments", NumSegments}
        , {"num_zones", NumZones}, {"format_version", FormatVersion}
        , {"creation_date", CreationDate}, {"geometry", Geometry}
        , {"gridmap_name", GridmapName}, {"segment", SegmentId}
        , {"segment_name", SegmentName}, {"vertices", Vertices}
        , {"num_lanes", NumLanes}, {"lane", LaneId}
        , {"num_waypoints", NumWaypoints}, {"waypoint", Waypoint}
        , {"end_waypoint", EndWaypoint}, {"end_lane", EndLane}
        , {"end_segment", EndSegment}, {"end_file", EndFile}
        , {"connection", Connections},  {"end_vertices", EndVertices}
        , {"connection_control_points", ConnectionControlPoints},  {"end_connection_control_points", EndConnectionControlPoints}
        , {"controlpoint", ControlPoints},  {"end_controlpoint", EndControlPoints}
        , {"intersection", IntersectionId},  {"end_intersection", EndIntersection}
        , {"laneline", LaneLineId}, {"end_laneline", EndLaneLine}
        };
    string line;
    stringstream ss;
    string k;//used to store line key
    string v;//used to store value, corresponding to k
    double px(0), py(0), c1x(0), c1y(0), c2x(0), c2y(0);//used to store (x,y) coordinates of point
    string id, lineColor, fillColor, name;
    double lineWidth(0), speed(0);
    int prop1(0), prop2(0), trackFlag(0), t(0);
    //following four pointers are used to indicate current element while constructing them
    Segment *seg = NULL;
    Lane *lane = NULL;
    CharPoint *p = NULL;
    Connection *conn = NULL;
    bool overRead = false;

    //following lambda will do all the trick related to constructing a new CharPoint
    auto addPoint = [&]()mutable->void{
        while (1) {
            getline(ifs, line);
            ss.clear();
            ss << line;
            ss >> id >> px >> py >> prop1 >> prop2 >> speed >> trackFlag >> t
               >> name >> c1x >> c1y >> c2x >> c2y >> lineWidth >> lineColor >> fillColor;
            if(id.find('.') == string::npos){
                overRead = true;
                break;
            }
            //map track flag value from rndf definition to Qt's combo item value
            //which does not support negative value
            if((unsigned int)t == SharedVertex){
                p = findPointById(id);
                if(p){//if p already exists, means it's parent is a segment (not intersection)
                    Segment* sg = qgraphicsitem_cast<Segment*>(p->parentItem());
                    seg->assocSegments.insert(sg);
                    sg->assocIntersections.insert(seg);
                }
            }
            if((unsigned int)t != SharedVertex || !p){
                p = new CharPoint(numBehindDot(id), (PointType)t);
                p->setSpeed(speed)->setProp1(prop1)->setProp2(prop2)->setTrackFlag(trackFlag)
                        ->setLineWidth(lineWidth)->setName(QString::fromStdString(name))
                        ->setLineColor(QColor(QString::fromStdString(lineColor)))->setFillColor(QString::fromStdString(fillColor));
                addItem(p);
                p->setPos(mapToLocal(px, py));
                p->inControl->setPos(c1x, c1y);
                p->outControl->setPos(c2x, c2y);
            }
            switch (t) {
            case SharedVertex:
            case IntersectionVertex:
            case SegmentVertex:
                if(!p->parentItem())
                    p->setParentItem(seg);
                seg->addVertex(p);
                break;
            case StandAlone:
                break;
            case ConnectionControlPoint:
                p->setParentItem(conn);
                conn->keyPoints.push_back(p);
                p->setId(numBehindDot(id));
                break;
            case LaneLinePoint:
                p->setParentItem(lane);
                lane->addKeyPoint(p);
                p->setId(numBehindDot(id));
            default:
                p->setParentItem(lane);
                lane->addKeyPoint(p);
                p->setId(numBehindDot(id));
                break;
            }
        }
    };
    int minorVersion;//used to store value that indicate .rndf or .rndfx
    while(overRead || getline(ifs, line)){
        if(overRead){
            k = id;
            overRead = false;
        }else{
            ss.clear();
            ss << line;
            ss >> k >> v;
        }
        switch (elemByName[k]) {
        case RndfName:
        case NumSegments:
        case NumZones:
        case CreationDate:
        case NumWaypoints:
            break;
        case FormatVersion:
            minorVersion = numBehindDot(v);
            break;
        case Geometry:
//            xCenter = stof(v);
//            ss >> yCenter >> width >> height;
//            setSceneRect(0, 0, width, height);
            break;
        case SegmentId:
            seg = new Segment(stoi(v));
            addItem(seg);
            segments.push_back(seg);
            break;
        case IntersectionId:
            seg = new Segment(stoi(v));
            addItem(seg);
            intersections.push_back(seg);
            break;
        case SegmentName:
            seg->setName(QString::fromStdString(v));
            break;
        case Vertices:
            if(NULL == seg){
                DLOG(INFO) << "bad file format: vertices should be placed between 'segment' and 'end_segment'";
                return false;
            }
            addPoint();
            break;
        case EndVertices:
            finishIntersection();
            finishSegment();
            break;
        case LaneId:
            if(NULL == seg || !seg->isFullConstructed){
                DLOG(INFO) << "bad file format: lanes should be placed between 'end_vertices' and 'end_segment'";
                return false;
            }
            lane = new Lane(numBehindDot(v),seg);
            addItem(lane);
            seg->lanes.push_back(lane);
            break;
        case LaneLineId:
            if(NULL == seg || !seg->isFullConstructed){
                DLOG(INFO) << "bad file format: lanelines should be placed between 'end_vertices' and 'end_segment'";
                return false;
            }
            lane = new Lane(numBehindDot(v),seg);
            addItem(lane);
            seg->laneLines.push_back(lane);
            break;
        case Waypoint:
        case ControlPoints:
            if(NULL == seg || NULL == lane){
                DLOG(INFO) << "bad file format: waypoints(controlpoints) should be placed between 'lane(laneline)' and 'end_lane(end_laneline)'";
                return false;
            }
            addPoint();
            break;
        case EndLane:
        case EndLaneLine:
            lane->isFullConstructed = true;
            break;
        case Connections:
        {
            CharPoint* from = findPointById(v);
            string toStr;
            double speed = 0;
            int p2 = 0;
            ss >> toStr >> p2 >> speed;
            CharPoint* to = findPointById(toStr);
            if(from && to){
                conn = new Connection(connIndex++);
                conn->setProp2(p2);
                conn->setSpeed(speed);
                conns.push_back(conn);
                conn->from = from;
                conn->to = to;
                from->conns.insert(conn);
                to->conns.insert(conn);
                addItem(conn);
            }else{
                conn = NULL;
                DLOG(INFO) << "Error: could not find corresponding point for '" << v << "' or '" << toStr << "'";
            }
        }
            break;
        case ConnectionControlPoints:
            if(NULL == conn){
                DLOG(INFO) << "bad file format: could not find corresponding connection for control points."
                              "\nEither because of the wrong place of control point in the file or"
                              "connection from||to point couldn't be found.";
                return false;
            }
            addPoint();
        default:
            break;
        }
    }
    updateIndices();
    getLogger()->clear();
    return true;
}

bool MapScene::getMdfPts(const QString &fileName)
{
//    readConf("map.conf");
    using namespace boost::program_options;
    options_description conf;
    conf.add_options()
            ("xBias", value<double>(&xBias)->default_value(0),"x coordinate bias")
            ("yBias", value<double>(&yBias)->default_value(0),"y coordinate bias");
    variables_map vm;
    try {
        store(parse_config_file<char>("map.conf", conf), vm);
    } catch (const reading_file& e) {
        std::cout << "Failed to open file " << "map.conf" << ": "
                  << e.what();
    }
    notify(vm);

    std::ifstream ifs(fileName.toStdString());
    std::string line;
    if(!ifs){
        DLOG(INFO) << "error reading " << fileName.toStdString();
        return false;
    }
    MdfPoint p;
    while(getline(ifs, line)){
        std::stringstream ss;
        ss << line;
        ss >> p.id >> p.second >> p.first >> p.flag1 >> p.flag2;
        mdfPts.push_back(p);
    }
    ifs.close();
    //turn them into SELF coordinate
//    qDebug() << "mdfPts size: " << mdfPts.size();
    //the origin of gps coordination @Kun Cheng Lake - Chang Shu
    double Org_llh[3]= {31.59971848,120.7682072,18.8910392};
    Org_llh[0] = Org_llh[0] * M_PI / 180.0;
    Org_llh[1] = Org_llh[1] * M_PI / 180.0;
    double Org_xyz[3];
    llh2xyz(Org_llh, Org_xyz);
    for(auto &p : mdfPts){
        double input[3] = {p.first * M_PI / 180.0, p.second * M_PI / 180.0, 0};
        double output[3] = {0};
        llh2enu(input, Org_xyz, output);
        p.first = output[0] + xBias;
        p.second = output[1] + yBias;//the bias between our's coordinate and race commitee's coordinate
//        qDebug() << p.first << '\t' << p.second;
        AddSinglePoint(p);
    }
    return true;
}

void MapScene::updateIndices(unsigned int segindx, unsigned int intIndx, unsigned int zoneIndx, unsigned int connIndx)
{
    if(segindx){
        segIndex = segindx;
    }
    for(auto s : segments){
        if(segIndex <= s->id())
            segIndex = s->id() + 1;
        s->updateIndices();
    }
    if(intIndx){
        intersectIndex = intIndx;
    }
    for(auto i : intersections){
        if(intersectIndex <= i->id())
            intersectIndex = i->id() + 1;
        i->updateIndices();
    }
    if(connIndx){
        connIndex = connIndx;
    }
    for(auto c : conns){
        if(connIndex <= c->id())
            connIndex = c->id() + 1;
        c->updateCharIndex();
    }
    if(zoneIndx){
        zoneIndex = zoneIndx;
    }
    for(auto z : zones){
        if(zoneIndex <= z->id())
            zoneIndex = z->id() + 1;
        z->updateIndices();
    }

}

void MapScene::reArrangeIds()
{
    intersectIndex = 1;
    for(auto i : intersections){
        i->setId(intersectIndex++);
        i->reArrangeIds();
    }
    segIndex = 1;
    for(auto s : segments){
        s->setId(segIndex++);
        s->reArrangeIds();
    }
    zoneIndex = 1;
    for(auto z : zones){
        z->setId(zoneIndex++);
        z->reArrangeIds();
    }
    connIndex = 1;
    for(auto c : conns){
        c->setId(connIndex++);
        c->reArrangeIds();
    }
    return;
}

void MapScene::clearAll()
{
    clearRoutePoints();//this function must be called first!
    clear();
    setSceneRect(0,0,600,500);
    xCenter = yCenter = 0;
    width = height = 0;
    intersectIndex = segIndex = zoneIndex = connIndex = 1;
    conns.clear();
    segments.clear();
    intersections.clear();
    curConn = 0;
    editorType = typeToRetore = Hand;
    logger->clear();
    zones.clear();
    updateCursor();
}

bool MapScene::findBestPath()
{
    if(!routeKeyPoints.size()){
        getLogger()->appendPlainText(">Empty route points, do nothing");
        return true;
    }
    //find start point & target
    CharPoint* startPoint = NULL, *target = NULL;
    for(auto p : routeKeyPoints){
        qDebug() << p->getProp1();
        if(p->getProp1() == 0) startPoint = p;
        if(p->getProp1() == 4) target = p;
    }
    //verify they all found
    if(!startPoint || !target){
        getLogger()->appendPlainText(">Coundn't find start point or target");
        return false;
    }
    //move startPoint to front, target to back
    auto iter = std::find(routeKeyPoints.begin(), routeKeyPoints.end(), startPoint);
    routeKeyPoints.erase(iter);
    routeKeyPoints.push_front(*iter);
    iter = std::find(routeKeyPoints.begin(), routeKeyPoints.end(), target);
    routeKeyPoints.erase(iter);
    routeKeyPoints.push_back(*iter);
    //find path to go through all points
    auto prev = routeKeyPoints.begin(), next = prev;
    ++next;
    while(next != routeKeyPoints.end()){
        if(!findPathBetween(*prev, *next)){
            getLogger()->appendPlainText(tr(">Couldn't find a path from %1 to %2").arg(QString::fromStdString(getFullId(*prev))).arg(QString::fromStdString(getFullId(*next))));
            bestPath.clear();
            return false;
        }
        bestPath.pop_back();//if don't pop_back(), there would be duplicated points for routeKeyPoints
        ++prev, ++next;
    }
    bestPath.push_back(target);
    //output arrow to visualize the result
    auto _prev = bestPath.begin();
    auto _next = _prev; ++_next;
    while(_next != bestPath.end()){
        auto c = new Connection(0u);
        bestPathArrow.push_back(c);
        c->from = *_prev;
        c->to = *_next;
        c->setLineColor(Qt::yellow);
        addItem(c);
        c->updatePath();
        ++_prev, ++_next;
    }
    return true;
}

bool MapScene::findPathBetween(CharPoint* startPoint, CharPoint *target)
{
    using namespace std;
    set<CharPoint*> closedSet, openSet;
    openSet.insert(startPoint);
    map<CharPoint*, CharPoint*> cameFrom;
    auto reconstructPath = [&](CharPoint* current)->void{
        std::vector<CharPoint*> totalPath;
        totalPath.push_back(current);
        while(cameFrom.count(current)){
            if(current->conns.size()){//if current subpath is a connection, add all key points on that connection in reversed order.
                for(auto conn : conns){
                    if(conn->to == current && conn->from == cameFrom[current]){
                        totalPath.insert(totalPath.end(), conn->keyPoints.rbegin(), conn->keyPoints.rend());
                        break;
                    }
                }
            }
            current = cameFrom[current];
            totalPath.push_back(current);
        }
        bestPath.insert(bestPath.end(), totalPath.rbegin(), totalPath.rend());
    };
    map<CharPoint*, double> _gScore, _fScore;//they should not be accessed directly, use g(x), f(x) instead
    //define g(x) & f(x)
    auto g = [&](CharPoint * c)->double&{
        if(!_gScore.count(c)){
            _gScore[c] = numeric_limits<double>::max() / 2;//divide by 2 to avoid overflow
        }
        return _gScore[c];
    };
    auto f = [&](CharPoint * c)->double&{
        if(!_fScore.count(c)){
            _fScore[c] = numeric_limits<double>::max() / 2;
        }
        return _fScore[c];
    };
    //distance between two points. if target is passed as param,
    //then it's heuristic cost estimate function in effect.
    auto d = [&](CharPoint *cur, CharPoint *t)->double{
        QPointF delta = t->scenePos() - cur->scenePos();
        return sqrt(pow(delta.x(), 2) + pow(delta.y(), 2));//simply the distance between two points
    };
    g(startPoint) = 0;
    f(startPoint) = g(startPoint) + d(startPoint, target);
    //lambda: finding the node in OpenSet having the lowest f(x) value
    auto getCurrent = [&]()->CharPoint*{
        CharPoint * curr = (*(openSet.begin()));
        for(auto p : openSet){
            if(f(p) < f(curr)){
                curr = p;
            }
        }
        return curr;
    };
    while(openSet.size()){
        CharPoint* current = getCurrent();
        //debug
        DLOG(INFO) << getFullId(current);
        //end debug
        if(current == target){
            reconstructPath(current);
            getLogger()->appendPlainText(tr(">A route have been found!"));
            return true;
        }
        openSet.erase(current);
        closedSet.insert(current);
        //find neighbors of current
        std::vector<CharPoint*> neighbors;
        Lane *lane = qgraphicsitem_cast<Lane*>(current->parentItem());
        Segment *seg = qgraphicsitem_cast<Segment *>(lane->parentItem());
        if(current == lane->keyPoints.back()){
            /*currently not a good choice for neighbor*/
//            for(auto l : seg->lanes){
//                if(l != lane){
//                    neighbors.push_back(l->keyPoints.back());
//                }
//            }
        }else{
            auto iter = std::find(lane->keyPoints.begin(), lane->keyPoints.end(), current);
            ++iter;
            neighbors.push_back(*iter);
//            if(current == lane->keyPoints.front()){
//                for(auto l : seg->lanes){
//                    if(l != lane){
//                        neighbors.push_back(l->keyPoints.front());
//                    }
//                }
//            }
        }
        if(current->conns.size()){
            for(auto conn : current->conns){
                if(conn->from == current){
                    neighbors.push_back(conn->to);
                }
            }
        }
        //end finding neighbors
        for(auto n : neighbors){
            if(closedSet.count(n))
                continue;
            double tentative_g = g(current) + d(current, n);
            if(!openSet.count(n)){
                openSet.insert(n);
            }
            else if(tentative_g >= g(n)){
                continue;
            }
            cameFrom[n] = current;
            g(n) = tentative_g;
            f(n) = g(n) + d(n, target);
        }
    }
    getLogger()->appendPlainText(">Coundn't find a valid route");
    return false;
}

std::string MapScene::getFullId(CharPoint *c)
{
    std::stringstream ss;
    Segment* seg = NULL;
    Lane* lane = NULL;
    Connection* conn = NULL;
    switch (c->getType()) {
    case SegmentVertex:
    case IntersectionVertex:
        seg = qgraphicsitem_cast<Segment*>(c->parentItem());
        ss << seg->id() << '.';
        break;
    case ConnectionControlPoint:
        conn = qgraphicsitem_cast<Connection*>(c->parentItem());
        ss << conn->id() << '.';
    case StandAlone:
        ss << "0.";
        break;
    default://lane || laneline
        lane = qgraphicsitem_cast<Lane*>(c->parentItem());
        seg = qgraphicsitem_cast<Segment*>(lane->parentItem());
        ss << seg->id() << '.' << lane->id() << '.' << c->id();
        break;
    }
    std::string line;
    std::getline(ss, line);
    return line;
}

void MapScene::processHdl(victl::HdlEngine *h)
{
    isProcessingHdl = true;
    saveALocalMap = false;
    while(isProcessingHdl && h->preProcess()){
        //if paused, we want the app still respond to user inputs
        while(isProcessingHdlPaused){
            qApp->processEvents();
        }
        DLOG(INFO) << "#####begin CONTAINS test";
        //processing
        size_t totalNum;
        victl::HdlPoint* pcd = h->getCurrentPcd(totalNum);
        victl::HdlPointXYZ *pcdXyz = h->getCurrentXyz();
        victl::array2d ptsBeam = h->getCurrentPtsByBeam();
        int* beam_counter = h->getCurrentPtsByBeamSize();
        const victl::Carpose carpose = h->getCurrentPose();
        QPointF carPos = mapToLocal(carpose.x, carpose.y);
        if(!carImgItem)
            carImgItem = addPixmap(QPixmap(":/images/car.png"));
        carImgItem->setPos(carPos  - QPointF(5, 10.5)); //shift by half size of the image
        carImgItem->setRotation(carpose.eulr * 180 / M_PI);
        //finding adjacent segments
        std::set<Segment *> adjSegs;
        for(auto seg : segments){
            if(seg->contains(carPos)){
                adjSegs.insert(seg);
                if(seg->assocIntersections.size())
                    adjSegs.insert(seg->assocIntersections.begin(), seg->assocIntersections.end());
                break;
            }
        }
        for(auto seg : intersections){
            if(seg->contains(carPos)){
                adjSegs.insert(seg);
                if(seg->assocSegments.size())
                    adjSegs.insert(seg->assocSegments.begin(), seg->assocSegments.end());
                break;
            }
        }
        QPainterPath path(carPos);
        QPainterPath adjPath;
        static const int DETECT_RANGE = 325;
        path.addEllipse(carPos, DETECT_RANGE , DETECT_RANGE);
        for(auto seg : adjSegs){
            adjPath = adjPath.united(seg->getNormalPath());
        }
        path = path.intersected(adjPath);
//        finish finding adjacent segments
        auto filterPtsInSegs = [&](int id)mutable->bool{
            int newCounter = 0;
            for(int i = 0; i < beam_counter[id]; ++i){
                int indx = ptsBeam[id][i];
                if(pcd[indx].y > 0 && pcd[indx].z < -1100 && pcd[indx].intensity != 0
                        && path.contains(mapToLocal(pcdXyz[indx].x / 1000.0, pcdXyz[indx].y / 1000))
                  ){
                    ptsBeam[id][newCounter++] = indx;
                }
            }
            beam_counter[id] = newCounter;
            return true;
        };
        //multi-thread process each beam points
        std::future<bool> results[LASER_NUM];
        for(int i = 0; i < LASER_NUM; ++i){//run each threads asyncronously
            results[i] = std::async(std::launch::async, filterPtsInSegs, i);
        }
        for(int i = 0; i < LASER_NUM; ++i){//make sure all threads complete their works
            if(results[i].get()){}
        }
        DLOG(INFO) << ">>>>>>>end CONTAINS test";
        qApp->processEvents();
        h->resumeProcessing();
        qApp->processEvents();
        if(saveALocalMap){
            static int mapid = 0;
            h->saveLocalMap("/tmp/" + std::to_string(++mapid) +".png");
            h->visualLocalMap("/tmp/" + std::to_string(mapid) +"-v.png");
            saveALocalMap = false;
        }
//        QPixmap pixmap = probabilityImgItem->pixmap();
//        QPainter painter(&pixmap);
//        painter.setPen(Qt::yellow);
//        for(auto p : lanePts){
//            painter.drawPoint(mapToLocal(p.x/1000.0, p.y/1000.0));
//        }
//        probabilityImgItem->setPixmap(pixmap);
//        //calc probability
//        static const float AVG_BIAS = 0.8;
//        static const float NEAR_STEP = 0.01;
//        static const float FAR_STEP = 0.1;
//        static const int FAR_THRES = 20000;//30 meters
//        for(size_t i = 0; i < NUM_BEAM; ++i){//calc average intesity
//            iAvg[i] /= pts_beam[i].size();
//            iAvg[i] *= AVG_BIAS;
//        }
//        qApp->processEvents();
//        //predefine variables to avoid reallocation
//        QPointF curPos;
//        double prob;
//        uchar val;
//        double coeff;
//        for(size_t i = 0; i < NUM_BEAM; ++i){
//            for(auto p : pts_beam[i]){
//                curPos = mapToLocal(p.x, p.y);
//                val = qRed(((QRgb*)(probabilityImg->scanLine((int)curPos.y())))[(int)curPos.x()]);
//                prob = val / 255.0;
//                coeff = prob > 0.5 ? 1 - prob : prob;
//                if(p.distance >= FAR_THRES){
//                    prob += coeff * FAR_STEP;
//                }else{
//                    prob += coeff * NEAR_STEP * (p.intensity - iAvg[i]) / iAvg[i];
//                }
//                if(prob > 0.99) prob = 0.99;
//                if(prob < 0.01) prob = 0.01;
//                val = uchar(prob * 255);
//                ((QRgb*)(probabilityImg->scanLine((int)curPos.y())))[(int)curPos.x()]=qRgb(val, val, val);
//            }
//            qApp->processEvents();
//        }
//        }

    }//end outer while loop
    if(isProcessingHdl){
        getLogger()->appendPlainText(">Finished processing HDL");
    }else{
        getLogger()->appendPlainText(">Interrupted processing HDL");
    }
    h->saveLocalMap("/tmp/final-3b.png");
    h->visualLocalMap("/tmp/v-final-3b.png");
    delete carImgItem;
    carImgItem = NULL;
}

void MapScene::writeBoundaryBit(const QString &fileName)
{
    cv::Mat b = cv::imread(fileName.toStdString());
    if(b.rows != height || b.cols != width){
        getLogger()->appendPlainText(">size of 3b-png & current map inconsist.");
        return;
    }
    cv::Mat v(b.rows, b.cols, CV_8UC1, cv::Scalar(255));
    QImage o(b.cols, b.rows, QImage::Format_RGB32);
    o.fill(Qt::white);
    QPainter painter(&o);
    painter.setPen(Qt::black);
    painter.setBrush(Qt::black);
    auto writeBetween = [&](const CharPoint* curr, const CharPoint* next) mutable -> void{
        QPainterPath p(curr->scenePos());
        p.cubicTo(curr->outControl->scenePos(), next->inControl->scenePos(), next->scenePos());
        double step = 0.5 / p.length();
        QPointF loc;
        for(double percent = 0; percent <=1; percent += step){
            loc = p.pointAtPercent(percent);
            if(loc.x() < 0 || loc.x() > width || loc.y() < 0 || loc.y() >height){
                continue;
            }
            unsigned char &base = b.at<cv::Vec3b>(lround(loc.y()), lround(loc.x()))[0];
            base |= victl::BOUNDARY;
            v.at<uchar>(lround(loc.y()), lround(loc.x())) = 0;
        }
    };
    auto writeSeg = [&](const std::list<Segment*>& segs) mutable -> void{
        for(auto seg : segs){
            //output boundary
            auto curr = seg->vertices.begin(); auto next = curr;
            ++next;
            while(next != seg->vertices.end()){
                if((*curr)->getType() == SharedVertex && (*next)->getType() == SharedVertex){
                    ++curr, ++next;
                    continue;
                }else{
                    writeBetween(*curr, *next);
                    ++curr, ++next;
                }
            }
             if(seg->vertices.front()->getType() != SharedVertex || seg->vertices.back()->getType() != SharedVertex){
                 writeBetween(seg->vertices.front(), seg->vertices.back());
             }
             //finish output boundary
             //output occupation
             QPainterPath path = seg->getNormalPath();
             painter.drawPath(path);
        }
    };
    writeSeg(segments);
    writeSeg(intersections);
    int counter = 0;
    for(int i = 0; i < height; ++i){
        for(int j = 0; j < width; ++j){
            if(qRed(((QRgb*)(o.scanLine(i)))[j]) == 255){
//                if(j > 13650){
                    unsigned char &base = b.at<cv::Vec3b>(i, j)[1];
                    base |= victl::CURB_YES;
                    ++counter;
//                }
            }
        }
    }
    DLOG(INFO) << width << '\t' << height;
    DLOG(INFO) << "wrote: " << counter;
    cv::imwrite(fileName.toStdString() + "-with-boundary.png", b);
    cv::imwrite(fileName.toStdString() + "-boundary-visual.png", v);
    return;
}

void MapScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons()==Qt::LeftButton){
        switch (editorType) {
        case DrawSegment:
            addSegmentVertex(event->scenePos());
            break;
        case DrawIntersection:
        {
            QGraphicsScene::mousePressEvent(event);
            if(event->isAccepted()/*0 == intersections.size() || intersections.back()->isFullConstructed*/){
                break;
            }
            if(!intersections.back()->isFullConstructed)
                intersections.back()->addVertex(0, event->scenePos(), IntersectionVertex);
            break;
        }
        case DrawLaneLine:
            for(auto seg : segments){
                if(seg->contains(event->scenePos())){
                    seg->addLaneLinePoint(LaneLinePoint, event->scenePos());
                }
            }
            break;
        case DrawLane:
            for(auto seg : segments){
                if(seg->contains(event->scenePos())){
                    PointType t = (event->modifiers() & Qt::ShiftModifier) ? LaneCharPoint : LaneControlPoint;
                    seg->addLanePoint(t, event->scenePos());
                }
            }
            break;
        case AddPoint:
        {
            CharPoint *pt = new CharPoint(standaloneIndex++, StandAlone);
            addItem(pt);
            standalonePts.push_back(pt);
            pt->setPos(event->scenePos());
        }
            break;
        default:
            break;
        }
    }
    if(event->buttons() == Qt::RightButton){
        switch (editorType) {
        case DrawSegment:
            finishSegment();
            break;
        case DrawIntersection:
            finishIntersection();
            break;
        case DrawLane:
            finishLane();
            break;
        case DrawLaneLine:
            finishLaneLine();
            break;
        default:
//            event->setAccepted(false);
            break;
        }
    }
    if(!event->isAccepted())
        QGraphicsScene::mousePressEvent(event);
}

void MapScene::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Space:
        if(Hand != editorType){
            typeToRetore = editorType;
            editorType = Hand;
        }else{
            editorType = typeToRetore;
        }
        updateCursor();
        break;
    case Qt::Key_Delete:
    {
        QListIterator<QGraphicsItem*> i(selectedItems());
        while(i.hasNext()){
            QGraphicsItem * item = i.next();
            CharPoint *cp = qgraphicsitem_cast<CharPoint *>(item);
            Segment *seg = qgraphicsitem_cast<Segment *>(item);
            Lane *lane = qgraphicsitem_cast<Lane *>(item);
            Connection *conn = qgraphicsitem_cast<Connection *>(item);
            //The start and end point of a lane, vertexes of segments are not permited to be deleted
            if(cp && SharedVertex == cp->getType()){
                break;
            }
            if(cp){
                cp->removeFromParent();
            }
            if(seg){
                removeSeg(seg);
            }
            if(lane){
                lane->removeFromParent();
            }
            if(conn){
                removeConn(conn);
            }
            delete item;
        }
        break;
    }
    case Qt::Key_End:
        isProcessingHdlPaused = isProcessingHdlPaused? false : true;
        break;
    case Qt::Key_Escape:
        isProcessingHdl = false;
        break;
    case Qt::Key_Insert:
        saveALocalMap = true;
        break;
    case Qt::Key_X:
    {
        auto items = selectedItems();
        CharPoint* i = 0;
        if(items.size())
            i = qgraphicsitem_cast<CharPoint*>(selectedItems().first());
        if(i){
            i->inControl->setPos(0,0);
            i->outControl->setPos(0,0);
        }
    }
    case Qt::Key_Z:
    {
        if(event->modifiers() & Qt::ControlModifier && routeKeyPoints.size()){
            getLogger()->appendPlainText("poped a key point:" + QString::number(routeKeyPoints.back()->id())) ;
            routeKeyPoints.pop_back();
        }
    }
        break;
    case Qt::Key_1://go to the first mission point
        findMissionPoint(1);
        break;
    case Qt::Key_0://go to the last mission point
        findMissionPoint();
    case Qt::Key_P://previous one
        findMissionPoint(--currentMissionPt);
        break;
    case Qt::Key_N://next one
        findMissionPoint(++currentMissionPt);
        break;
    case Qt::Key_G:
        if(event->modifiers() & Qt::ShiftModifier){
            bool ok;
            int n = QInputDialog::getInt(0, "Goto", "Mission Point ID: ", 0, 0, 2147483647, 1, &ok);
            if(ok){
                findMissionPoint(n);
            }
        }
        break;
    default:
        break;
    }
    QGraphicsScene::keyPressEvent(event);
}

unsigned int MapScene::numBehindDot(string v)
{
    return stoi(v.substr(v.rfind('.')+1));
}

QPlainTextEdit *MapScene::getLogger() const
{
    return logger;
}

void MapScene::setLogger(QPlainTextEdit *value)
{
    logger = value;
}

bool MapScene::loadCarTrack(const QString &fileName, bool hasId)
{
    clearTrack();
    int id;
    double x, y;
    std::ifstream ifs(fileName.toStdString());
    std::string line;
    if(ifs){
        while(std::getline(ifs, line)){
            std::stringstream ss;
            ss << line;
            if(hasId){
                ss >> id >> x >> y;
            }else{
                ss >> x >> y;
            }
            trackpoints.push_back(std::make_pair(x, y));
        }
        ifs.close();
    }
    else
        return false;
    addCarTrack(trackpoints);
    return true;
}

void MapScene::clearTrack()
{
    for(auto p : tracks){
        delete p;
    }
    tracks.clear();
    trackpoints.clear();
}

bool MapScene::saveKeyPoints(const QString &fileName)
{
    if(routeKeyPoints.size())
    {
        std::ofstream ofs(fileName.toStdString());
        if(!ofs)
            return false;
        for(auto p : routeKeyPoints){
            ofs << pointInfoEmitor(p) << std::endl;
        }
    }else
        return false;
    return true;
}

bool MapScene::restoreKeyPoints(const QString &fileName, bool createIfNotExist)
{
    std::ifstream ifs(fileName.toStdString());
    if(!ifs)
        return false;
    std::string line, id;
    while(ifs){
        std::stringstream ss;
        getline(ifs, line);
        ss << line;
        ss >> id;
        CharPoint *pt = findPointById(id);
        if(pt){
            routeKeyPoints.push_back(pt);
        }else if(createIfNotExist){
            //criteria not avialable for the moment
            continue;
        }else{
            DLOG(ERROR) << "Failed to find route key point: " << id;
            continue;
        }
    }
    return true;
}

string MapScene::pointInfoEmitor(CharPoint *w)
{
    using namespace std;
    stringstream ss;
    QPointF g = mapToGlobal(w->scenePos());
    Segment* seg = NULL;
    Lane* lane = NULL;
    Connection* conn = NULL;
    switch (w->getType()) {
    case SegmentVertex:
    case IntersectionVertex:
    case SharedVertex:
        seg = qgraphicsitem_cast<Segment*>(w->parentItem());
        ss << seg->id() << '.';
        break;
    case ConnectionControlPoint:
        conn = qgraphicsitem_cast<Connection*>(w->parentItem());
        ss << conn->id() << '.';
    case StandAlone:
        ss << "0.";
        break;
    default://lane || laneline
        lane = qgraphicsitem_cast<Lane*>(w->parentItem());
        seg = qgraphicsitem_cast<Segment*>(lane->parentItem());
        ss << seg->id() << '.' << lane->id() << '.';
        break;
    }
    ss << w->id() << '\t' << g.x() << '\t' << g.y() << '\t' << w->getProp1() << '\t' << w->getProp2() << '\t'  << w->getSpeed() << '\t' << w->getTrackFlag() << '\t';

    //output type, name, control points, styles etc.
    ss << (int)w->getType() << '\t' << w->getName().toStdString() << '\t' << w->inControl->pos().x() << '\t' << w->inControl->pos().y() << '\t'
        << w->outControl->pos().x() << '\t' << w->outControl->pos().y() << '\t' <<  w->lineWidth() << '\t'
        << w->lineColor().name().toStdString() << '\t' << w->fillColor().name().toStdString();
    string str;
    getline(ss, str);
    return str;
}

CharPoint *MapScene::findPointById(const string &s)
{
    auto first_of = s.find_first_of('.');
    auto last_of = s.find_last_of('.');
    unsigned int pid = numBehindDot(s);
    unsigned int lid = 0;
    unsigned int sid = (unsigned int) stoi(s.substr(0, first_of));
    if(first_of != string::npos && last_of != string::npos && first_of != last_of){
        lid = (unsigned int) stoi(s.substr(first_of + 1, last_of));
    }
    CharPoint* pt = NULL;
    Lane* ln = NULL;
    Segment* sg = NULL;
    for(auto seg : segments){
        if(seg->id() == sid){
            sg = seg;
            break;
        }
    }
    if(sg)
        for(auto lane : sg->lanes){
            if(lane->id() == lid){
                ln = lane;
                break;
            }
        }
    if(lid && ln)
        for(auto point : ln->keyPoints){
            if(point->id() == pid){
                pt = point;
                return pt;
            }
        }
    if(!lid)
        for(auto point : sg->vertices){
            if(point->id() == pid){
                pt = point;
                return pt;
            }
        }
//        if(!pt)DLOG(INFO) << "couldn't find: " << s;
    return pt;
}

void MapScene::AddSinglePoint(const MdfPoint &p)
{
    CharPoint *pt = new CharPoint(standaloneIndex++, StandAlone);
    pt->setProp1(p.flag1);
    pt->setProp2(p.flag2);
    pt->setId(p.id);
    standalonePts.push_back(pt);
    addItem(pt);
    pt->setPos(mapToLocal(p.first, p.second));
}

bool MapScene::readConf(const std::string &fileName)
{
//    using namespace boost::program_options;
//    options_description conf;
//    conf.add_options()
//            ("xBias", value<double>(&xBias)->default_value(0),"x coordinate bias")
//            ("yBias", value<double>(&yBias)->default_value(0),"y coordinate bias");
//    variables_map vm;
//    try {
//        store(parse_config_file<char>(fileName.c_str(), conf), vm);
//    } catch (const reading_file& e) {
//        std::cout << "Failed to open file " << fileName << ": "
//                  << e.what();
//    }
//    notify(vm);
    return true;
}

void MapScene::findMissionPoint(int n)
{
    if(n < 0){ n = 0;} else
    if(n == 0){
        for(auto p : standalonePts){
            if(p->id() > n){
                n = p->id();
            }
        }
        currentMissionPt = n;
        findMissionPoint(n);
    }else if(n == 1){
        n = 100000;// a real big integer
        for(auto p : standalonePts){
            if(p->id() < n){
                n = p->id();
            }
        }
        currentMissionPt = n;
        findMissionPoint(n);
    }else{
        for(auto p : standalonePts){
            if(p->id() == n){
                views().front()->centerOn(p);
                p->setSelected(true);
            }
        }
    }
}

template<class T>
bool MapScene::addCarTrack(const std::vector<T> &points)
{
    addItem(trackitems);
    for(auto p : points){
        QGraphicsPixmapItem * pi = new QGraphicsPixmapItem(oneTrackPoint);
        addItem(pi);
        pi->setPos(mapToLocal(QPointF(p.first, p.second)) - QPointF(1,1));//minus the half size of image
        trackitems->addToGroup(pi);
        qDebug() << mapToLocal(QPointF(p.first, p.second)) - QPointF(1,1);
        tracks.push_back(pi);
    }
    return true;
}

Segment *MapScene::curIntersect()
{
    if (intersections.size() == 0)
        return 0;
    else
        return intersections.back();
}

Segment* MapScene::addIntersect()
{
    Segment *intersect = new Segment(intersectIndex++);
    addItem(intersect);
    intersections.push_back(intersect);
    getLogger()->appendPlainText(tr(">Drawing intersection %1").arg(intersect->id()));
    return intersect;
}

