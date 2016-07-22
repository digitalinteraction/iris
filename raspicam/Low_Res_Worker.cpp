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
//#include "Buffer.h"




using namespace std;

Low_Res_Worker::Low_Res_Worker(Packetbuffer *out, NetworkControl *nc, Buffer *images_in) {
    processing = 0;
    pMOG2 = createBackgroundSubtractorMOG2(100, 16, false);
    cnt = 0;
    learning = 0.05;
    previous = Mat::zeros(LOW_OUTPUT_Y, LOW_OUTPUT_X, CV_8UC4);
    new_low_buffer = 0;
    requests_pending = 0;
    nr_img = 0;
    id_cnt = 0;
    this->images_in = images_in;
    if (pthread_mutex_init(&buffer_lock, NULL) != 0)
    {
        printf("mutex init failed\n");
    }
    
    this->out = out;
    this->nc = nc;
    pos = 0;
    next_send = 0;
    this->first = 0;
    this->last = 0;
    
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
        /*if(new_low_buffer == 1){
            pthread_mutex_lock(&buffer_lock);
            counter++;
            
            process_image(low_patch.buffer, low_patch.size);

            nr++;
            nr_img++;
            free(low_patch.buffer);
            new_low_buffer = 0;
            pthread_mutex_unlock(&buffer_lock);
        }*/
        RASPITEX_PATCH *patch;
        uint8_t group;
        if(images_in->get(&patch, &group) == 0){
            counter++;
            //printf("size of patch %d pointer %p %d %d\n", patch->size, patch->buffer, patch->height, patch->width);
            process_image(patch->buffer, patch->size);
            nr++;
            nr_img++;
            //free(patch->buffer);
            //free(patch);
        }
    }
}

