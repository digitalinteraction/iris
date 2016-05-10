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
        /* pMOG2->apply(img, mask, learning);
            
            cnt++;
            if(cnt == 60){
                //learning = 0;
                printf("Learning Phase done\n");
                learning = std::numeric_limits< double >::min();
            }*/
        if(light == 0){
           
            previous = img.clone();
        }else{
            if(previous.empty() == 0){
                absdiff(img, previous, mask);
            }
            
        }
                
        /*previous[prev_cnt] = mask.clone();
        prev_cnt++;
        if(prev_cnt == 10) prev_cnt = 0;
        
        double sum =0;
        for(int i=0;i<10;i++){
            if(!previous[i].empty()){
                //absdiff(previous[i], mask, out);
                sum += mean(previous[i] - mask)[0];
                //s += mean(out)[0];
            }
        }
        
        double p = mean(mask)[0];
        //interprete_params(p, sum);*/
        /////////////////////////////////////////////////////////////
        //Mat output;
        
        
        //int morph_size = 1;
        //Mat element = getStructuringElement( 2, Size( 2*morph_size + 1, 2*morph_size+1 ), Point( morph_size, morph_size ) );
        //morphologyEx(mask, output, 2, element);
        
        //fastNlMeansDenoising(mask, output, 3, 3, 5);
        
        
        
        /*vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;
        //Canny(mask, canny_output, 100, 200, 3);
        findContours(mask, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
        RNG rng(12345);

        
        Mat drawing = Mat::zeros(mask.size(), CV_8UC3);
        for (int i = 0; i < contours.size(); i++) {
            Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
            drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());
        }

        /// Show in a window
        namedWindow("Contours", CV_WINDOW_AUTOSIZE);
        imshow("Contours", drawing);*/
        ///////////////////////////////////////////////////////////////
        if(mask.empty() == 0 && light != 1){
            //Mat greyMat;
            //cvtColor(mask, greyMat, CV_BGR2GRAY);
            //greyMat *= 20;
            mask *= 10;
            imshow("Background Separator", mask);
            waitKey(30);
        }
        //std::this_thread::sleep_for(std::chrono::seconds(1));

        
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