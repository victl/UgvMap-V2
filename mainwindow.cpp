#include "inc.h"
#include "def.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mapview.h"
#include "mapscene.h"
#include "mapobject.h"
#include "charpoint.h"
#include "lane.h"
#include "connection.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsView>
#include <QActionGroup>
#include <QGraphicsItem>
#include <QGraphicsItemGroup>
#include <QString>
#include <QList>
#include <fstream>
#include <utility>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , scene(new MapScene(0, 0, 600, 500))
    , view(new MapView)
    , gridmapItem(0)
    , hdlEngine(0)
    , gridmap(new QPixmap)
    , gridmapClip(new QPixmap)
    , gridmapClipItem(0)
{
    ui->setupUi(this);
    view->setScene(scene);
    setCentralWidget(view);
    setWindowTitle(tr("UGV Map"));
    initEnv();
}

MainWindow::~MainWindow()
{
    delete ui;
    if (hdlEngine)
        delete hdlEngine;
}

void MainWindow::on_actionOpenHdl_triggered()
{
    QString fileName = open(QString(), "Open HDL file", "HDL Point Cloud files (*.hdl)");
    if(fileName.isEmpty())
        return;
    if (hdlEngine)
        delete hdlEngine;
    std::string filedir = QFileInfo(fileName).absolutePath().toStdString();
    hdlEngine = new victl::HdlEngine(fileName.toStdString());
    hdlEngine->processAllFrames(filedir);
    loadGridMap(QString::fromStdString(filedir + "/visualized-final.png"));
}

void MainWindow::on_actionFindByHdl_triggered()
{
    on_actionHand_toggled(true);
    QString fileName = open(QString(), "Open HDL file", "HDL Point Cloud files (*.hdl)");
    if(fileName.isEmpty())
        return;
    if (hdlEngine)
        delete hdlEngine;
    hdlEngine = new victl::HdlEngine(fileName.toStdString());
    scene->processHdl(hdlEngine);
}

void MainWindow::on_actionOpenGridMap_triggered()
{
    QString fileName = open(QString(), "Open Grid Map file", "Grid Map file (*.png)");
    if(fileName.isEmpty())
        return;
    loadGridMap(fileName);
}

void MainWindow::on_actionOpenMixedMap_triggered()
{
    QString fileName = open(QString(), "Open Mixed Map file", "Mixed Map file (*.txt)");
    if(fileName.isEmpty())
        return;
    scene->fromRndfx(fileName);
}

void MainWindow::on_actionImportTaskDescription_triggered()
{
    QString fileName = open(QString(), "Open Mission Discription file", "Mission Discription file (*.txt)");
    if(fileName.isEmpty())
        return;
    scene->getMdfPts(fileName);
}

