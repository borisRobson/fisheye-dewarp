#-------------------------------------------------
#
# Project created by QtCreator 2016-09-09T08:53:05
#
#-------------------------------------------------

QT       += core gui

TARGET = fisheye-ptz
TEMPLATE = app

INCLUDEPATH += /usr/include/gstreamer-0.10 /usr/include/glib-2.0 /usr/lib/glib-2.0/include /usr/include/libxml2 /usr/lib/x86_64-linux-gnu/glib-2.0/include
LIBS +=  -L/usr/lib -lgstreamer-0.10 -lgstapp-0.10 -lgstbase-0.10 -lglib-2.0 -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lrt -lz -lxml2

INCLUDEPATH += /home/standby/doorentry/ltib/rootfs/usr/include

LIBS += -L/usr/local/lib -lopencv_core -lopencv_imgproc -lopencv_video -lopencv_highgui -lopencv_objdetect -lopencv_contrib -lopencv_flann
LIBS += -lopencv_features2d -lopencv_calib3d -lopencv_ml -lopencv_legacy -lopencv_photo -lopencv_gpu -lopencv_ocl -lopencv_nonfree

SOURCES += main.cpp\
        mainwindow.cpp \
    calibrate_square.cpp \
    calibrate_panorama.cpp \
    stream.cpp

HEADERS  += mainwindow.h \
    calibrate_square.h \
    calibrate_panorama.h \
    stream.h

FORMS    += mainwindow.ui
