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
    
    classifier = cv::ml::RTrees::create();
    classifier->load<RTrees>("classifier.xml");
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
        //identify_object();
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
        
        Mat rgb;
        cvtColor(img, rgb, COLOR_BGRA2RGB);
        imwrite("patch.png", rgb);

        //FILE *fp = fopen("patch.tga", "wb");
        //write_tga(fp, patch->width, patch->height, patch->buffer, patch->size);
        //fclose(fp);
        
        //marker richtig plazieren... auch am rand
        //Mat marker = Mat::zeros(img.size(), CV_32SC1);
        Size img_size = img.size();
        printf("error code: %d %d\n", patch->active, patch->select);
        printf("patch size: %d %d at %d %d\n", img_size.width, img_size.height, patch->x, patch->y);
        //circle(marker, Point(img_size.width/2, img_size.height/2), 20, CV_RGB(1,1,1),-1);
        //circle(marker, Point(0,0), 5, CV_RGB(255,255,255), -1);
        //circle(marker, Point(0,img_size.height), 5, CV_RGB(255,255,255), -1);
        //circle(marker, Point(img_size.width,0), 5, CV_RGB(255,255,255), -1);
        //circle(marker, Point(img_size.width,img_size.height), 5, CV_RGB(255,255,255), -1);
        
        //watershed(rgb, marker);
        //imwrite("water.png", marker);
        //Mat mark = Mat::zeros(marker.size(), CV_8UC1);
        //marker.convertTo(mark, CV_8UC1);
        //bitwise_not(mark, mark);
        
        /*
        Mat blur, thres;
        GaussianBlur(rgb, blur, Size(5,5), 0,0);
        Scalar mean;
        Scalar stddev;
        meanStdDev(blur, mean, stddev);
        cout << "Mean " << mean << " stddev " << stddev << endl;
        inRange(blur, mean - stddev, mean + stddev, thres);
        //threshold(blur, thres, 0, 255,CV_THRESH_BINARY | CV_THRESH_OTSU);
        imwrite("water.png", thres);
         */
        /*Mat gray, thres;
        cvtColor(rgb, gray, CV_RGB2GRAY);
        threshold(gray, thres, 0, 255,CV_THRESH_BINARY | CV_THRESH_OTSU);
        imwrite("water.png", thres);*/
        
    
        Mat gray, thres;
        cvtColor(rgb, gray, CV_RGB2GRAY);
        //threshold(gray, thres, 40, 255,CV_THRESH_BINARY | CV_THRESH_OTSU);
        //Canny(gray, gray, 40, 120);
        //imwrite("water.png", gray);