void MainWindow::on_actionAddMapClip_triggered()
{
    //readin two maps, two (meta-info)headers
    QString fileName = open(QString(), "Open Grid Map file", "Grid Map file (*.png)");
    if(fileName.isEmpty())
        return;
    cv::Mat base = cv::imread(fileName.toStdString());
    fileName = open(QString(), "Open Map Meta file", "Map meta files (*.txt)");
    if(fileName.isEmpty())
        return;
    loadMapMeta(fileName);
    fileName = open(QString(), "Open Grid Map file", "Grid Map file (*.png)");
    if(fileName.isEmpty())
        return;
    cv::Mat addition = cv::imread(fileName.toStdString());
    fileName = open(QString(), "Open Map Meta file", "Map meta files (*.txt)");
    if(fileName.isEmpty())
        return;
    loadMapMeta(fileName, true);//'true' means "is addtion"

    // merge
    QPointF additionCenter = scene->mapToLocal(xCenter, yCenter);
    QPoint offset = (additionCenter - QPointF(width / 2, height / 2)).toPoint();
    for(int i = 0; i < height; ++i){
        for(int j = 0; j < width; ++j){
            if((i + offset.y()) >= base.rows ||( i + offset.y()) < 0 || (j + offset.x()) < 0 || (j + offset.x() >= base.cols )){
                continue;
            }
            unsigned char &a = addition.at<cv::Vec3b>(i, j)[0];
            unsigned char &b = base.at<cv::Vec3b>(i + offset.y(), j + offset.x())[0];
//            if(addition.at<cv::Vec3b>(i, j)[2] == 200){
//                continue;
//            }else{
//                b = a;
//                base.at<cv::Vec3b>(i + offset.y(), j + offset.x())[1] = addition.at<cv::Vec3b>(i, j)[1];
//                base.at<cv::Vec3b>(i + offset.y(), j + offset.x())[2] = addition.at<cv::Vec3b>(i, j)[2];
//            }
            if(victl::isPresent(b, victl::ROADEDGE_OCCUPIED) || a == victl::ROADEDGE_UNKNOWN || (victl::isPresent(b, victl::ROADEDGE_CLEAR) && victl::isPresent(a, victl::ROADEDGE_CLEAR))){
                continue;
            }else{
                b = a;
            }
        }
    }

//    auto likelihood = [&](double xCenter_, double yCenter_, bool doUpdate = false)->unsigned long {
//        QPointF clipCenter = scene->mapToLocal(xCenter_, yCenter_);
//        QPoint offset = (clipCenter - QPointF(width / 2, height / 2)).toPoint();
//        unsigned long c = 0;
//        QRgb* a, *b;//addition & base
//        uchar av, bv; //addition value & ...
//        bool ao, bo; //addition occupied & ...
//        int counter = 0;
//        for(int i = 0; i < height; ++i){
//            for(int j = 0; j < width; ++j){
//                if((i + offset.y()) >= base.height() ||( i + offset.y()) < 0 || (j + offset.x()) < 0 || (j + offset.x() >= base.width() )){
//                    continue;
//                }
//                a = (QRgb*)addition.scanLine(i);
//                b = (QRgb*)base.scanLine(i + offset.y());
//                av = (uchar)qBlue(a[j]);
//                bv = (uchar)qBlue(b[j + offset.x()]);
//                ao = victl::isPresent(av, victl::ROADEDGE_OCCUPIED);
//                bo = victl::isPresent(bv, victl::ROADEDGE_OCCUPIED);
//                if(!doUpdate){
//                    c += ao && bo;
//                }else{
//                    if(bo || victl::isPresent(av, victl::ROADEDGE_UNKNOWN) || (victl::isPresent(bv, victl::ROADEDGE_CLEAR) && victl::isPresent(av, victl::ROADEDGE_CLEAR))){
//                        continue;
//                    }else{
//                        b[j + offset.x()] = a[j];
//                        ++counter;
//                    }
//                }
//            }
//        }
//        DLOG(INFO) << "IMPORTED: " << counter;
//        return c;
//    };
//    unsigned long bestMatch = 0;
//    int besti, bestj;
//    for(double i = -1; i < 1; i += 0.2){
//        for(double j = -1; j < 1; j += 0.2){
//            auto v = likelihood(xCenter + i, yCenter + j);
//            if(v > bestMatch){
//                bestMatch = v;
//                besti = i;
//                bestj = j;
//            }
//        }
//    }
//    xCenter += besti;
//    yCenter += bestj;
//    DLOG(INFO) << "(i,j): (" << besti << ',' << bestj << ')';
//    likelihood(xCenter, yCenter, true);
//    gridmap->fromImage(base);
//    gridmapItem->setPixmap(&gridmap);
//    base.save(fileName + "-merged.png");
    cv::imwrite(fileName.toStdString() + "-merged.png", base);
}

void MainWindow::on_actionSaveRndf_triggered()
{
    QString fileName = save(QString(), "Save RNDF file", "RNDF files (*.txt)");
    if(fileName.isEmpty())
        return;
    scene->exportRndf(fileName);
}

void MainWindow::on_actionExportFinalRoute_triggered()
{
    QString fileName = save(QString(), "Save Route file", "Route files (*.thu)");
    if(fileName.isEmpty())
        return;
    scene->exportRouteTrack(fileName);
}

void MainWindow::on_actionSaveMergedMap_triggered()
{
    QString fileName = save(QString(), "Save Merged map file", "Image files (*.png)");
    if(fileName.isEmpty())
        return;
    QImage img(scene->sceneRect().width(),scene->sceneRect().height(), QImage::Format_RGB32);
    QPainter painter(&img);
    scene->render(&painter, scene->sceneRect(), img.rect());
    img.save(fileName);
}

