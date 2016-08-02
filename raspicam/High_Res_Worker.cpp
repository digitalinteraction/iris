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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

#include "Buffer.h"

#ifdef DEBUG_HIGH_RES
#define deb_printf(fmt, args...) fprintf(stderr, "HIGH_RES_WORKER: %d:%s(): " fmt, __LINE__, __func__, ##args)
#else
#define deb_printf(fmt, args...)
#endif

const char * const object_names[] = {"NOTHING", "CARROT", "CUCUMBER", "PEACH", "APPLE"};

using namespace std;


High_Res_Worker::High_Res_Worker(Buffer *buffer, Packetbuffer *out_buf, Packetbuffer *in_buf, NetworkControl *nc) {
    buf = buffer;
    cnt = 0;
    prev_group = 0;
    out = out_buf;
    in = in_buf;
    this->nc = nc;
    pos = 0;
    id = 0;
    
    first = 0;
    last = 0;
    comm = new CommImage(nc);
    
    //classifier = cv::ml::RTrees::create();
    //classifier->load<cv::ml::RTrees>("/home/pi/cutting_board/raspicam/build/classifier.xml");
    classifier = Algorithm::load<cv::ml::RTrees>("/home/pi/cutting_board/raspicam/build/classifier.xml");
    if(classifier->empty()){
        printf("error loading classifier\n");
    }else{
        printf("classifier loaded\n");
    }
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
            //printf("%d %d got patch %d %d %d\n", patch->width, patch->height, patch->size, group, cnt);

            
            find_features(patch, group);
            free(patch->buffer);
            free(patch);
            
            cnt++;
            prev_group = group;
            
        }
        
        comm->check_recv_buffer(first);

        patch_packet *item = first;
        while (item != 0) {
            int32_t res = -1;
            if (item->state != 1) {
                res = identify_object(item);
            }
            if (res != -1) {
                item->prev->next = item->next;
                item->next->prev = item->prev;
                if (((int) item->left) != 0) {
                    free(item->left);
                }
                if (((int) item->right) != 0) {
                    free(item->right);
                }
                if (((int) item->up) != 0) {
                    free(item->up);
                }
                if (((int) item->down) != 0) {
                    free(item->down);
                }
                patch_packet *old_item = item;
                item = item->next;
                free(old_item);
            } else {
                item = item->next;
            }
        }
    }
}

