#include <QtGui>
#include <QtGui/QApplication>
#include <QCoreApplication>
#include <QApplication>
#include <vector>
#include "view.h"

Rect ROIs[3];
bool roiSet[3];
Scalar colour[3];

using namespace cv;
using namespace std;

view::view()
{    

    for(int i = 0; i < 3; i++){
        ROIs[i] = Rect(Point(0,0), Point(0,0));
        roiSet[i] = false;
    }
}

int topleftx, toplefty, bottomrightx, bottomrighty;


void mouseCallback(int event, int x, int y, int flags, void* userdata)
{
    Q_UNUSED(flags);
    Q_UNUSED(userdata);

    //store top left co-ordinates
    if ( event == EVENT_LBUTTONDOWN){
        cout << "lbutton down, x: " << x << " y: " << y << endl;
        topleftx = x;
        toplefty = y;
    }
    //store bottom right coordinates
    else if ( event == EVENT_LBUTTONUP){
        cout << "lbutton up, x: " << x << " y: " << y << endl;
        bottomrightx = x;
        bottomrighty = y;
        //if top left == bottom right close image
        if(topleftx == bottomrightx || toplefty == bottomrighty){
            topleftx = toplefty = bottomrightx = bottomrighty = 0;     
        }
    }    
}

bool view::showImage(Mat img)
{
    //create window and set call back
    namedWindow("Dewarped");
    setMouseCallback("Dewarped", mouseCallback);

    //draw any ROI's
    Mat image = drawRect(img);

    //show the image
    imshow("Dewarped", image);

    //if 'esc' key then break
    char c = waitKey(10);
    if(c == 27){
        return false;
    }else{
        return true;
    }
}

Mat view::drawRect(Mat img)
{
    //check rect is within bounds
    if(topleftx < 0 || toplefty < 0 || bottomrightx < 0 || bottomrighty < 0 || topleftx > img.cols || bottomrightx > img.cols\
            || toplefty > img.rows || bottomrighty > img.rows){
        topleftx = toplefty = bottomrightx = bottomrighty = 0;
    }

    Point p0(topleftx,toplefty);
    Point p1(bottomrightx,bottomrighty);
    Rect ROI(p0,p1);

    char name[12];
    //determine which view is selected
    for(int i = 0; i < 3; i ++){
        if(roiSet[i] == true){
            ROIs[i] = ROI;
        }

        //if set
        if(ROIs[i].width != 0){
            //draw rectangle on original image
            //rectangle(img, ROIs[i], colour[i]);

            //create and display selcted region of interest in new window
            Mat roi = img(ROIs[i]);
            resize(roi,roi,Size(300,300));
            sprintf(name, "roi %d", i);
            imshow(name, roi);
        }
    }
    //return panoramic image with any/no ROI's drawn on
    return img;
}

void view::selectView(int index)
{
    //switch viewSet property and box colour
    switch(index){
        case 0:{
            roiSet[0] = true;
            roiSet[1] = roiSet[2] = false;
            colour[0] = Scalar(0,0,255);
            break;
        }
        case 1:{
            roiSet[1] = true;
            roiSet[0] = roiSet[2] = false;
            colour[1] = Scalar(0,255,0);
            break;
        }
        case 2:{
            roiSet[2] = true;
            roiSet[1] = roiSet[0] = false;
            colour[2] = Scalar(255,0,0);
            break;
        }
    }
}


