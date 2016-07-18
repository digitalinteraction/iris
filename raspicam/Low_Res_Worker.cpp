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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>




using namespace std;

Low_Res_Worker::Low_Res_Worker(Packetbuffer *out, NetworkControl *nc) {
    processing = 0;
    pMOG2 = createBackgroundSubtractorMOG2(100, 16, false);
    cnt = 0;
    learning = 0.05;
    previous = Mat::zeros(LOW_OUTPUT_Y, LOW_OUTPUT_X, CV_8UC4);
    new_low_buffer = 0;
    requests_pending = 0;
    nr_img = 0;
    if (pthread_mutex_init(&buffer_lock, NULL) != 0)
    {
        printf("mutex init failed\n");
    }
    
    this->out = out;
    this->nc = nc;
    pos = 0;
    
    //pMOG2 = bgsegm::createBackgroundSubtractorGMG();
}


Low_Res_Worker::~Low_Res_Worker() {
}


void Low_Res_Worker::run(){
    uint8_t *image;
    size_t image_size;
    int light;
    int nr = 0;
    while(processing){
        if(new_low_buffer == 1){
            pthread_mutex_lock(&buffer_lock);
            counter++;
            
            process_image(low_patch.buffer, low_patch.size);

            nr++;
            nr_img++;
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

        //imwrite("test.png", img);
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
                    if (contours[i][j].y > ymax) {
                        ymax = contours[i][j].y;
                    }
                }
                if ((xmax - xmin) > 10 && (ymax - ymin) > 10) {
                    //requests[cnt].fb = low_patch.fb;
                    //requests[cnt].token = low_patch.token;
                    //printf("Low Res: %d %d %d %d\n", xmin, ymin, xmax, ymax);
                    float factorx = (float) HIGH_OUTPUT_X / LOW_OUTPUT_X;
                    float factory = (float) HIGH_OUTPUT_Y / LOW_OUTPUT_Y;
                    float x = (xmin - 5) * factorx - factorx;
                    float y = (ymin - 5) * factory - factory;
                    if (x < 0) x = 0;
                    if (y < 0) y = 0;
                    float height = (ymax + 5) * factory + factory - y;
                    float width = (xmax + 5) * factorx + factorx - x;
                    if ((y + height) > HIGH_OUTPUT_Y) height = HIGH_OUTPUT_Y - y;
                    if ((x + width) > HIGH_OUTPUT_X) width = HIGH_OUTPUT_X - x;
                    //requests[cnt].x = (int) x;
                    //requests[cnt].y = (int) y;
                    //requests[cnt].height = (int) (width + 1.5);
                    //requests[cnt].width = (int) (height + 1.5);
                    //add limits for request in ImageCapture
                    cnt++;
                }
            }
        }
        /////////////////////////////////////////////////////
        
        if(cnt > 0){
            //requests_pending = cnt;
        }
        
        ///////////////////////////////////////////////////////////////
        imshow("H", channel[0]);
        imshow("S", channel[1]);
        imshow("V", channel[2]);
        //imshow("cleaned", cleaned);
        //imshow("Mask", drawing);
        //printf("Mat size: %d, channels: %d total_size: %d\n", cleaned.total(), cleaned.channels(), cleaned.total()*cleaned.elemSize());
        //send_to_server(low_patch.buffer, low_patch.size, 1, pos);
        //send_to_server(cleaned.data, cleaned.total()*cleaned.elemSize(), 1, pos);
        //Mat send_img(channel[1]);
        //send_to_server(&channel[1], 1, pos);
        pos++;
        if (pos == 8) {
            pos = 0;
        }

        //char filename[30];

        /*snprintf(filename, 30, "pics/%d_lowres_ch0.png", nr_img);
        imwrite(filename, channel[0]);
        memset(filename, 0, 30);
        
        snprintf(filename, 30, "pics/%d_lowres_ch1.png", nr_img);
        imwrite(filename, channel[1]);
        memset(filename, 0, 30);
        
        snprintf(filename, 30, "pics/%d_lowres_ch2.png", nr_img);
        imwrite(filename, channel[2]);
        memset(filename, 0, 30);*/
        
        /*snprintf(filename, 30, "pics/%d_lowres_cleaned.png", nr_img);
        imwrite(filename, cleaned);
        memset(filename, 0, 30);
        
        snprintf(filename, 30, "pics/%d_lowres_mask.png", nr_img);
        imwrite(filename, mask);
        memset(filename, 0, 30);

        waitKey(30);*/
        hsv.release();
        channel[0].release();
        channel[1].release();
        channel[2].release();
        kernel.release();
        cleaned.release();
        drawing.release();
    } else {
        printf("Failed to convert camera image to Mat\n");
    }
    img.release();
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

