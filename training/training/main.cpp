#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <vector>
#include <cstring>
#include <fstream>
#include <stdint.h>
#include <stdlib.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/bgsegm.hpp>
#include "opencv2/photo.hpp"
#include <opencv2/features2d.hpp>
#include "opencv2/xfeatures2d/nonfree.hpp"
#include "opencv2/ml.hpp"

using namespace std;
using namespace cv;
using namespace cv::ml;

#define HISTOGRAM_SIZE 32

Mat src; Mat src_gray;
char *pic_name = 0;
int thresh = 50;
int max_thresh = 255;
RNG rng(12345);

FILE *save_features;

#define PICTURE_DIR "../pictures/"

struct feature{
    uint32_t *final_vector;
    uint64_t mac;
    uint16_t id;
    uint16_t result;
    struct feature* next;
};

struct feature *first = 0;
struct feature *last = 0;
uint16_t count_list = 0;

void extract_features(char *name);
void load_features();
void save_in_file(uint32_t *final_vector, int32_t res);
uint8_t search_list(uint64_t mac, uint16_t id);

int main() {
    
    const char * dir_pictures = PICTURE_DIR;
    
    save_features = fopen("output.txt", "a+");
    
    load_features();
    printf("Count list %d\n", count_list);

    DIR *dp;
    struct dirent *ep;
    dp = opendir(dir_pictures);
    if (dp != NULL) {
        while (ep = readdir(dp)) {
            size_t string_size = strlen(ep->d_name) + strlen(dir_pictures);
            char * name = (char *) malloc(string_size);
            strcpy(name, dir_pictures);
            strcpy(name + strlen(dir_pictures), ep->d_name);
            pic_name = ep->d_name;
            extract_features(name);
            
        }
        (void) closedir(dp);
    } else
        perror("Couldn't open the directory pictures");
    
    fclose(save_features);
    
    struct feature *item = first;
    int cnt = 0;
    while(item != 0){
        cnt++;
        item = item->next;
    }
    printf("Feature Count %d\n", cnt);
    
    Mat features(cnt, 7+5+3*HISTOGRAM_SIZE, CV_32SC1);
    Mat classification(cnt, 1, CV_32SC1);
    item = first;
    cnt = 0;
    while(item != 0){
        for(int i = 0; i < 7+5+3*HISTOGRAM_SIZE; i++){
            features.at<int>(i) = item->final_vector[i];
        }
        classification.at<int>(cnt) = item->result;
        cnt++;
        item = item->next;
    }
    
    
    Ptr<RTrees> rtrees = RTrees::create();
    rtrees->setMaxDepth(15);
    rtrees->setMinSampleCount(2);
    rtrees->setRegressionAccuracy(0.f);
    rtrees->setUseSurrogates(false);
    rtrees->setMaxCategories(16);
    rtrees->setPriors(Mat());
    rtrees->setCalculateVarImportance(false);
    rtrees->setActiveVarCount(1);
    rtrees->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, 5, 0));
    rtrees->train(features, ROW_SAMPLE, classification);

    rtrees->save("classifier.xml");

    //build mat file out of features
    //run classifier


    return 0;
}

