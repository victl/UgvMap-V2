#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QtGui>

#include <QMainWindow>
#include <HdlEngine.h>

class MapScene;
class QGraphicsView;
class QFile;
class QGraphicsPixmapItem;
class MapView;
class QGraphicsItemGroup;


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected slots:
    void on_actionOpenHdl_triggered();
    void on_actionFindByHdl_triggered();
    void on_actionOpenGridMap_triggered();
    void on_actionOpenMixedMap_triggered();
    void on_actionImportTaskDescription_triggered();
    void on_actionAddMapClip_triggered();
    void on_actionSaveRndf_triggered();
    void on_actionExportFinalRoute_triggered();
    void on_actionSaveMergedMap_triggered();
    void on_actionWriteBoundaryBit_triggered();
    void on_actionHdlToTxt_triggered();
     void on_actionAddKeyPoint_toggled(bool flag);
     void on_actionDrawSegment_toggled(bool flag);
     void on_actionEditor_toggled(bool flag);
     void on_actionDrawLaneLine_toggled(bool flag);
     void on_actionDrawLane_toggled(bool flag);
     void on_actionHand_toggled(bool flag);
     void on_actionAddConnection_toggled(bool flag);
     void on_actionDrawIntersection_toggled(bool flag);
     void on_actionCurveAjustor_toggled(bool flag);
     void on_actionSpeedAdjustor_toggled(bool flag);
     void on_actionSelectRoutePoints_toggled(bool flag);
     void on_actionClearRoutePoints_triggered();
     void on_actionClearAll_triggered();
     void on_actionReArrangeIds_triggered();
     void on_actionFindPath_triggered();
     void on_actionImportTrack_triggered();
     void on_actionClearTrack_triggered();
     void on_actionLoadMapMeta_triggered();
     void on_actionSaveRkPts_triggered();
     void on_actionRestoreRkPts_triggered();

     //following slots set corresponding item propeties
     void on_lineColorLabel_colorChanged(QColor c);
     void on_fillColorLabel_colorChanged(QColor c);
     void on_nameEdit_textChanged(QString s);
     void on_widthSpin_valueChanged(double v);
     void prop1Combo_currentIndexChanged(int v);
     void prop2Combo_currentIndexChanged(int v);
     void on_speedEdit_textEdited(QString s);
     void trackCombo_currentIndexChanged(int v);
     void on_typeCombo_currentIndexChanged(int v);
     void selectionChangedHandler();

protected /*functions*/:


private /*functions*/:
     void initEnv();
    QString open(const QString &path, const QString &title, const QString &filter);
    QString save(const QString &path, const QString &title, const QString &filter);
    bool loadGridMap(const QString &fileName, bool isAddition = false);
    bool loadMapMeta(const QString &fileName, bool isAddition = false);
    void disableAProp();
    void enableAProp();
    void updateStatusBar();

private /*variable*/:
    //ui
    Ui::MainWindow *ui;
    MapScene *scene;
    MapView *view;
    QGraphicsPixmapItem *gridmapItem;

    //data
    victl::HdlEngine *hdlEngine;
    QString hdlFileName;
    QString gridMapFileName;
    QString mixedMapFileName;
    QString currentPath;
    QPixmap *gridmap;
    QPixmap *gridmapClip;
    QGraphicsPixmapItem *gridmapClipItem;
    double xCenter;
    double yCenter;
    int width;
    int height;
};

#endif // MAINWINDOW_H
