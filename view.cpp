#include <QtGui>
#include <QtGui/QApplication>
#include <QCoreApplication>
#include <QApplication>
#include <vector>
#include "view.h"

Rect ROIs[3];
bool roiSet[3];
Scalar colour;

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
    namedWindow("Dewarped");
    setMouseCallback("Dewarped", mouseCallback);

    Mat image = drawRect(img);

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
    //if rect has been drawn
    if(bottomrightx != 0 && bottomrighty != 0){
        Point p0(topleftx,toplefty);
        Point p1(bottomrightx,bottomrighty);

        //show rectangle on original image
        rectangle(img, p0,p1,colour);
        //show selected image in new window
        Rect ROI(p0,p1);

        if(roiSet[0] == true){
            ROIs[0] = ROI;
        }
        else if(roiSet[1] == true){
            ROIs[1] = ROI;
        }
        else{
            ROIs[2] = ROI;
        }

        Mat roi = img(ROI);
        resize(roi,roi,Size(300,300));
        namedWindow("roi");
        imshow("roi", roi);

    }else{
        if(roiSet[0] == true){
            ROIs[0] = Rect(Point(0,0), Point(0,0));
        }
        else if(roiSet[1] == true){
            ROIs[1] = Rect(Point(0,0), Point(0,0));
        }
        else{
            ROIs[2] = Rect(Point(0,0), Point(0,0));
        }
        destroyWindow("roi");
    }
    return img;
}

void view::selectView(int index)
{
    switch(index){
        case 0:{
            roiSet[0] = true;
            roiSet[1] = roiSet[2] = false;
            colour = Scalar(0,0,255);
            break;
        }
        case 1:{
            roiSet[1] = true;
            roiSet[0] = roiSet[2] = false;
            colour = Scalar(0,255,0);
            break;
        }
        case 2:{
            roiSet[2] = true;
            roiSet[1] = roiSet[0] = false;
            colour = Scalar(255,0,0);
            break;
        }
    }
}


