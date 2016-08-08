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

CommImage *CommImage::static_call = 0;


#ifdef DEBUG_HIGH_RES
#define deb_printf(fmt, args...) fprintf(stderr, "HIGH_RES_WORKER: %d:%s(): " fmt, __LINE__, __func__, ##args)
#else
#define deb_printf(fmt, args...)
#endif

#define EPSILON 1.0

const char * const object_names[] = {"NOTHING", "CARROT", "CUCUMBER", "PEACH", "APPLE"};

using namespace std;


High_Res_Worker::High_Res_Worker(Buffer *buffer, Packetbuffer *out_buf, Packetbuffer *in_buf, NetworkControl *nc, Buffer *class_out) {
    buf = buffer;
    cnt = 0;
    prev_group = 0;
    out = out_buf;
    in = in_buf;
    this->nc = nc;
    pos = 0;
    id = 0;
    this->class_out = class_out;
    first = 0;
    last = 0;
    this->comm = new CommImage(nc);
    
    //classifier = cv::ml::RTrees::create();
    //classifier->load<cv::ml::RTrees>("/home/pi/cutting_board/raspicam/build/classifier.xml");
    classifier = Algorithm::load<cv::ml::RTrees>("classifier.xml");
    if(classifier->empty()){
        printf("error loading classifier\n");
    }else{
        printf("classifier loaded\n");
    }
    
    surf = xfeatures2d::SURF::create( 200 );
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
            deb_printf("finished finding features\n");
            free(patch->buffer);
            free(patch);
            deb_printf("found features and freed patch\n");
            cnt++;
            prev_group = group;
            
        }
        
        comm->check_recv_buffer(first);
        comm->match_recv_list(first);
        check_objects(first);
        
        patch_packet *item = first;
        while (item != 0) {
            int32_t res = -1;
            if (item->state != 1) {
                deb_printf("identifying object %p\n", item);
                res = identify_object(item);
                deb_printf("end identifying %d\n", res);
            }
            if (res != -1) {
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
                deb_printf("freeing items %p %p %p %p\n", item->left, item->right, item->up, item->down);
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
        //0.0035
        double epsilon = 0.002 * arcLength(contours[winner], true);
        vector<Point> *contour = new vector<Point>;
        approxPolyDP(contours[winner], *contour, epsilon, true);
        
        RNG rng(12345);
        Mat cont_img = thres;
        Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
        drawContours(cont_img, vector<vector<Point> >(1,*contour), -1, color, 1, 8);
        imwrite("cont_img.png", cont_img);
        
        Mat rg(rgb.size(), rgb.type());

        for (int i = 0; i < rgb.rows; i++) {
            for (int j = 0; j < rgb.cols; j++) {
                Vec3f intensity = rgb.at<Vec3b>(i, j);
                float blue = intensity.val[0];
                float green = intensity.val[1];
                float red = intensity.val[2];
                double sum = red + blue + green;
                double r = red / sum;
                double g = green / sum;
                double b = blue / sum;
                //printf("%f %f %f %f\n", sum, r, g, b);
                rg.data[rg.step[0] * i + rg.step[1] * j + 0] = (b * 255);
                rg.data[rg.step[0] * i + rg.step[1] * j + 1] = (g * 255);
                rg.data[rg.step[0] * i + rg.step[1] * j + 2] = (r * 255);
            }
        }

        //Histogram
        float range[] = {0, 255};
        const float *histRange = {range};
        int buck = HISTOGRAM_SIZE;
        Mat h_hist, s_hist, v_hist;
        //Ptr<CLAHE> clahe = cv::createCLAHE();
        //clahe->setClipLimit(4);
        //Mat lum_channel;
        //clahe->apply(channel[2], lum_channel);
        Mat rg_hist_sp[3];
        split(rg, rg_hist_sp);
        calcHist(&rg_hist_sp[0], 1, 0, thres, h_hist, 1, &buck, &histRange, true, true);
        calcHist(&rg_hist_sp[1], 1, 0, thres, s_hist, 1, &buck, &histRange, true, true);
        calcHist(&rg_hist_sp[2], 1, 0, thres, v_hist, 1, &buck, &histRange, true, true);

        
        //cout << "Histogram H: " << h_hist<< endl;
        //cout << "Histogram S: " << s_hist<< endl;
        //cout << "Histogram V: " << v_hist<< endl;

        //normalize(r_hist, r_hist, 0, 255.0, NORM_MINMAX, -1, Mat());
        //normalize(g_hist, g_hist, 0, 255.0, NORM_MINMAX, -1, Mat());
        //normalize(b_hist, b_hist, 0, 255.0, NORM_MINMAX, -1, Mat());
        
        patch_packet *item = (patch_packet *) calloc(1, sizeof (patch_packet));
        item->feature = (feature_vector*) calloc(1, sizeof(feature_vector));
        for(int i = 0; i < contour->size(); i++){
            Point2i pt = contour->at(i);
            pt.x = pt.x + patch->x;
            pt.y = pt.y + patch->y;
            contour->at(i) = pt;
        }
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
        /*for(int i = 0; i<HISTOGRAM_SIZE; i++){
            printf("HISTO: %f %f %f\n", item->feature->hist_h[i], item->feature->hist_s[i],item->feature->hist_v[i]);
        }*/
        
        
        item->left = (patch_packet*)patch->left;
        item->right = (patch_packet*)patch->right;
        item->up = (patch_packet*)patch->up;
        item->down = (patch_packet*)patch->down;
        item->mac = nc->topo->mac;
        //item->id = comm->file_cnt;
        item->id = patch->id;
        struct timespec current;
        clock_gettime(CLOCK_REALTIME, &current);
        item->timeout = current;
        item->timeout.tv_sec += 7;
        item->next = 0;
        item->prev = 0;
        deb_printf("size of item: %d %d\n", sizeof(patch_packet), sizeof(feature_vector));
        deb_printf("item %p\n", item);
        comm->ask_neighbours(item);
        
        //fill item with data from RASPIPATCH
        deb_printf("saving it in list\n");
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
        deb_printf("saved it in list\n");
        comm->file_cnt++;
        deb_printf("increased file counter\n");

        RotatedRect boundRect = minAreaRect(*contour);
        
        match_surf_features(&thres, &rgb, boundRect.angle, patch->id);
    }
    //img.release();

}

