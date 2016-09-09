#ifndef CALIBRATE_SQUARE_H
#define CALIBRATE_SQUARE_H

#include <QObject>

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "gst/gst.h"
#include "gst/app/gstappsink.h"
#include "gst/app/gstappsrc.h"
#include "glib-2.0/glib.h"

using namespace cv;
using namespace std;

class calibrate_square
{

public:
    calibrate_square();
    Mat getmatrixvalues(Mat img);
};

#endif //CALIBRATE_SQUARE_H
