#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <vector>
#include <cstring>
#include <fstream>
#include <stdint.h>
#include <stdlib.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/bgsegm.hpp>
#include "opencv2/photo.hpp"
#include <opencv2/features2d.hpp>
#include "opencv2/xfeatures2d/nonfree.hpp"

using namespace std;
using namespace cv;

Mat src; Mat src_gray;
char *pic_name = 0;
int thresh = 54;
int max_thresh = 255;
RNG rng(12345);

#define PICTURE_DIR "../pictures/"

struct feature{
    
};

void extract_features(char *name);
void thresh_callback(int, void* );

int main() {
    
    const char * dir_pictures = PICTURE_DIR;
    
    DIR *dp;
    struct dirent *ep;
    dp = opendir(dir_pictures);
    if (dp != NULL) {
        while (ep = readdir(dp)) {
            size_t string_size = strlen(ep->d_name) + strlen(dir_pictures);
            char * name = (char *) malloc(string_size);
            strcpy(name, dir_pictures);
            strcpy(name + strlen(dir_pictures), ep->d_name);
            pic_name = ep->d_name;
            extract_features(name);
        }
        (void) closedir(dp);
    } else
        perror("Couldn't open the directory pictures");



   
    //read out files from valid list
    //loop for classify to recognize pictures
    //save result in big file with feature vector and class
    //train classifier on this data and save trained classifier in file classifier.xml
    
    
    return 0;
}

void extract_features(char *name) {
    src = imread(name);
    if (src.empty() == 0) {
        cvtColor(src, src_gray, CV_RGB2GRAY);
        blur(src_gray, src_gray, Size(3, 3));
        char *source_window = "Source";
        namedWindow(source_window, CV_WINDOW_AUTOSIZE);
        

        ///////////////////////////////////////
        Mat canny_output;
        vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;

        Mat hsv;
        cvtColor(src, hsv, COLOR_BGR2HSV);
        Mat channel[3];
        split(hsv, channel);
        threshold(channel[1], canny_output, thresh, 255, THRESH_BINARY);

        findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

        int winner = 0;
        int largest = 0;
        for (int i = 0; i < contours.size(); i++) {
            if (largest < contours[i].size()) {
                largest = contours[i].size();
                winner = i;
            }

        }

        double epsilon = 0.05*arcLength(contours[winner],true);
        approxPolyDP(contours[winner], contours[winner], 20.0, true);

        //Contour Features
        vector<Point> contour = contours[winner];
        RotatedRect boundRect = minAreaRect(contour);
        Point2f vertices[4];
        boundRect.points(vertices);
        

        double aspectRatio = ((double) boundRect.size.width) / boundRect.size.height;
        double area = contourArea(contour);
        double extend = area / (boundRect.size.width * boundRect.size.height); //good
        vector<Point> hull;
        convexHull(contour, hull, false);
        double hull_area = contourArea(hull);
        double solidity = area / hull_area; //good
        double equiv_diameter = sqrt(4 * area / 3.14159); //not bad
        vector<Point> temp;
        double trian_area = minEnclosingTriangle(contour, temp);
        double trian_extend = area/trian_area;//good
        Point2f center;
        float radius;
        minEnclosingCircle(contour, center, radius);
        double circle_extend = area/(3.1415*radius*radius);//good
        
        printf("%s %f %f %f %f %f %f %f %f \n", pic_name, aspectRatio, area, extend, hull_area, solidity, equiv_diameter, trian_extend, circle_extend);
        //moments
        //color histogram in HSV


        //printf("Contour Points: %ld\n", contours[winner].size());
        Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
        Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
        drawContours(drawing, contours, winner, color, 2, 8, hierarchy, 0, Point());
        for (int i = 0; i < 4; i++)
            line(drawing, vertices[i], vertices[(i + 1) % 4], Scalar(0, 255, 0));
        namedWindow("Contours", CV_WINDOW_AUTOSIZE);
        resize(drawing, drawing, Size(640, 480));

        imshow("Contours", drawing);
        resize(src, src, Size(640, 480));
        imshow(source_window, src);

        ////////////////////////////////////////////////////////






        //createTrackbar(" Canny thresh:", "Source", &thresh, max_thresh, thresh_callback);
        //thresh_callback(0, 0);
        waitKey(0);
    }
}
/*
void thresh_callback(int, void*) {
    Mat canny_output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    Mat hsv;
    cvtColor(src, hsv, COLOR_BGR2HSV);
    Mat channel[3];
    split(hsv, channel);
    threshold(channel[1], canny_output, thresh, 255, THRESH_BINARY);

    findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

    int winner = 0;
    int largest = 0;
    for (int i = 0; i < contours.size(); i++) {
        if (largest < contours[i].size()) {
            largest = contours[i].size();
            winner = i;
        }

    }

    approxPolyDP(contours[winner], contours[winner], 4.0, true);
    
    //Contour Features
    vector<Point> contour = contours[winner];
    Rect boundRect = boundingRect(contour);
    double aspectRatio = ((double)boundRect.width)/boundRect.height;
    double area = contourArea(contour);
    double extend = area/(boundRect.width*boundRect.height);
    vector<Point> hull;
    convexHull(contour, hull, false);
    double hull_area = contourArea(hull);
    double solidity = area/hull_area;
    double equiv_diameter = sqrt(4*area/3.14159);
    //printf("%s %f %f %f %f %f %f\n", pic_name, aspectRatio, area, extend, hull_area, solidity, equiv_diameter);
    //moments
    //color histogram in HSV
    
    
    //printf("Contour Points: %ld\n", contours[winner].size());
    Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
    Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
    drawContours(drawing, contours, winner, color, 2, 8, hierarchy, 0, Point());

    namedWindow("Contours", CV_WINDOW_AUTOSIZE);
    resize(drawing, drawing, Size(640, 480));

    imshow("Contours", drawing);
}

*/