void MainWindow::on_actionWriteBoundaryBit_triggered()
{
    QString fileName = open(QString(), "Open Grid Map file", "Grid Map file (*.png)");
    if(fileName.isEmpty())
        return;
    scene->writeBoundaryBit(fileName);
}

void MainWindow::on_actionHdlToTxt_triggered()
{
    int id = QInputDialog::getInt(this, "Please input the frame you want to save", "Frame No. :", 1);
    if(hdlEngine){
        delete hdlEngine;
    }
    QString fileName = open(QString(), "Open HDL file", "HDL Point Cloud files (*.hdl)");
    if(fileName.isEmpty())
        return;
    hdlEngine = new victl::HdlEngine(fileName.toStdString());
    for(int i = 0; i < id - 1; ++i){
        hdlEngine->readPointsFromFile();
    }
    size_t sz;
    hdlEngine->readPointsFromFile();
    victl::HdlPoint *pcd = hdlEngine->getCurrentPcd(sz);
    fileName = save(QString(), "Save HDL Frame", "text files (*.txt)");
    if(fileName.isEmpty())
        return;
    std::ofstream ofs(fileName.toStdString(), std::ios_base::out);
    if(!ofs)
        return;
    ofs << "size: " << sz <<std::endl
        << "BeamId\tRotAngle\t\tDistance\t\tIntense\tx\t\ty\t\tz" << std::endl;
    for(size_t i = 0; i < sz; ++i){
        ofs << pcd[i].beamId <<"\t" << pcd[i].rotAngle << "\t\t"
            << pcd[i].distance << "\t\t" << pcd[i].intensity << '\t'
            << pcd[i].x << "\t\t" << pcd[i].y << "\t\t" << pcd[i].z << std::endl;
    }
    ofs.close();
    statusBar()->showMessage("Output frame finished", 2000);
}

void MainWindow::on_actionAddKeyPoint_toggled(bool flag)
{
    if(flag)
        scene->editorType = AddPoint;
    else
        scene->editorType = Hand;
    scene->updateCursor();
    updateStatusBar();
}

void MainWindow::on_actionDrawSegment_toggled(bool flag)
{
    if(flag)
        scene->editorType = DrawSegment;
    else
        scene->editorType = Hand;
    scene->updateCursor();
    updateStatusBar();
}

void MainWindow::on_actionEditor_toggled(bool flag)
{
    if(flag)
        scene->editorType = Editor;
    else
        scene->editorType = Hand;
    scene->updateCursor();
    updateStatusBar();
}

void MainWindow::on_actionDrawLaneLine_toggled(bool flag)
{
    if(flag)
        scene->editorType = DrawLaneLine;
    else
        scene->editorType = Hand;
    scene->updateCursor();
    updateStatusBar();
}

void MainWindow::on_actionDrawLane_toggled(bool flag)
{
    if(flag)
        scene->editorType = DrawLane;
    else
        scene->editorType = Hand;
    scene->updateCursor();
    updateStatusBar();
}

void MainWindow::on_actionCurveAjustor_toggled(bool flag)
{
    if(flag)
        scene->editorType = CurveAdjustor;
    else
        scene->editorType = Hand;
    scene->updateCursor();
    updateStatusBar();
}

void MainWindow::on_actionSpeedAdjustor_toggled(bool flag)
{
    if(flag){
        scene->editorType = SpeedAdjustor;
        auto items = scene->items();
        for(auto i : items){
            CharPoint *p = qgraphicsitem_cast<CharPoint*>(i);
            if(p)
                p->setFlag(QGraphicsItem::ItemIsSelectable);
            Connection *c = qgraphicsitem_cast<Connection *>(i);
            if(c)
                c->setFlag(QGraphicsItem::ItemIsSelectable);
        }
    }
    else
        scene->editorType = Hand;
    scene->updateCursor();
    updateStatusBar();
}

void MainWindow::on_actionHand_toggled(bool flag)
{
    if(flag)
        scene->editorType = Hand;
    scene->updateCursor();
    updateStatusBar();
}

