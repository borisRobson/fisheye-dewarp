#include "calibrate_square.h"

using namespace cv;
using namespace std;

int framecount;
Mat map_x, map_y;

calibrate_square::calibrate_square()
{
    framecount = 0;
}



Mat calibrate_square::getmatrixvalues(Mat img)
{
    framecount++;
    if(framecount == 1){
        float Ws = img.cols;
        float Hs = img.rows;
        float Wd = Ws*(4.5/3.0);
        float Hd = Hs*0.9;
        map_x = Mat::zeros(Size(Wd,Hd), CV_32F);
        map_y = Mat::zeros(Size(Wd,Hd), CV_32F);


        float hfovd = 150.0;
        float vfovd = 150.0;
        float vfov = (vfovd / 180.0) * CV_PI;
        float hfov = (hfovd / 180.0) * CV_PI;
        float vstart = ((180.0-vfovd)/180.00)*CV_PI/2.0;
        float hstart = ((180.0-hfovd)/180.00)*CV_PI/2.0;
        int count = 0;
        float xmax = sin(CV_PI/2.0)*cos(vstart);
        float xmin = sin(CV_PI/2)*cos(vstart + vfov);
        float xscale = xmax - xmin;
        float xoff = xscale / 2.0;
        float zmax = cos(hstart);
        float zmin = cos(hfov + hstart);
        float zscale = zmax - zmin;
        float zoff = zscale / 2.0;
        for(int y = 0; y < int(Hd); y++){
            for(int x = 0; x < int(Wd); x++){
                count++;
                float phi = vstart + (vfov*((float(x)/float(Wd))));
                float theta = hstart + (hfov*((float(y)/float(Hd))));
                float xp = ((sin(theta) * cos(phi))+xoff)/zscale;
                float zp = ((cos(theta)) + zoff)/ zscale;
                float xS = Ws - (xp*Ws);
                float yS = Hs - (zp*Hs);
                map_x.at<float>(y,x) = (xS);
                map_y.at<float>(y,x) = (yS);
            }
        }
    }

    Mat remapped;
    remap(img, remapped, map_x, map_y, CV_INTER_LINEAR);
    Rect crop(remapped.cols * 0.2, remapped.rows * 0.1, remapped.cols*0.6, remapped.rows * 0.8 );
    Mat final = remapped(crop);

    return final;
}
