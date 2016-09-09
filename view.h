#ifndef VIEW_H
#define VIEW_H

#include <QObject>
#include <QtCore>
#include <QDebug>
#include <QFile>
#include <string>
#include <stdio.h>
#include <QApplication>


#include "gst/gst.h"
#include "gst/app/gstappsink.h"
#include "gst/app/gstappsrc.h"
#include "glib-2.0/glib.h"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"

using namespace cv;
using namespace std;

class view : public QObject
{    
    Q_OBJECT
public:
    view();
    Mat drawRect(Mat img);
    bool showImage(Mat img);

public slots:
    void selectView(int index);

};


#endif // VIEW_H