void MainWindow::on_actionAddConnection_toggled(bool flag)
{
    if(flag)
        scene->editorType = AddConnection;
    else
        scene->editorType = Hand;
    scene->updateCursor();
    updateStatusBar();
}

void MainWindow::on_actionDrawIntersection_toggled(bool flag)
{
    if(flag)
        scene->editorType = DrawIntersection;
    else
        scene->editorType = Hand;
    scene->updateCursor();
    updateStatusBar();
}

void MainWindow::on_actionSelectRoutePoints_toggled(bool flag)
{
    if(flag)
        scene->editorType = SelectRoute;
    else
        scene->editorType = Hand;
    scene->updateCursor();
    updateStatusBar();
}

void MainWindow::on_actionClearRoutePoints_triggered()
{
    scene->clearRoutePoints();
}

void MainWindow::on_actionClearAll_triggered()
{
    scene->clearAll();
}

void MainWindow::on_actionReArrangeIds_triggered()
{
    scene->reArrangeIds();
}

void MainWindow::on_actionFindPath_triggered()
{
    scene->findBestPath();
}

void MainWindow::on_actionImportTrack_triggered()
{
    QString fileName = open(QString(), "Open Track file", "Car Track files (*.txt *.carposes)");
    if(fileName.isEmpty())
        return;
    scene->loadCarTrack(fileName);
}

void MainWindow::on_actionClearTrack_triggered()
{
    scene->clearTrack();
}

void MainWindow::on_actionLoadMapMeta_triggered()
{
    QString fileName = open(QString(), "Open Map Meta file", "Map meta files (*.txt)");
    if(fileName.isEmpty())
        return;
    loadMapMeta(fileName);
}

void MainWindow::on_actionSaveRkPts_triggered()
{
    QString fileName = save(QString(), "Save Route Key Points", "txt files (*.txt)");
    if(fileName.isEmpty())
        return;
    scene->saveKeyPoints(fileName);
}

void MainWindow::on_actionRestoreRkPts_triggered()
{
    QString fileName = open(QString(), "Open Map Meta file", "Map meta files (*.txt)");
    if(fileName.isEmpty())
        return;
    scene->restoreKeyPoints(fileName);
}

void MainWindow::on_lineColorLabel_colorChanged(QColor c)
{
    QList<QGraphicsItem *>	items = scene->selectedItems();
    QListIterator<QGraphicsItem *> i(items);
    while(i.hasNext()){
        dynamic_cast<MapObject *>(i.next())->setLineColor(c);
    }
    scene->update(/*scene->sceneRect()*/);
}

void MainWindow::on_fillColorLabel_colorChanged(QColor c)
{
    QList<QGraphicsItem *>	items = scene->selectedItems();
    QListIterator<QGraphicsItem *> i(items);
    while(i.hasNext()){
        dynamic_cast<MapObject *>(i.next())->setFillColor(c);
    }
    scene->update(/*scene->sceneRect()*/);
}

void MainWindow::on_nameEdit_textChanged(QString s)
{
    QList<QGraphicsItem *>	items = scene->selectedItems();
    if(items.size()){
        dynamic_cast<MapObject *>(items.front())->setName(s);
    }
}

void MainWindow::on_widthSpin_valueChanged(double v)
{
    QList<QGraphicsItem *>	items = scene->selectedItems();
    QListIterator<QGraphicsItem *> i(items);
    while(i.hasNext()){
        dynamic_cast<MapObject *>(i.next())->setLineWidth(v);
    }
    scene->update(/*scene->sceneRect()*/);
}

void MainWindow::prop1Combo_currentIndexChanged(int v)
{
    QList<QGraphicsItem *>	items = scene->selectedItems();
    if(items.size()){
        CharPoint * p = qgraphicsitem_cast<CharPoint *>(items.front());
        if(p){
            p->setProp1(v);
            scene->update();
        }
    }
}

void MainWindow::prop2Combo_currentIndexChanged(int v)
{
    QList<QGraphicsItem *>	items = scene->selectedItems();
    if(items.size()){
        CharPoint * p = qgraphicsitem_cast<CharPoint *>(items.front());
        if(p){
            p->setProp2(v);
        }
        Connection * c = qgraphicsitem_cast<Connection *>(items.front());
        if(c){
            c->setProp2(v);
        }
    }
}

