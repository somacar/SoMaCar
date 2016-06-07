//
// Created by 이영인 on 2016. 5. 5..
//

#ifndef OPENCV_TARGET_H
#define OPENCV_TARGET_H

#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <wiringSerial.h>

#define STOP 0
#define CENTER 1
#define LEFT 2
#define RIGHT 3

#define SLOW 3
#define MIN_DIST 50

#define ORIG_WIDTH 14
#define ORIG_DIS 30
#define ORIG_PIXEL 192
#define ORIG_F (ORIG_PIXEL * ORIG_DIS / ORIG_WIDTH)

#define LOWCOLOR Scalar(0, 200, 0)
#define UPCOLOR Scalar(19, 255, 255)

using namespace cv;
using namespace std;

class Target {
public:
    Target();

    void init(UMat u);

    bool find_square(UMat *sqr);

    bool is_square(vector<Point> c, Rect *rect);

    bool is_star(UMat u);

    void found(bool b);

    void serial();

    void show();

private:
    UMat orig, cvt, draw;
    int dist;
    vector<Point> approx;
};

#endif
