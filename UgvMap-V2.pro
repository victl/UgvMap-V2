#-------------------------------------------------
#
# Project created by QtCreator 2015-10-14T21:36:56
#
#-------------------------------------------------

QT       += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UgvMap-V2
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    mapview.cpp \
    segment.cpp \
    charpoint.cpp \
    lane.cpp \
    connection.cpp \
    mapobject.cpp \
    controlpoint.cpp \
    mapscene.cpp \
    def.cpp \
    colorlabel.cpp \
    CoordiTran.cpp \
    num3dialog.cpp

HEADERS  += mainwindow.h \
    inc.h \
    mapview.h \
    segment.h \
    charpoint.h \
    lane.h \
    def.h \
    connection.h \
    mapobject.h \
    controlpoint.h \
    mapscene.h \
    colorlabel.h \
    CoordiTran.h \
    num3dialog.h

FORMS    += mainwindow.ui \
    num3dialog.ui

RESOURCES += \
    resources.qrc

qtHaveModule(opengl): QT += opengl

CONFIG += c++14
DEFINES += OFFLINE

LIBS += `pkg-config opencv --cflags --libs`\
        -lglog\
        -L/usr/local/boost/lib\
        -L/usr/local/opencv-2.4.9/lib\
       -lboost_system\
       -lboost_thread\
       -lboost_program_options\
 -L/home/victor/workspace/ugv/libs\
        -lhdlengine\
-llog4cxx\
-lrt\
-lpthread

INCLUDEPATH += /usr/include/opencv\
            /usr/include/opencv2\
#             /home/denggroup/Workspace/zoulu/HdlEngine\
#            /home/denggroup/Workspace/zoulu/ugv-share
/home/victor/workspace/ugv/HdlEngine\
/home/victor/workspace/ugv/ugv-share