void High_Res_Worker::find_features(RASPITEX_PATCH *patch, uint8_t group) {
    Mat img = convert(patch);
    //printf("Image:: %p %d %d\n", patch->buffer, patch->size, img.empty());
    if (img.empty() == 0) {
        

        //////////////////////////////////////////////////////////
        //Mat hsv, mask;
        //cvtColor(img, hsv, COLOR_BGR2HSV);
        //Mat channel[3];
        //split(hsv, channel);
        
        imwrite("patch2.png", img);
        
        Mat hsv, rgb;
        cvtColor(img, rgb, CV_RGBA2RGB);
        cvtColor(rgb, hsv, CV_RGB2HSV);
        Mat save_img;
        cvtColor(hsv, save_img, CV_HSV2BGR);
        imwrite("patch.png", save_img);
        Mat channel[3];
        split(hsv, channel);
        comm->save_to_file_image(&rgb);

        
        Size img_size = img.size();
        deb_printf("error code: %d %d\n", patch->active, patch->select);
        deb_printf("patch size: %d %d at %d %d\n", img_size.width, img_size.height, patch->x, patch->y);
        
        Mat gray, thres;
        cvtColor(rgb, gray, CV_RGB2GRAY);
        
        threshold(channel[1], thres, 50, 255, THRESH_BINARY);
        imwrite("mask.png", thres);
        
        vector<Vec4i> hierarchy;
        vector<vector<Point> > contours;
        findContours(thres, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
        int winner = 0;
        int largest = 0;
        for (int i = 0; i < contours.size(); i++) {
            if (largest < contours[i].size()) {
                largest = contours[i].size();
                winner = i;
            }
        }
        double epsilon = 0.0035 * arcLength(contours[winner], true);
        vector<Point> *contour = new vector<Point>;
        approxPolyDP(contours[winner], *contour, epsilon, true);
        
        RNG rng(12345);
        Mat cont_img = thres;
        Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
        drawContours(cont_img, vector<vector<Point> >(1,*contour), -1, color, 1, 8);
        imwrite("cont_img.png", cont_img);
        
        
        //Histogram
        float range[] = {0, 256};
        const float *histRange = {range};
        int buck = HISTOGRAM_SIZE;
        Mat h_hist, s_hist, v_hist;
        Ptr<CLAHE> clahe = cv::createCLAHE();
        clahe->setClipLimit(4);
        Mat lum_channel;
        clahe->apply(channel[2], lum_channel);
        calcHist(&channel[0], 1, 0, thres, h_hist, 1, &buck, &histRange, true, true);
        calcHist(&channel[1], 1, 0, thres, s_hist, 1, &buck, &histRange, true, true);
        calcHist(&lum_channel, 1, 0, thres, v_hist, 1, &buck, &histRange, true, true);

        
        cout << "Histogram H: " << h_hist<< endl;
        cout << "Histogram S: " << s_hist<< endl;
        cout << "Histogram V: " << v_hist<< endl;

        //normalize(r_hist, r_hist, 0, 255.0, NORM_MINMAX, -1, Mat());
        //normalize(g_hist, g_hist, 0, 255.0, NORM_MINMAX, -1, Mat());
        //normalize(b_hist, b_hist, 0, 255.0, NORM_MINMAX, -1, Mat());
        
        patch_packet *item = (patch_packet *) calloc(1, sizeof (patch_packet));
        item->feature = (feature_vector*) calloc(1, sizeof(feature_vector));
        item->feature->contour = contour;
        /*for(int i = 0; i < 50; i++){
            Point *pt = new Point(i,i);
            item->feature->contour->push_back(*pt);
        }*/
        for(int i = 0; i < HISTOGRAM_SIZE; i++){
            item->feature->hist_h[i] = h_hist.at<float>(i);
            item->feature->hist_s[i] = s_hist.at<float>(i);
            item->feature->hist_v[i] = v_hist.at<float>(i);
        }
        for(int i = 0; i<HISTOGRAM_SIZE; i++){
            printf("HISTO: %f %f %f\n", item->feature->hist_h[i], item->feature->hist_s[i],item->feature->hist_v[i]);
        }
        
        
        item->left = (patch_packet*)patch->left;
        item->right = (patch_packet*)patch->right;
        item->up = (patch_packet*)patch->up;
        item->down = (patch_packet*)patch->down;
        item->mac = nc->topo->mac;
        item->id = comm->file_cnt;
        deb_printf("size of item: %d %d\n", sizeof(patch_packet), sizeof(feature_vector));
        deb_printf("item %p\n", item);
        comm->ask_neighbours(item);

        //fill item with data from RASPIPATCH
        if (first == 0) {
            first = item;
            last = first;
            first->next = 0;
            first->prev = 0;
        } else {
            item->next = 0;
            item->prev = last;
            last->next = item;
            last = item;
        }
        comm->file_cnt++;

    }
    img.release();
    
}

Mat High_Res_Worker::convert(RASPITEX_PATCH *patch) {
        Mat mat_image(patch->height, patch->width, CV_8UC4, (void*)patch->buffer);
        return mat_image;
}

int32_t High_Res_Worker::identify_object(patch_packet *item) {
    if (((int) item->left) != 1 && ((int) item->right) != 1 && ((int) item->up) != 1 && ((int) item->down) != 1) {
        //get all feature vector and classify

        combine_objects(item, item->left, LEFT_SIDE);
        combine_objects(item, item->right, RIGHT_SIDE);
        combine_objects(item, item->up, UP_SIDE);
        combine_objects(item, item->down, DOWN_SIDE);

        //normalize histograms
        vector<Point> *contour = item->feature->contour;
        RotatedRect boundRect = minAreaRect(*contour);
        Point2f vertices[4];
        boundRect.points(vertices);
        double area = contourArea(*contour);
        double extend = area / (boundRect.size.width * boundRect.size.height); //good
        vector<Point> hull;
        convexHull(*contour, hull, false);
        double hull_area = contourArea(hull);
        double solidity = area / hull_area; //good
        double equiv_diameter = sqrt(4 * area / 3.14159); //not bad
        vector<Point> temp;
        double trian_area = minEnclosingTriangle(*contour, temp);
        double trian_extend = area / trian_area; //good
        Point2f center;
        float radius;
        minEnclosingCircle(*contour, center, radius);
        double circle_extend = area / (3.1415 * radius * radius); //good

        //HuMoments
        Moments mu = moments(*contour, false);
        double hu[7];
        HuMoments(mu, hu);
        
        float *final_vector = (float*)malloc(sizeof(float)*(7+5+3*HISTOGRAM_SIZE));
        for(int i = 0; i < 7; i++){
            final_vector[i] = (float)(1000*hu[i]);
        }
        final_vector[7] = (float)(1000*extend);
        final_vector[8] = (float)(1000*solidity);
        final_vector[9] = (float)(equiv_diameter);
        final_vector[10] = (float)(1000*trian_extend);
        final_vector[11] = (float)(1000*circle_extend);
        memcpy(&final_vector[12], item->feature->hist_h, sizeof(float)*HISTOGRAM_SIZE);
        memcpy(&final_vector[12 + HISTOGRAM_SIZE], item->feature->hist_s, sizeof(float)*HISTOGRAM_SIZE);
        memcpy(&final_vector[12 + HISTOGRAM_SIZE*2], item->feature->hist_v, sizeof(float)*HISTOGRAM_SIZE);

        Mat features(1, 7+5+3*HISTOGRAM_SIZE, CV_32FC1);
        for(int i = 0; i < 7+5+3*HISTOGRAM_SIZE; i++){
            features.at<float>(0, i) = final_vector[i];
            //printf("%f\n", item->final_vector[i]);
        }
        cout << features << endl;
        float result = classifier->predict(features);
        int object = (int)floor(result+0.5);
        printf("Result of classifier: %s %f %d\n", object_names[object], result, object);
        
        return object;
        //do something with result
    }
    return -1;
}

void High_Res_Worker::combine_objects(patch_packet* dest, patch_packet* src, uint8_t dir) {

    if (src != 0 && src->feature != 0) {
        for (int i = 0; i < HISTOGRAM_SIZE; i++) {
            dest->feature->hist_h[i] += src->feature->hist_h[i];
            dest->feature->hist_s[i] += src->feature->hist_s[i];
            dest->feature->hist_v[i] += src->feature->hist_v[i];
        }

        for (int i = 0; i < src->feature->contour->size(); i++) {
            Point pt = src->feature->contour->at(i);
            switch (dir) {
                case LEFT_SIDE:
                    pt.x = pt.x - HIGH_OUTPUT_X;
                    break;
                case RIGHT_SIDE:
                    pt.x = pt.x + HIGH_OUTPUT_X;
                    break;
                case UP_SIDE:
                    pt.y = pt.y + HIGH_OUTPUT_Y;
                    break;
                case DOWN_SIDE:
                    pt.y = pt.y - HIGH_OUTPUT_Y;
                    break;
            }
            dest->feature->contour->push_back(pt);
        }
    }
}


