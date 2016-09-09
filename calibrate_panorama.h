#ifndef CALIBRATE_PANORAMA_H
#define CALIBRATE_PANORAMA_H

#include <QObject>
#include <vector>

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "gst/gst.h"
#include "gst/app/gstappsink.h"
#include "gst/app/gstappsrc.h"
#include "glib-2.0/glib.h"

using namespace cv;
using namespace std;

class calibrate_panorama
{
public:
    calibrate_panorama();
    Mat do_warp(Mat img);
};

#endif // CALIBRATE_PANORAMA_H
