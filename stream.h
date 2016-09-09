#ifndef STREAM_H
#define STREAM_H

#include <QObject>
#include <QtCore>
#include <QDebug>
#include <QTime>
#include <QTimer>
#include <QFile>
#include <vector>
#include <string>
#include <stdio.h>
#include <QtGui>
#include <QtGui/QApplication>

#include "gst/gst.h"
#include "gst/app/gstappsink.h"
#include "gst/app/gstappsrc.h"
#include "glib-2.0/glib.h"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"

#include "mainwindow.h"

using namespace std;
using namespace cv;

class stream : public QObject
{
    Q_OBJECT
public:
    stream( );
    bool buildpipeline();
    void startstream( );


};

class Task: public QObject{
    Q_OBJECT
public:
    Task(QObject* parent=0) : QObject(parent){}
private:

public slots:
    void run();
signals:
    void finished();
};

class viewThread : public QThread
{
public:
    viewThread();
    void run();
    void showImage(Mat view);
};


#endif // STREAM_H