void MainWindow::on_speedEdit_textEdited(QString s)
{
    QList<QGraphicsItem *>	items = scene->selectedItems();
    if(items.size()){
        CharPoint * p = qgraphicsitem_cast<CharPoint *>(items.front());
        if(p){
            p->setSpeed(s.toDouble());
        }
        Connection * c = qgraphicsitem_cast<Connection *>(items.front());
        if(c){
            c->setSpeed(s.toDouble());
        }
    }
}

void MainWindow::trackCombo_currentIndexChanged(int v)
{
    QList<QGraphicsItem *>	items = scene->selectedItems();
    if(items.size()){
        CharPoint * p = qgraphicsitem_cast<CharPoint *>(items.front());
        if(p){
            p->setTrackFlag(v);
        }
    }
}

void MainWindow::on_typeCombo_currentIndexChanged(int v)
{
    QList<QGraphicsItem *>	items = scene->selectedItems();
    if(items.size()){
        CharPoint * p = qgraphicsitem_cast<CharPoint *>(items.front());
        if(p){
            if(v != LaneCharPoint && v != LaneControlPoint){
                ui->typeCombo->setCurrentIndex(p->getType());
            }else{
                p->setType((PointType)v);
                scene->update();
            }
        }
    }
}

void MainWindow::selectionChangedHandler()
{
    disableAProp();
    QList<QGraphicsItem *>	items = scene->selectedItems();
    if(items.size()){
        MapObject *m = dynamic_cast<MapObject *>(items.front());
        ui->idEdit->setText(QString::fromStdString(std::to_string(m->id())));
        ui->nameEdit->setText(m->getName());
        ui->widthSpin->setValue(m->lineWidth());
        ui->lineColorLabel->setStyleSheet(tr("background-color: rgb(%1, %2, %3);").arg(m->lineColor().red()).arg(m->lineColor().green()).arg(m->lineColor().blue()));
        ui->fillColorLabel->setStyleSheet(tr("background-color: rgb(%1, %2, %3);").arg(m->fillColor().red()).arg(m->fillColor().green()).arg(m->fillColor().blue()));
        CharPoint * p = qgraphicsitem_cast<CharPoint *>(items.front());
        if(p){
            QPointF g = scene->mapToGlobal(p->scenePos());
            ui->xLabel->setText(QString::fromStdString(std::to_string(g.x())));
            ui->yLabel->setText(QString::fromStdString(std::to_string(g.y())));
            switch(p->getType())
            {
            case SegmentVertex:
            case IntersectionVertex:
            case SharedVertex:
                break;
            case LaneCharPoint:
            case LaneControlPoint:
                ui->typeCombo->setEnabled(true);
            case LaneLinePoint:
            case ConnectionControlPoint:
                enableAProp();
                ui->prop1Combo->setCurrentIndex(p->getProp1());
                ui->prop2Combo->setCurrentIndex(p->getProp2());
                ui->speedEdit->setText(QString::fromStdString(std::to_string(p->getSpeed())));
                ui->trackCombo->setCurrentIndex(p->getTrackFlag());
                break;
            default:
                break;
            }
            ui->typeCombo->setCurrentIndex(p->getType());
        }
        Connection * c = qgraphicsitem_cast<Connection *>(items.front());
        if(c){
            ui->prop2Combo->setEnabled(true);
            ui->speedEdit->setEnabled(true);
            ui->prop2Combo->setCurrentIndex(c->getProp2());
            ui->speedEdit->setText(QString::fromStdString(std::to_string(c->getSpeed())));
        }
    }
}

