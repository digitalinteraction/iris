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
#include "tga.h"
#include <limits>



using namespace std;

Low_Res_Worker::Low_Res_Worker() {
    processing = 0;
    pMOG2 = createBackgroundSubtractorMOG2(100, 16, false);
    cnt = 0;
    learning = 0.05;
    previous = Mat::zeros(LOW_OUTPUT_Y, LOW_OUTPUT_X, CV_8UC4);
    new_low_buffer = 0;
    requests_pending = 0;
    if (pthread_mutex_init(&buffer_lock, NULL) != 0)
    {
        printf("mutex init failed\n");
    }
    
    //pMOG2 = bgsegm::createBackgroundSubtractorGMG();
}


Low_Res_Worker::~Low_Res_Worker() {
}


void Low_Res_Worker::run(){
    uint8_t *image;
    size_t image_size;
    int light;
    while(processing){
        if(new_low_buffer == 1){
            pthread_mutex_lock(&buffer_lock);
            counter++;
            process_image(low_patch.buffer, low_patch.size);
            free(low_patch.buffer);
            new_low_buffer = 0;
            pthread_mutex_unlock(&buffer_lock);
        }
    }
}

void Low_Res_Worker::process_image(uint8_t *image, size_t image_size) {
    //Mat is in format BGRA
    Mat img = convert(image, image_size);
    if (img.empty() == 0) {

        //BACKGROUND SUBSTRACTOR/////////////////////////////
        /*pMOG2->apply(img, mask, learning);
        cnt++;
        if (cnt == 60) {
            printf("Learning Phase done\n");
            learning = std::numeric_limits< double >::min();
        }*/
        /////////////////////////////////////////////////////
        Mat hsv;
        cvtColor(img, hsv, COLOR_BGR2HSV);
        Mat channel[3];
        split(hsv, channel);
        //channel[1];
        threshold(channel[1], mask, 40, 255, THRESH_BINARY);
                
        //CLEANING UP////////////////////////////////////////
        Mat kernel = Mat::ones(3, 3, CV_8U);
        Mat cleaned;
        morphologyEx(mask, cleaned, MORPH_OPEN, kernel);
        
        
        /////////////////////////////////////////////////////
        
        //GET SHAPE//////////////////////////////////////////
        vector<vector<Point> > contours;
        RNG rng(12345);
        findContours(cleaned, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
        /////////////////////////////////////////////////////
        
        //DRAW CONTOURS//////////////////////////////////////
        Mat drawing = Mat::zeros(cleaned.size(), CV_8UC3);
        for (int i = 0; i < contours.size(); i++) {
            Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
            drawContours(drawing, contours, i, color, 2);
        }
        /////////////////////////////////////////////////////
        
        //FIND MAX/MIN POINTS////////////////////////////////
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
                if((xmax-xmin) > 10 && (ymax - ymin) > 10){
                    requests[cnt].fb = low_patch.fb;
                    requests[cnt].token = low_patch.token;
                    float factorx = (float)HIGH_OUTPUT_X/LOW_OUTPUT_X;
                    float factory = (float)HIGH_OUTPUT_Y/LOW_OUTPUT_Y;
                    float x = (xmin-5)*factorx - factorx;
                    float y = (ymin-5)*factory - factory;
                    if(x < 0) x = 0;
                    if(y < 0) y = 0;
                    float height = (ymax+5)*factory + factory - y;
                    float width = (xmax+5)*factorx + factorx - x;
                    if((y + height) > HIGH_OUTPUT_Y) height = HIGH_OUTPUT_Y - y;
                    if((x + width) > HIGH_OUTPUT_X) width = HIGH_OUTPUT_X - x;
                    requests[cnt].x = (int)x;
                    requests[cnt].y = (int)y;
                    requests[cnt].height = (int)(height+0.5);
                    requests[cnt].width = (int)(width+0.5);
                    //add limits for request in ImageCapture
                    cnt++;
                }
            }
        }
        /////////////////////////////////////////////////////
        
        if(cnt > 0){
            requests_pending = cnt;
        }
        
        ///////////////////////////////////////////////////////////////
        imshow("H", channel[0]);
        imshow("S", channel[1]);
        imshow("V", channel[2]);
        imshow("cleaned", cleaned);
        imshow("Mask", drawing);

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