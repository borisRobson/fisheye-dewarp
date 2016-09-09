#include "calibrate_panorama.h"

using namespace std;
using namespace cv;

Mat map_x1, map_y2;
int once;
calibrate_panorama::calibrate_panorama()
{
    once = 0;
}

Mat calibrate_panorama::do_warp(Mat img)
{
    once++;

    //calculate warp matrices, same for eacg frame so only perform once
    if (once == 1){
        int width = img.cols;
        int height = img.rows;

        int Cx = width/2;
        int Cy = height/2;

        int R1x = (width/2) + (width/10);
        int R1 = R1x - Cx;

        int R2x = width * 0.9;

        int R2 = R2x - Cx;

        float Wd = 2.0*((R2 + R1)/2)*CV_PI;
        float Hd = (R2 - R1);

        map_x1 = Mat::zeros(Size(Wd,Hd), CV_32F);
        map_y2 = Mat::zeros(Size(Wd,Hd), CV_32F);

        for(int y = 0; y < int(Hd - 1); y++){
            for(int x = 0; x < int(Wd - 1); x++){
                float r = (float(y)/float(Hd)) * (R2-R1)+R1;
                float theta = (float(x)/float(Wd))*2.0*CV_PI;
                float xS = Cx + r*sin(theta);
                float yS = Cy + r*cos(theta);
                map_x1.at<float>(y,x) = (xS);
                map_y2.at<float>(y,x) = (yS);
            }
        }
    }

    //warp image
    Mat remapped;
    remap(img, remapped,map_x1, map_y2, CV_INTER_LINEAR);

    //crop edges
    Rect crop(remapped.cols * 0.01, remapped.rows * 0.1, remapped.cols * 0.99, remapped.rows * 0.75 );
    Mat final = remapped(crop);

    return final;
}