void extract_features(char *name) {
    uint64_t mac;
    uint16_t id;
    sscanf(pic_name, "%lx_%d.png", &mac, &id);
    if (search_list(mac, id) == 0) {
        src = imread(name);
        if (src.empty() == 0) {
            cvtColor(src, src_gray, CV_RGB2GRAY);
            blur(src_gray, src_gray, Size(3, 3));
            char *source_window = "Source";
            namedWindow(source_window, CV_WINDOW_AUTOSIZE);


            ///////////////////////////////////////
            Mat canny_output;
            vector<vector<Point> > contours;
            vector<Vec4i> hierarchy;

            Mat hsv;
            cvtColor(src, hsv, COLOR_BGR2HSV);
            Mat channel[3];
            split(hsv, channel);
            threshold(channel[1], canny_output, thresh, 255, THRESH_BINARY);

            findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

            float range[] = {0, 256};
            const float *histRange = {range};
            int buck = 32;
            Mat b_hist, g_hist, r_hist;
            calcHist(&channel[0], 1, 0, canny_output, b_hist, 1, &buck, &histRange, true, true);
            calcHist(&channel[1], 1, 0, canny_output, g_hist, 1, &buck, &histRange, true, true);
            calcHist(&channel[2], 1, 0, canny_output, r_hist, 1, &buck, &histRange, true, true);

            int winner = 0;
            int largest = 0;
            for (int i = 0; i < contours.size(); i++) {
                if (largest < contours[i].size()) {
                    largest = contours[i].size();
                    winner = i;
                }

            }

            double epsilon = 0.0035 * arcLength(contours[winner], true);
            vector<vector<Point> > new_cont;
            vector<Point> vec_temp;
            new_cont.push_back(vec_temp);
            approxPolyDP(contours[winner], new_cont.at(0), epsilon, true);

            //Contour Features
            vector<Point> contour = new_cont.at(0);
            RotatedRect boundRect = minAreaRect(contour);
            Point2f vertices[4];
            boundRect.points(vertices);


            double aspectRatio = ((double) boundRect.size.width) / boundRect.size.height;
            double area = contourArea(contour);
            double extend = area / (boundRect.size.width * boundRect.size.height); //good
            vector<Point> hull;
            convexHull(contour, hull, false);
            double hull_area = contourArea(hull);
            double solidity = area / hull_area; //good
            double equiv_diameter = sqrt(4 * area / 3.14159); //not bad
            vector<Point> temp;
            double trian_area = minEnclosingTriangle(contour, temp);
            double trian_extend = area / trian_area; //good
            Point2f center;
            float radius;
            minEnclosingCircle(contour, center, radius);
            double circle_extend = area / (3.1415 * radius * radius); //good

            printf("%s %f %f %f %f %f %f %f %f \n", pic_name, aspectRatio, area, extend, hull_area, solidity, equiv_diameter, trian_extend, circle_extend);
            //moments
            //color histogram in HSV
            Moments mu = moments(contour, false);
            double hu[7];
            HuMoments(mu, hu);

            uint32_t hist_r[HISTOGRAM_SIZE];
            uint32_t hist_g[HISTOGRAM_SIZE];
            uint32_t hist_b[HISTOGRAM_SIZE];

            for (int i = 0; i < HISTOGRAM_SIZE; i++) {
                hist_r[i] = (uint32_t) r_hist.at<float>(i);
                hist_g[i] = (uint32_t) g_hist.at<float>(i);
                hist_b[i] = (uint32_t) b_hist.at<float>(i);
            }

            uint32_t *final_vector = (uint32_t*) malloc(sizeof (uint32_t)*(7 + 5 + 3 * HISTOGRAM_SIZE));
            for (int i = 0; i < 7; i++) {
                final_vector[i] = (uint32_t) (1000 * hu[i]);
            }
            final_vector[7] = (uint32_t) (1000 * extend);
            final_vector[8] = (uint32_t) (1000 * solidity);
            final_vector[9] = (uint32_t) (equiv_diameter);
            final_vector[10] = (uint32_t) (1000 * trian_extend);
            final_vector[11] = (uint32_t) (1000 * circle_extend);
            memcpy(&final_vector[12], hist_r, sizeof (uint32_t) * HISTOGRAM_SIZE);
            memcpy(&final_vector[12 + HISTOGRAM_SIZE], hist_g, sizeof (uint32_t) * HISTOGRAM_SIZE);
            memcpy(&final_vector[12 + HISTOGRAM_SIZE * 2], hist_b, sizeof (uint32_t) * HISTOGRAM_SIZE);

            //Mat features(1, 7 + 5 + 3 * HISTOGRAM_SIZE, CV_32SC1, (void*) final_vector);

            //printf("Contour Points: %ld\n", contours[winner].size());
            Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
            Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
            drawContours(drawing, new_cont, 0, color, 2, 8, hierarchy, 0, Point());
            for (int i = 0; i < 4; i++)
                line(drawing, vertices[i], vertices[(i + 1) % 4], Scalar(0, 255, 0));
            namedWindow("Contours", CV_WINDOW_AUTOSIZE);
            resize(drawing, drawing, Size(640, 480));

            imshow("Contours", drawing);
            resize(src, src, Size(640, 480));
            imshow(source_window, src);
            int32_t res = waitKey(0);
            res -= 1048624;
            printf("key pressed: %d\n\n", res);

            struct feature *item = (struct feature *) calloc(1, sizeof (struct feature));
            item->final_vector = final_vector;
            item->mac = mac;
            item->id = id;
            item->result = res;
            if (first == 0) {
                first = item;
                last = item;
            } else {
                last->next = item;
                last = item;
            }
            count_list++;

            ////////////////////////////////////////////////////////
            save_in_file(final_vector, res);
        }
    }
}

void save_in_file(uint32_t *final_vector, int32_t res) {
    char * str = " %d ";
    char buffer[20];
    snprintf(buffer, 20, str, res);
    fputs(buffer, save_features);
    
    uint64_t mac;
    uint16_t id;
    sscanf(pic_name, "%lx_%d.png", &mac, &id);
    memset(buffer, 0, 20);
    snprintf(buffer, 20, " %lx %d", mac, id);
    fputs(buffer, save_features);
    for (int i = 0; i < (7 + 5 + 3 * HISTOGRAM_SIZE); i++) {
        memset(buffer, 0, 20);
        snprintf(buffer, 20, str, final_vector[i]);
        fputs(buffer, save_features);
    }
    char *end = "\n\0";
    fputs(end, save_features);
    
    
    //add features to linked list
}

void load_features() {
    char buffer[1000];


    while (feof(save_features) == 0) {
        printf("Reading line %d\n", count_list);
        memset(buffer, 0, 1000);
        struct feature *item = (struct feature *) calloc(1, sizeof (struct feature));
        item->final_vector = (uint32_t*) malloc(sizeof (uint32_t)*(7 + 5 + 3 * HISTOGRAM_SIZE));
        int i = 0;

        char *str = fgets(buffer, 1000, save_features);
        if(str != NULL){
        char *token;
        token = strtok(buffer, " ");
        while (token != NULL) {
            uint64_t temp = 0;
            if (i == 1) {
                sscanf(token, "%lx", &temp);
            } else {
                sscanf(token, "%d", &temp);
            }
            switch (i) {
                case 0: item->result = temp;
                    break;
                case 1: item->mac = temp;
                    break;
                case 2: item->id = temp;
                    break;
                default:
                    item->final_vector[i - 3] = temp;
            }
            token = strtok(NULL, " ");
            i++;
        }
        count_list++;

        printf("feature from %lx %d loaded\n", item->mac, item->id);

        if (first == 0) {
            first = item;
            last = item;
        } else {
            last->next = item;
            last = item;
        }
        printf("finished reading line\n");
        }
    }
}

uint8_t search_list(uint64_t mac, uint16_t id){
    struct feature *item = first;
    while(item != 0){
        if(item->mac == mac && item->id == id){
            return 1;
        }
        item = item->next;
    }
    return 0;
}