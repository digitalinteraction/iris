/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Low_Res_Worker.cpp
 * Author: tobias
 * 
 * Created on May 3, 2016, 10:44 AM
 */

#include "Low_Res_Worker.h"
#include "RaspiTex.h"
#include "tga.h"
#include <limits>



using namespace std;

Low_Res_Worker::Low_Res_Worker(Buffer *buffer) {
    buf = buffer;
    processing = 0;
    pMOG2 = createBackgroundSubtractorMOG2(100, 16, false);
    cnt = 0;
    learning = 0.05;
    previous = Mat::zeros(LOW_OUTPUT_Y, LOW_OUTPUT_X, CV_8UC4);
    //pMOG2 = bgsegm::createBackgroundSubtractorGMG();
}


Low_Res_Worker::~Low_Res_Worker() {
}


void Low_Res_Worker::run(){
    uint8_t *image;
    size_t image_size;
    int light;
    while(processing){
        if(buf->get(&image, &image_size, &light) == 0){
            if(image != 0 && image_size != 0){
                counter++;
                process_image(image, image_size, light);
                buf->release();
                
            }
        }
    }
}

void Low_Res_Worker::process_image(uint8_t *image, size_t image_size, int light) {
    //Mat is in format BGRA
    Mat img = convert(image, image_size);
    if (img.empty() == 0) {

        pMOG2->apply(img, mask, learning);
        cnt++;
        if (cnt == 60) {
            printf("Learning Phase done\n");
            learning = std::numeric_limits< double >::min();
        }
        
        Mat kernel = Mat::ones(3, 3, CV_8U);
        Mat cleaned;
        morphologyEx(mask, cleaned, MORPH_OPEN, kernel);
        
        vector<vector<Point> > contours;
        RNG rng(12345);
        findContours(cleaned, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
        Mat drawing = Mat::zeros(cleaned.size(), CV_8UC3);
        for (int i = 0; i < contours.size(); i++) {
            Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
            drawContours(drawing, contours, i, color, 2);
        }
        
        int cnt = 0;
        if(contours.size() > 0 && contours.size() < 10){
            for(int i = 0;i< contours.size();i++){
                int xmin = contours[i][0].x;
                int ymin = contours[i][0].y;
                int xmax = contours[i][0].x;
                int ymax = contours[i][0].y;
                for(int j=0;j<contours[i].size();j++){
                    if(contours[i][j].x < xmin){
                        xmin = contours[i][j].x;
                    }
                    if(contours[i][j].x > xmax){
                        xmax = contours[i][j].x;
                    }
                    if(contours[i][j].y < ymin){
                        ymin = contours[i][j].y;
                    }
                    if(contours[i][j].y > ymax){
                        ymax = contours[i][j].y;
                    }
                }
                if((xmax-xmin) > 20 && (ymax - ymin) > 20){
                    requests[cnt][0] = xmin;
                    requests[cnt][1] = ymin;
                    requests[cnt][2] = xmax;
                    requests[cnt][3] = ymax;
                    cnt++;
                }
                //printf("Quadrant: (%d %d), (%d %d)\n", xmin, ymin, xmax, ymax);
            }
        }
        if(cnt > 0){
            requests_pending = cnt;
        }
        
        ///////////////////////////////////////////////////////////////
        imshow("Background Separator", drawing);
        waitKey(30);
    } else {
        printf("Failed to convert camera image to Mat\n");
    }
}

Mat Low_Res_Worker::convert(uint8_t* image, size_t image_size) {
    if (image_size == (LOW_OUTPUT_X * LOW_OUTPUT_Y * 4)) {
        Mat mat_image(LOW_OUTPUT_Y, LOW_OUTPUT_X, CV_8UC4, (void*)image);
        return mat_image;
    }else{
        Mat null;
        return null;
    }
}