void Low_Res_Worker::process_image(uint8_t *image, size_t image_size) {
    //Mat is in format BGRA
    Mat img = convert(image, image_size);
    if (img.empty() == 0) {
        cleanup_list();
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
        vector<vector<Point> > *contours = new vector<vector<Point>>;
        RNG rng(12345);
        findContours(cleaned, *contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
        /////////////////////////////////////////////////////
        /*int movement = 10;
        if (prev1.empty() == 0 && prev2.empty() == 0 && prev3.empty() == 0) {
            Mat d1,d2,d3;
            absdiff(prev1, img, d1);
            absdiff(prev2, img, d2);
            absdiff(prev3, img, d3);
            Scalar means1(0,0,0,0);
            Scalar means2(0,0,0,0);
            Scalar means3(0,0,0,0);
            means1 = sum(d1);
            means2 = sum(d2);
            means3 = sum(d3);
            int sumsum1 = (int)(means1[0]+means1[1]+means1[2]);
            int sumsum2 = (int)(means2[0]+means2[1]+means2[2]);
            int sumsum3 = (int)(means3[0]+means3[1]+means3[2]);
            movement = (sumsum1 + sumsum2 + sumsum3)/100000;
            printf("Image similarity %d\n", movement);
        }*/

        //if(movement < 10){
        for(int i = 0; i < contours->size();i++){
            match_contours(contours);
        }
        //}
        //DRAW CONTOURS//////////////////////////////////////
        /*Mat drawing = Mat::zeros(cleaned.size(), CV_8UC3);
        for (int i = 0; i < contours->size(); i++) {
            Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
            drawContours(drawing, *contours, i, color, 2);
        }*/
        
        vector<vector<Point> > contours_list;
        Mat drawing = Mat::zeros(cleaned.size(), CV_8UC3);
        struct objects *item = first;
        int i = 0;
        while(item != 0){
            if(item->duration > 60){
                //Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
                //drawContours(drawing, (item->contour), i, color, 2);
                contours_list.push_back(*(item->contour));
                i++;
            }
            item = item->next;
        }
        
        for (int i = 0; i < contours_list.size(); i++) {
            Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
            drawContours(drawing, contours_list, i, color, 2);
        }
        
        
        /////////////////////////////////////////////////////
        //FIND MAX/MIN POINTS////////////////////////////////
        /*int cnt = 0;
        if(contours->size() > 0 && contours->size() < 10){
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
                    requests[cnt].fb = low_patch.fb;
                    requests[cnt].token = low_patch.token;
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
                    requests[cnt].x = (int) x;
                    requests[cnt].y = (int) y;
                    requests[cnt].height = (int) (width + 1.5);
                    requests[cnt].width = (int) (height + 1.5);
                    //add limits for request in ImageCapture
                    cnt++;
                }
            }
        }
        /////////////////////////////////////////////////////
        
        if(cnt > 0){
            //requests_pending = cnt;
        }*/
        
        ///////////////////////////////////////////////////////////////
        //imshow("H", channel[0]);
        //imshow("S", channel[1]);
        //imshow("V", channel[2]);
        //imshow("cleaned", cleaned);
        //imshow("Mask", drawing);
        //printf("Mat size: %d, channels: %d total_size: %d\n", cleaned.total(), cleaned.channels(), cleaned.total()*cleaned.elemSize());
        //send_to_server(low_patch.buffer, low_patch.size, 1, pos);
        //send_to_server(cleaned.data, cleaned.total()*cleaned.elemSize(), 1, pos);
        //Mat send_img(channel[1]);
        
        Mat gray, roi;
        //printf("size %d mask size %d", img.total(), cleaned.total());
        //img.copyTo(roi, cleaned);
        cvtColor(drawing, gray, COLOR_BGR2GRAY);
        if(next_send % 2 == 0){
        send_to_server(&gray, 1, pos);
        pos++;
        if (pos == 8) {
            pos = 0;
        }}
        next_send++;
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
    free(prev3.data);
    prev3 = prev2;
    prev2 = prev1;
    prev1 = img;
    //img.release();
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
        
    }
}

uint8_t Low_Res_Worker::match_contours(vector<vector<Point>> *contour) {

    vector<Point> *found = 0;
    struct objects *item = first;
    while (item != 0) {
        if (item->matched == 0) {

            double similarity = 1000.0;
            Point2f new_centroid = {0.0f,0.0f};
            double new_area = 0;
            int pos_elem = 0;

            for (int i = 0; i < contour->size(); i++) {
                //calculate centroid
                Moments mu = moments(contour->at(i), false);
                Point2f mc = Point2f(mu.m10 / mu.m00, mu.m01 / mu.m00);

                //calculate similarity of shape
                double sim_shapes = matchShapes(*(item->contour), contour->at(i), CV_CONTOURS_MATCH_I1, 0);

                float area = mu.m00;

                float diff_area = (item->area - area) / (item->area);
                Point2f diff_xy = item->centroid - mc;
                float diff_dist = sqrt(diff_xy.x * diff_xy.x + diff_xy.y * diff_xy.y);

                double total_sim = 0.003 * diff_dist + 0.01 * diff_area + sim_shapes;

                if (total_sim < similarity) {
                    similarity = total_sim;
                    found = contour->at(i);
                    new_centroid = mc;
                    new_area = area;
                    pos_elem = i;
                }
            }
            if (similarity <= 0.5) {
                printf("found match %d with similarity %d\n", item->id, similarity);
                item->contour = new vector<Point>;
                *item->contour = *found;
                contour->erase(pos_elem);
                item->area = new_area;
                item->centroid = new_centroid;
                item->expiring = 0;
                item->matched = 1;
                if (item->duration != 255) {
                    item->duration++;
                }
            }
        }
        item = item->next;
    }

    printf("size left %d\n", contour->size());
    for (int i = 0; i < contour->size(); i++) {
        Moments mu = moments(contour->at(i), false);
        Point2f mc = Point2f(mu.m10 / mu.m00, mu.m01 / mu.m00);

        //calculate similarity of shape
        float area = mu.m00;

        if (first == 0) {
            first = (struct objects *) malloc(sizeof (struct objects));
            first->contour = &contour->at(i);
            first->id = this->id_cnt++;
            first->expiring = 0;
            first->area = area;
            first->centroid = mc;
            printf("added item with id %d as first\n", first->id);
            last = first;
            first->next = 0;
            first->prev = 0;
        } else {
            item = (struct objects *) malloc(sizeof (struct objects));
            item->contour = &contour->at(i);
            item->id = this->id_cnt++;
            item->expiring = 0;
            item->area = area;
            item->centroid = mc;
            printf("added item with id %d\n", item->id);
            item->next = 0;
            item->prev = last;
            last->next = item;
            last = item;
        }
    }

    item = first;
    while (item != 0) {
        item->matched = 0;
        item = item->next;
    }

    return 0;
}


void Low_Res_Worker::cleanup_list() {
    struct objects *item = first;
    while (item != 0) {
        if (item->expiring == 60) {
            //printf("freeing item %p with id %d\n", item, item->id);
            if (item->prev == 0 && item->next == 0) {
                first = 0;
                last = 0;
            } else if (item->prev == 0) {
                first = item->next;
                first->prev = 0;
            } else if (item->next == 0) {
                item->prev->next = 0;
                last = item->prev;
            } else {
                item->prev->next = item->next;
                item->next->prev = item->prev;
            }
            //delete item->contour;
            free(item);
        } else {
            item->expiring++;
        }
        item = item->next;
    }

}