void Low_Res_Worker::send_to_server(Mat *img, uint8_t mode, uint8_t pos) {
    //printf("sending image to server %ld with pos %d \n", image_size, pos);
    if (nc->unrel->send_buf->getCnt() < 70) {
        
        if((img->total()*img->elemSize()) != 0){
            uint32_t addr;
            if (inet_aton("172.16.0.1", (in_addr *) & addr) == 0) {
                printf("inet_aton() failed\n");
            }

            uint32_t new_size = img->total()*img->elemSize();

            size_t part_size = new_size / 8;
            //int ret = 0;
            //for (int i = 0; i < 10; i++) {
                //ret = 0;

                uint32_t size = part_size + sizeof (struct low_res_header);
                struct low_res_header * header = (struct low_res_header *) malloc(size);
                memcpy((((unsigned char *) header) + sizeof (struct low_res_header)), (void*) (img->data + pos * part_size), part_size);
                header->mac = nc->topo->mac;
                header->port = IMAGE_PACKET;
                header->pos = pos;
                header->mac = nc->topo->mac;
                header->size = part_size;
                header->weight = nc->debug->get_weight();
                //ret = out->add(size, addr, (void*) header);
                out->add(size, addr, (void*) header);
                //if (ret != 0)
                //    i--;
                free(header);

            //}
            //printf("Send 10 packets: buffer length: %d\n", out->getCnt());
        }
        /*if (image_size == (LOW_OUTPUT_X * LOW_OUTPUT_Y * 3) && image_size < 500000) {
            uint8_t color = 0;
            if (mode == 0) {
                color = 3;
            } else {
                color = 1;
            }


            uint8_t *img = (uint8_t *) malloc(LOW_OUTPUT_X * LOW_OUTPUT_Y * color * sizeof (uint8_t));
            size_t new_size = 0;
            for (int i = 0; i < image_size; i++) {
                if (mode == 0) {
                    //if ((i + 1) % 4 != 0) {
                        img[new_size] = image[i];
                        new_size++;
                    //}
                } else {
                    if(i%3 == 0){
                        img[new_size] = image[i]/3 + image[i+1]/3 + image[i+2]/3;
                        new_size++;
                    }
                }
            }
            uint32_t addr;
            if (inet_aton("172.16.0.1", (in_addr *) & addr) == 0) {
                printf("inet_aton() failed\n");
            }

            new_size = LOW_OUTPUT_X * LOW_OUTPUT_Y * color * sizeof (uint8_t);

            size_t part_size = new_size / 8;
            //int ret = 0;
            //for (int i = 0; i < 10; i++) {
                //ret = 0;

                uint32_t size = part_size + sizeof (struct low_res_header);
                struct low_res_header * header = (struct low_res_header *) malloc(size);
                memcpy((((unsigned char *) header) + sizeof (struct low_res_header)), (void*) (img + pos * part_size), part_size);
                header->mac = nc->topo->mac;
                header->port = IMAGE_PACKET;
                header->pos = pos;
                header->mac = nc->topo->mac;
                header->size = part_size;
                header->weight = nc->debug->get_weight();
                //ret = out->add(size, addr, (void*) header);
                out->add(size, addr, (void*) header);
                //if (ret != 0)
                //    i--;
                free(header);

            //}
            printf("Send 10 packets: buffer length: %d\n", out->getCnt());
        
        
        }
         * */
    }
}