Mat High_Res_Worker::convert(RASPITEX_PATCH *patch) {
        Mat mat_image(patch->height, patch->width, CV_8UC4, (void*)patch->buffer);
        return mat_image;
}

int32_t High_Res_Worker::identify_object(patch_packet *item) {
    if (((int) item->left) != 1 && ((int) item->right) != 1 && ((int) item->up) != 1 && ((int) item->down) != 1) {
        //get all feature vector and classify
        for (int i = 0; i < item->feature->contour->size(); i++) {
            Point2i pt = item->feature->contour->at(i);
            deb_printf("original Contour Point: %d %d\n", pt.x, pt.y);
        }
        
       
        
        deb_printf("start combining objects\n");
        combine_objects(item, item->left, LEFT_SIDE);
        combine_objects(item, item->right, RIGHT_SIDE);
        combine_objects(item, item->up, UP_SIDE);
        combine_objects(item, item->down, DOWN_SIDE);
        deb_printf("combined all objects\n");
        
        
        save_contour_in_file(item->feature->contour);
        //normalize histograms
        /*for(int i = 0; i < HISTOGRAM_SIZE, i++){
            item->feature->hist_h
        }*/
        deb_printf("calculating contour data\n");
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
        deb_printf("calculate moments and hu moments\n");
        //HuMoments
        Moments mu = moments(*contour, false);
        double hu[7];
        HuMoments(mu, hu);
        
        deb_printf("allocating final vector for classification\n");
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
        
        deb_printf("fill mat object with final vector\n");
        Mat features(1, 7+5+3*HISTOGRAM_SIZE, CV_32FC1);
        for(int i = 0; i < 7+5+3*HISTOGRAM_SIZE; i++){
            features.at<float>(0, i) = final_vector[i];
            //printf("%f\n", item->final_vector[i]);
        }
        //cout << features << endl;
        cout << features.size() << endl;
        float result = classifier->predict(features);
        int object = (int)floor(result+0.5);
        printf("Nummeric Result: %f %d\n", result, object);
        printf("Result of classifier: %s\n", object_names[object]);
        
        struct classification_result *class_item = (struct classification_result*) malloc(sizeof(struct classification_result));
        class_item->id = item->id;
        class_item->classification = object;
        class_item->object = -1;
        class_out->add((RASPITEX_PATCH*) class_item, 0);
        
        return object;
    }
    return -1;
}