void MainWindow::initEnv()
{
    scene->setLogger(ui->logger);
    //init menu
    QActionGroup* editorGroup = new QActionGroup(this);
    editorGroup->addAction(ui->actionHand);
    editorGroup->addAction(ui->actionDrawLaneLine);
    editorGroup->addAction(ui->actionDrawLane);
    editorGroup->addAction(ui->actionCurveAjustor);
    editorGroup->addAction(ui->actionDrawSegment);
    editorGroup->addAction(ui->actionEditor);
    editorGroup->addAction(ui->actionAddKeyPoint);
    editorGroup->addAction(ui->actionAddConnection);
    editorGroup->addAction(ui->actionDrawIntersection);
    editorGroup->addAction(ui->actionSelectRoutePoints);
    editorGroup->addAction(ui->actionSpeedAdjustor);
    ui->actionHand->setChecked(true);

    //init comboBoxes
    ui->prop1Combo->insertItem(0, "Start");
    ui->prop1Combo->insertItem(1, "Intersection IN");
    ui->prop1Combo->insertItem(2, "Intersection OUT");
    ui->prop1Combo->insertItem(3, "Normal");
    ui->prop1Combo->insertItem(4, "Terminal Stop");
    ui->prop1Combo->insertItem(5, "Enter Parking Area");
    ui->prop1Combo->insertItem(6, "Out Parking Area");
    ui->prop1Combo->insertItem(7, "Parking Spot");

    ui->prop2Combo->insertItem(0, "Unkown");
    ui->prop2Combo->insertItem(1, "Straight Ahead");
    ui->prop2Combo->insertItem(2, "Right Turn");
    ui->prop2Combo->insertItem(3, "Left Turn");
    ui->prop2Combo->insertItem(4, "U-Turn");
    ui->prop2Combo->insertItem(5, "Traffic Sign");

    ui->trackCombo->insertItem(0, "Keep");
    ui->trackCombo->insertItem(1, "Normal");
    ui->trackCombo->insertItem(2, "Start");
    ui->trackCombo->insertItem(3, "I/0 building blocks");
    ui->trackCombo->insertItem(4, "U-Turn");
    ui->trackCombo->insertItem(5, "Queue wait");
    ui->trackCombo->insertItem(6, "Over take");
    ui->trackCombo->insertItem(7, "Pedestrain crossing road");
    ui->trackCombo->insertItem(8, "Road in constructing");
    ui->trackCombo->insertItem(9, "Away from defect car");
    ui->trackCombo->insertItem(10, "Left-turn avoid conflicting");
    ui->trackCombo->insertItem(11, "Off road");
    ui->trackCombo->insertItem(12, "Parking");
    ui->trackCombo->insertItem(13, "Follow Laneline");
    ui->trackCombo->insertItem(14, "No Map Match");

    ui->typeCombo->insertItem(0, "Stand Alone");
    ui->typeCombo->insertItem(1, "Segment Vertex");
    ui->typeCombo->insertItem(2, "Intersection Vertex");
    ui->typeCombo->insertItem(3, "Shared Vertex");
    ui->typeCombo->insertItem(4, "Lane CharPoint");
    ui->typeCombo->insertItem(5, "Lane Control Point");
    ui->typeCombo->insertItem(6, "Lane Line Point");
    ui->typeCombo->insertItem(7, "Connection Control Point");
    disableAProp();
    ui->speedEdit->setValidator(new QDoubleValidator(this));

    //signal & slots:
    connect(scene, SIGNAL(selectionChanged()), this, SLOT(selectionChangedHandler()));
    connect(ui->prop1Combo, SIGNAL(currentIndexChanged(int)), this, SLOT(prop1Combo_currentIndexChanged(int)));
    connect(ui->prop2Combo, SIGNAL(currentIndexChanged(int)), this, SLOT(prop2Combo_currentIndexChanged(int)));
    connect(ui->trackCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(trackCombo_currentIndexChanged(int)));
}

QString MainWindow::open(const QString &path, const QString &title, const QString &filter)
{
    QString fileName;
    if (path.isNull())
        fileName = QFileDialog::getOpenFileName(this, title,
                currentPath, filter);
    else
        fileName = path;

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.exists()) {
            QMessageBox::critical(this, title,
                           QString("Could not open file '%1'.").arg(fileName));
            return "";
        }
        return fileName;

//        if (!fileName.startsWith(":/")) {
//            currentPath = fileName;
//            setWindowTitle(tr("%1 - UgvMap").arg(currentPath));
//        }
    }
    return "";
}

