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



using namespace std;

Low_Res_Worker::Low_Res_Worker(Buffer *buffer) {
    buf = buffer;
    processing = 0;
    pMOG2 = createBackgroundSubtractorMOG2(500, 16, false);
}


Low_Res_Worker::~Low_Res_Worker() {
}


void Low_Res_Worker::run(){
    uint8_t *image;
    size_t image_size;
    while(processing){
        if(buf->get(&image, &image_size) == 0){
            if(image != 0 && image_size != 0){
                counter++;
                process_image(image, image_size);
                buf->release();
                
            }
        }
    }
}

void Low_Res_Worker::process_image(uint8_t *image, size_t image_size){
    //Mat is in format BGRA
    Mat img = convert(image, image_size);
    
    if(img.empty() == 0){
        pMOG2->apply(img, mask, 0.01);
        imshow("Background Separator", mask);
        waitKey( 30 );
    }else{
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