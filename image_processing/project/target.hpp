//
// Created by 이영인 on 2016. 5. 5..
//

#ifndef OPENCV_TARGET_H
#define OPENCV_TARGET_H

#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

#define LEFT 0
#define RIGHT 1

#define DRAW Scalar(0, 0, 255)
#define DRAW_THICK 1
#define LOWCOLOR Scalar(0, 200, 0)
#define UPCOLOR Scalar(19, 255, 255)

using namespace cv;
using namespace std;

class Target {
public:
    Target();
    void init(UMat f, bool color=false);
    bool find_square(UMat *sqr);
    bool is_square(vector<Point> c, Rect * rect);
    void found();
    void show();
private:
    UMat orig, cvt, draw;
    int dir;
    vector<Point> approx;
};

#endif