QString MainWindow::save(const QString &path, const QString &title, const QString &filter)
{
    QString fileName;
    if (path.isNull())
        fileName = QFileDialog::getSaveFileName(this, title,
                currentPath, filter);
    else
        fileName = path;

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.exists()) {
            QMessageBox::StandardButton a = QMessageBox::question(this, title,
                           QString("File '%1' already exists, do you really want to OVERWRITE it?").arg(fileName)
                                  , QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);
            if (a == QMessageBox::No || a == QMessageBox::Escape)
                return "";
        }
        return fileName;

//        if (!fileName.startsWith(":/")) {
//            currentPath = fileName;
//            setWindowTitle(tr("%1 - UgvMap").arg(currentPath));
//        }
    }
    return "";
}

bool MainWindow::loadGridMap(const QString &fileName, bool isAddition)
{
    if(!isAddition){
        gridmap->load(fileName);
        gridmapItem = new QGraphicsPixmapItem(*gridmap);
        scene->addItem(gridmapItem);
        gridmapItem->setZValue(-2);
    }else{
        gridmapClip->load(fileName);
        gridmapClipItem = new QGraphicsPixmapItem(*gridmapClip);
        scene->addItem(gridmapClipItem);
        gridmapClipItem->setZValue(-1);
    }
    //temp
//    gridmapItem->setPos(10000, 10000);
    //end temp
//    scene->setSceneRect(gridmapItem->boundingRect());
//    resize(view->sizeHint() + QSize(0, menuBar()->height()));
    return true;
}

bool MainWindow::loadMapMeta(const QString &fileName, bool isAddition)
{
    std::ifstream ifs(fileName.toStdString());
    if(ifs){
        ifs >> xCenter >> yCenter >> width >> height;
//        //temp, just for handling MDF file
//        int oldCenterX = 10000 + width / 2;
//        int oldCenterY = 10000 + height/ 2;
//        width += 10000;
//        height += 10000;
//        int newCenterX = width / 2;
//        int newCenterY = height / 2;
//        double deltaX = (oldCenterX - newCenterX) * 0.2;
//        double deltaY = (oldCenterY - newCenterY) * 0.2;
//        xCenter -= deltaX;
//        yCenter -= deltaY;
//        //end temp
        if(!isAddition){
            scene->setGeometry(xCenter, yCenter, width, height);
        }
        ifs.close();
    }
    else{
        DLOG(INFO) << "Error reading meta file";
        return false;
    }
    return true;
}

void MainWindow::disableAProp()
{
    ui->typeCombo->setEnabled(false);
    ui->speedEdit->clear();
    ui->speedEdit->setEnabled(false);
    ui->prop2Combo->setEnabled(false);
    ui->xLabel->clear();
    ui->yLabel->clear();
    ui->prop1Combo->setEnabled(false);
    ui->trackCombo->setEnabled(false);
}

void MainWindow::enableAProp()
{
    ui->prop1Combo->setEnabled(true);
    ui->trackCombo->setEnabled(true);
    ui->prop2Combo->setEnabled(true);
    ui->speedEdit->setEnabled(true);
//    ui->typeCombo->setEnabled(true);//should be enabled only for Lane related points
}

void MainWindow::updateStatusBar()
{
    switch (scene->editorType) {
    case Editor:
        statusBar()->showMessage("Current edit status: Editor");
        break;
    case DrawSegment:
        statusBar()->showMessage("Current edit status: Draw Segment");
        break;
    case DrawLane:
        statusBar()->showMessage("Current edit status: Draw Lane");
        break;
    case DrawLaneLine:
        statusBar()->showMessage("Current edit status: Draw Lane Line");
        break;
    case AddPoint:
        statusBar()->showMessage("Current edit status: Add Key Point");
        break;
    case AddConnection:
        statusBar()->showMessage("Current edit status: Add Connection");
        break;
    case DrawIntersection:
        statusBar()->showMessage("Current edit status: Draw Intersection");
        break;
    case SelectRoute:
        statusBar()->showMessage("Current edit status: Select Route Points");
        break;
    case CurveAdjustor:
        statusBar()->showMessage("Current edit status: Curve Adjustor");
        break;
    case SpeedAdjustor:
        statusBar()->showMessage("Current edit status: Speed Adjustor");
        break;
    case Hand:
        statusBar()->showMessage("Current edit status: Hand");
        break;
    default:
        statusBar()->showMessage("Current edit status: Unknown");
        break;
    }
}