//        vector<Mat> bgr_planes;
//        split(rgb, bgr_planes);
//        
//        
//        float range[] = {0,256};
//        const float *histRange = {range};
//        int buck = 32;
//        Mat b_hist, g_hist, r_hist;
//        
//        //Histogram
//        calcHist(&bgr_planes[0], 1, 0, thres, b_hist, 1, &buck, &histRange, true, true);
//        calcHist(&bgr_planes[1], 1, 0, thres, g_hist, 1, &buck, &histRange, true, true);
//        calcHist(&bgr_planes[2], 1, 0, thres, r_hist, 1, &buck, &histRange, true, true);
//        
//        //HuMoments
//        vector<vector<Point>> contours;
//        RNG rng(12345);
//        findContours(thres, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
//        double largest = 0;
//        int largest_index = 0;
//        for(int i = 0; i < contours.size(); i++){
//            double a = contourArea(contours[i], false);
//            if(a > largest){
//                largest = a;
//                largest_index = i;
//            }
//        }
//        Moments mu = moments(contours[largest_index], false);
//        double hu[7];
//        HuMoments(mu, hu);
//        
//        //contourArea
//        double area = contourArea(contours[largest_index]);
//        
//        //contour Perimeter
//        double perimeter = arcLength(contours[largest_index], true);
//        
//        cout << "Area: " << area << endl;
//        cout << "Perimeter: " << perimeter << endl;
//        cout << "HuMoments: " << hu[0] << " " << hu[1] << " " << hu[2] << " " << hu[3] << " "
//                << hu[4] << " " << hu[5] << " " << hu[6] << " " << hu[7] << endl;
//        cout << "Histogram R: " << endl;
//        for(int i = 0; i < 32; i++){
//            cout << " " << r_hist.at<float>(i);
//        }
//        cout << endl;
//        cout << "Histogram G: " << endl;
//        for(int i = 0; i < 32; i++){
//            cout << " " << g_hist.at<float>(i);
//        }
//        cout << endl;
//        cout << "Histogram B: " << endl;
//        for(int i = 0; i < 32; i++){
//            cout << " " << b_hist.at<float>(i);
//        }
//        cout << endl;
//        
//        
        //Mat gray;
        //cvtColor(rgb, gray, COLOR_BGR2GRAY);
        
        //resize(gray, gray, Size(256,192));
        
        //for(int i = 0; i < 8; i++){
        //send_to_server(&gray, 1, i);
        //}
        
        //channel[1];
        //threshold(channel[1], mask, 40, 255, THRESH_BINARY);
        //Mat kernel = Mat::ones(3, 3, CV_8U);
        //Mat cleaned;
        //morphologyEx(mask, cleaned, MORPH_OPEN, kernel);
        //////////////////////////////////////////////////////////
        
        /*//////////////////////////////////////////////////////////
        std::vector<vector<Point> > contours;
        RNG rng(12345);
        findContours(marker, contours, CV_RETR_FLOODFILL, CV_CHAIN_APPROX_SIMPLE);
        //////////////////////////////////////////////////////////
        
        printf("Contours size %d\n", contours.size());
        //DRAW CONTOURS//////////////////////////////////////
        Mat drawing = Mat::zeros(marker.size(), CV_8UC3);
        for (int i = 0; i < contours.size(); i++) {
            Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
            drawContours(drawing, contours, i, color, 2);
            printf("CON:: %d\n", contours[i].size());
        }
        //////////////////////////////////////////////////////////
        
        Mat part_img, inv_marker;
        bitwise_not(marker, inv_marker);
        inv_marker.convertTo(inv_marker, CV_8UC1);
        img.copyTo(part_img, inv_marker);
        
        //imshow("High res", mask);
        //waitKey(30);
        
        std::vector<KeyPoint> kp;
        detector->detect(part_img, kp);
        std::cout << "Found " << kp.size() << " Keypoints " << std::endl;
        Mat out, rgb2;
        cvtColor(img, rgb2, COLOR_RGBA2RGB);
        drawKeypoints(rgb2, kp, out, Scalar::all(255));
         */
        comm->save_to_file_image(&img);
        
        
        patch_packet *item = (patch_packet *) calloc(1, sizeof (patch_packet));
        item->feature = (feature_vector*) calloc(1, sizeof(feature_vector));
        item->feature->contour = new vector<Point>;
        for(int i = 0; i < 1000; i++){
            Point<int> pt= new Point(2,5);
            item->feature->contour->push_back(pt);
        }
        item->left = (patch_packet*)1;
        item->right = (patch_packet*)1;
        item->up = (patch_packet*)1;
        item->mac = nc->topo->mac;
        item->id = comm->file_cnt;
        
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

        comm->ask_neighbours(item);
        
        
        
        
        
        //char tmp[] = "testing00a.png";
        //char tmp2[] = "testing00b.png";
        
        /*char filename[30];
        snprintf(filename, 30, "pics/%d_%d_highres_out.png", group, cnt);
        imwrite(filename, out);
        memset(filename, 0, 30);
        
        snprintf(filename, 30, "pics/%d_%d_highres_draw.png", group, cnt);
        imwrite(filename, drawing);
        memset(filename, 0, 30);*/
        
        /*tmp[7] = group + '0';
        tmp[8] = cnt + '0';
        tmp2[7] = group + '0';
        tmp2[8] = cnt + '0';*/
        
        //imwrite(tmp2, out);
        //imwrite(tmp, drawing);

        //FILE *fp = fopen(tmp, "wb");
        //write_tga(fp, patch->height, patch->width, patch->buffer, patch->size);
        //fclose(fp);
        
        //rgb.release();
        //marker.release();
        //drawing.release();
        //part_img.release();
        //inv_marker.release();
        //out.release();
        //rgb2.release();
        comm->file_cnt++;

    }
    img.release();
    
}

Mat High_Res_Worker::convert(RASPITEX_PATCH *patch) {
        Mat mat_image(patch->height, patch->width, CV_8UC4, (void*)patch->buffer);
        return mat_image;
}

void High_Res_Worker::identify_object(patch_packet *item){
    if(((int)item->left) != 1 && ((int)item->right) != 1 && ((int)item->up) != 1 && ((int)item->down) != 1){
        //get all feature vector and classify
       
       combine_objects(item, item->left); 
       combine_objects(item, item->right); 
       combine_objects(item, item->up); 
       combine_objects(item, item->down); 

        
       //classifier->predict(item->feature);
        
        item->prev->next = item->next;
        item->next->prev = item->prev;
        if(item->left != 0){
            free(item->left);
        }
        if(item->right != 0){
            free(item->right);
        }
        if(item->up != 0){
            free(item->up);
        }
        if(item->down != 0){
            free(item->down);
        }
        free(item);
    }
}

void High_Res_Worker::combine_objects(patch_packet* dest, patch_packet* src, uint8_t dir) {
    
    if (src != 0 && src->feature != 0) {
        for (int i = 0; i < HISTOGRAM_SIZE; i++) {
            dest->feature->hist_r[i] += src->feature->hist_r[i];
            dest->feature->hist_g[i] += src->feature->hist_g[i];
            dest->feature->hist_b[i] += src->feature->hist_b[i];
        }
        dest->feature->contourArea += src->feature->contourArea;
        //double epsilon = 0.1*(dest->feature->contourPerimeter + src->feature->contourPerimeter);
        
    }
}