void High_Res_Worker::combine_objects(patch_packet* dest, patch_packet* src, uint8_t dir) {

    
    if (src != 0 && src->feature != 0) {
        deb_printf("Combining Objects %lx %lx from side %d\n", dest->mac, src->mac, dir);

        for (int i = 0; i < HISTOGRAM_SIZE; i++) {
            dest->feature->hist_h[i] += src->feature->hist_h[i];
            dest->feature->hist_s[i] += src->feature->hist_s[i];
            dest->feature->hist_v[i] += src->feature->hist_v[i];
        }

        for (int i = 0; i < src->feature->contour->size(); i++) {
            Point pt = src->feature->contour->at(i);
            deb_printf("new Contour Point: %d %d\n", pt.x, pt.y);
            switch (dir) {
                case LEFT_SIDE:
                    pt.x = pt.x + HIGH_OUTPUT_X;
                    if (fabs(pt.x - HIGH_OUTPUT_X) > EPSILON) {
                        dest->feature->contour->push_back(pt);
                    }
                    break;
                case RIGHT_SIDE:
                    pt.x = pt.x - HIGH_OUTPUT_X;
                    if (fabs(pt.x) > EPSILON) {
                        dest->feature->contour->push_back(pt);
                    }
                    break;
                case UP_SIDE:
                    pt.y = pt.y + HIGH_OUTPUT_Y;
                    if (fabs(pt.y - HIGH_OUTPUT_Y) > EPSILON) {
                        dest->feature->contour->push_back(pt);
                    }
                    break;
                case DOWN_SIDE:
                    pt.y = pt.y - HIGH_OUTPUT_Y;
                    if (fabs(pt.y) > EPSILON) {
                        dest->feature->contour->push_back(pt);
                    }
                    break;
            }
        }
    }
}


void High_Res_Worker::check_objects(patch_packet *start){
    //deb_printf("checking objects for timeout\n");
    struct timespec current;
    clock_gettime(CLOCK_REALTIME, &current);
    //packet->timeout.tv_sec = current.tv_sec + TIMEOUT;
    patch_packet *item = start;
    while(item != 0){
        if(item->timeout.tv_sec < current.tv_sec){
            if(((int) item->left) == 1){
                item->left = (patch_packet *)0; 
            }
            if(((int) item->right) == 1){
                item->right = (patch_packet *)0; 
            }
            if(((int) item->up) == 1){
                item->up = (patch_packet *)0; 
            }
            if(((int) item->down) == 1){
                item->down = (patch_packet *)0; 
            }
            if(item->state == 1){
                item->state = 0;
            }
        }
        item = item->next;
    }
    //deb_printf("end checking objects\n");
}

void High_Res_Worker::save_contour_in_file(vector<Point> *contour){
    int x_min=10000, y_min=10000;
    int x_max=0, y_max=0;
    for(int i = 0; i < contour->size(); i++){
        Point2i pt = contour->at(i);
        if(x_min > pt.x){
            x_min = pt.x;
        }
        if(y_min > pt.y){
            y_min = pt.y;
        }
        if(x_max < pt.x){
            x_max = pt.x;
        }
        if(y_max < pt.y){
            y_max = pt.y;
        }
    }
    deb_printf("%d %d %d %d\n", x_min, x_max, y_min, y_max);
    for(int i = 0; i < contour->size(); i++){
        Point2i pt = contour->at(i);
        deb_printf("old Contour Point: %d %d\n", pt.x, pt.y);
        pt.x = pt.x + -1*x_min;
        pt.y = pt.y + -1*y_min;
        deb_printf("new Contour Point: %d %d\n", pt.x, pt.y);

        contour->at(i) = pt;
    }
    
    vector<Point> temp;
    convexHull(*contour, temp, false);
    RNG rng(12345);
    Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
    Mat img(y_max - y_min, x_max - x_min, CV_8UC3);
    drawContours(img, vector<vector<Point> >(1,temp), -1, color, 1, 8);
    imwrite("combined_image.png", img);
}

