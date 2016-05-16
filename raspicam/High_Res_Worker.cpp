/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   High_Res_Worker.cpp
 * Author: tobias
 * 
 * Created on May 12, 2016, 4:12 PM
 */

#include "High_Res_Worker.h"
#include "tga.h"

using namespace std;


High_Res_Worker::High_Res_Worker(Buffer *buffer) {
    buf = buffer;
    cnt = 0;
    prev_group = 0;
    // Default parameters of ORB
    
int nfeatures=500;
    float scaleFactor=1.2f;
    int nlevels=8;
    int edgeThreshold=15; // Changed default (31);
    int firstLevel=0;
    int WTA_K=2;
    int scoreType=ORB::HARRIS_SCORE;
    int patchSize=31;
    int fastThreshold=20;

    detector = FastFeatureDetector::create();
    //detector = xfeatures2d::SURF::create();
}

High_Res_Worker::~High_Res_Worker() {
}

void High_Res_Worker::run(){
    while(processing){
        RASPITEX_PATCH *patch;
        uint8_t group;
        if(buf->get(&patch, &group) == 0){
            if(group != prev_group){
                cnt = 0;
            }
            printf("%d %d got patch %d %d %d\n", patch->width, patch->height, patch->size, group, cnt);

            
            find_features(patch, group);
            
            cnt++;
            prev_group = group;
            buf->release();
        }
    }
}

void High_Res_Worker::find_features(RASPITEX_PATCH *patch, uint8_t group) {
    Mat img = convert(patch);
    printf("Image:: %p %d %d\n", patch->buffer, patch->size, img.empty());
    if (img.empty() == 0) {
        //////////////////////////////////////////////////////////
        Mat hsv, mask;
        cvtColor(img, hsv, COLOR_BGR2HSV);
        Mat channel[3];
        split(hsv, channel);
        //channel[1];
        threshold(channel[1], mask, 40, 255, THRESH_BINARY);
        Mat kernel = Mat::ones(3, 3, CV_8U);
        Mat cleaned;
        morphologyEx(mask, cleaned, MORPH_OPEN, kernel);
        //////////////////////////////////////////////////////////
        
        //////////////////////////////////////////////////////////
        std::vector<vector<Point> > contours;
        RNG rng(12345);
        findContours(cleaned, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
        //////////////////////////////////////////////////////////
        
        //DRAW CONTOURS//////////////////////////////////////
        Mat drawing = Mat::zeros(cleaned.size(), CV_8UC3);
        for (int i = 0; i < contours.size(); i++) {
            Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
            drawContours(drawing, contours, i, color, 2);
        }
        //////////////////////////////////////////////////////////
        
        Mat part_img;
        img.copyTo(part_img, cleaned);
        
        //imshow("High res", mask);
        //waitKey(30);
        
        std::vector<KeyPoint> kp;
        detector->detect(part_img, kp);
        std::cout << "Found " << kp.size() << " Keypoints " << std::endl;
        Mat out, rgb;
        cvtColor(img, rgb, COLOR_RGBA2RGB);
        drawKeypoints(rgb, kp, out, Scalar::all(255));
        
        
        
        
        char tmp[] = "testing00a.png";
        char tmp2[] = "testing00b.png";

        tmp[7] = group + '0';
        tmp[8] = cnt + '0';
        tmp2[7] = group + '0';
        tmp2[8] = cnt + '0';
        
        imwrite(tmp2, drawing);
        imwrite(tmp, out);

        //FILE *fp = fopen(tmp, "wb");
        //write_tga(fp, patch->height, patch->width, patch->buffer, patch->size);
        //fclose(fp);
    }
}

Mat High_Res_Worker::convert(RASPITEX_PATCH *patch) {
        Mat mat_image(patch->width, patch->height, CV_8UC4, (void*)patch->buffer);
        return mat_image;
}