void High_Res_Worker::match_surf_features(Mat* mask, Mat* img, float angle, uint16_t id){
        printf("Angle of object: %f\n", angle);

    vector<KeyPoint> kp;
    surf->detect(*img, kp, *mask);
    Mat desc;
    surf->compute(*img, kp, desc);
    cout << "Descriptor" << desc.size() << endl;
    cout << "Keypoints" << kp.size() << endl;
    
    Mat img_keypoints;
    /*cout << "Keypoints: ";
    for(int i = 0; i < kp.size(); i++){
       cout << kp[i].pt.x << " " << kp[i].pt.y << endl; 
    }*/
    drawKeypoints(*img, kp, img_keypoints, Scalar::all(-1), DrawMatchesFlags::DEFAULT);
    imwrite("surf_match.png", img_keypoints);


    BFMatcher matcher(NORM_L2, false);
    float min_angle_count = 0;
    int min_angle_index = 0;
    int min_total_count = 0;

    for (int p = 0; p < surf_saved_desc.size(); p++) {
        vector<DMatch> matches;
        double sel_dist = 0;
        float matching_angle = 0;
        int total_count = 0;
        if (surf_saved_desc[p].empty() == false && desc.empty() == false) {
            matcher.match(surf_saved_desc[p], desc, matches);
            std::sort(matches.begin(), matches.end(), comparator);
            if (!matches.empty()) {
                int ceil = std::min(10, (int) matches.size());
                for (int i = 0; i < ceil; i++) {
                    for (int j = i; j < ceil; j++) {
                        for (int q = j; q < ceil; q++) {
                            total_count++;
                            Point2f orig1 = surf_saved_key[p][matches[i].queryIdx].pt;
                            Point2f orig2 = surf_saved_key[p][matches[j].queryIdx].pt;
                            Point2f orig3 = surf_saved_key[p][matches[q].queryIdx].pt;
                            double angle1 = 0, dist1 = 0;
                            calc_angle_dist(orig1, orig2, orig3, &angle1, &dist1);

                            Point2f new1 = kp[matches[i].trainIdx].pt;
                            Point2f new2 = kp[matches[j].trainIdx].pt;
                            Point2f new3 = kp[matches[q].trainIdx].pt;
                            double angle2 = 0, dist2 = 0;
                            calc_angle_dist(new1, new2, new3, &angle2, &dist2);

                            if (isnormal(angle1) && isnormal(angle2)) {
                                if (fabs(angle1 - angle2) < 5.0) {
                                    matching_angle += 1.0;
                                }
                            }
                            if (isnormal(dist1) && isnormal(dist2)) {
                                sel_dist += fabs(dist1 - dist2);
                            }
                        }
                    }
                }
                if (matching_angle > min_angle_count) {
                    min_angle_count = matching_angle;
                    min_angle_index = p;
                    min_total_count = total_count;
                }
            }
        }
    }
    printf("min angle count %d min total count %d\n", min_angle_count, min_total_count);
    float confidence = (min_angle_count/min_total_count)*100;
    printf("confidence: %f in cat %d\n", confidence, min_angle_index);
    
    struct classification_result *item = (struct classification_result*) malloc(sizeof(struct classification_result));

    //percent of angles right
    if(isnormal(confidence) == 0 || confidence < 20.0){
        surf_saved_desc.push_back(desc);
        surf_saved_key.push_back(kp);
    }
    
    if(isnormal(confidence) == 1 && confidence >= 20.0){
        item->object = min_angle_index;
    }else{
        item->object = -2;
    }
    item->id = id;
    item->classification = -1;
    
    class_out->add((RASPITEX_PATCH *)item, 0);
}

bool High_Res_Worker::comparator(DMatch a,DMatch b)
{
        return a.distance<b.distance;
}
void High_Res_Worker::calc_angle_dist(Point2f pt1, Point2f pt2, Point2f pt3, double *angle, double *dist) {
    double dist1 = sqrt((pt1.x - pt2.x) * (pt1.x - pt2.x) + (pt1.y - pt2.y) * (pt1.y - pt2.y));
    double dist2 = sqrt((pt1.x - pt3.x) * (pt1.x - pt3.x) + (pt1.y - pt3.y) * (pt1.y - pt3.y));
    double dist3 = sqrt((pt2.x - pt3.x) * (pt2.x - pt3.x) + (pt2.y - pt3.y) * (pt2.y - pt3.y));
    double angle1 = acos((dist1 * dist1 + dist2 * dist2 - dist3 * dist3) / (2 * dist1 * dist2))*(180 / 3.14159);
    if(isnormal(angle1)){
        *angle += angle1;
    }
    if(isnormal(dist1)){
        *dist += dist1;
    }
}