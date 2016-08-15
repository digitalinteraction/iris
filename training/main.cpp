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
int thresh = 47;
int max_thresh = 255;
RNG rng(12345);

FILE *save_features;
FILE *load_test;

#define PICTURE_DIR "../pictures/"
#define TEST_DIR "../test/"

struct feature{
    float *final_vector;
    uint64_t mac;
    uint16_t id;
    float result;
    struct feature* next;
};

struct feature *first = 0;
struct feature *last = 0;
struct feature *first_test = 0;
struct feature *last_test = 0;
uint16_t count_list = 0;
Ptr<RTrees> rtrees;
int var_arg = 0;

void extract_features(char *name, uint8_t mode);
void load_features(uint8_t mode);
void save_in_file(float *final_vector, float res, uint64_t mac, uint16_t id, uint8_t mode);
struct feature * search_list(uint64_t mac, uint16_t id, uint8_t mode);
void trainClassifier();
void checkClassifier();

int main(int argc, char **argv) {
    
    sscanf(argv[1], "%d", &var_arg);
    //printf("Please classify the pictures by pressing following keys:\n");
    //printf("0: Nothing\n");
    //printf("1: Carrot\n");
    //printf("2: Peach\n");
    //printf("3: Apple\n");
    save_features = fopen("output.txt", "a+");
    load_test = fopen("output_test.txt", "a+");
    
    load_features(0);
    //printf("Count list %d\n", count_list);

    trainClassifier();
    
    /*for(int i = 0; i < features.rows; i++){
        float result = rtrees->predict(features.row(i));
        if(result != classification.at<int>(i)){
            printf("%d Result: %f in class %d\n", i, result, classification.at<int>(i));
        }
    }*/
    load_features(1);
    checkClassifier();
    
    rtrees->save("classifier.xml");

    //build mat file out of features
    //run classifier


    return 0;
}

void checkClassifier(){
    const char * dir_train = TEST_DIR;

    DIR *dp;
    struct dirent *ep;
    dp = opendir(dir_train);
    if (dp != NULL) {
        while (ep = readdir(dp)) {
            size_t string_size = strlen(ep->d_name) + strlen(dir_train);
            char * name = (char *) malloc(string_size);
            strcpy(name, dir_train);
            strcpy(name + strlen(dir_train), ep->d_name);
            pic_name = ep->d_name;
            extract_features(name, 1);
        }
        (void) closedir(dp);
    } else
        perror("Couldn't open the directory train");

    fclose(load_test);

    struct feature *item = first_test;
    int cnt = 0;
    while (item != 0) {
        cnt++;
        item = item->next;
    }
    //printf("Train Feature Count %d\n", cnt);

    Mat features(cnt, 7 + 5 + 3 * HISTOGRAM_SIZE, CV_32FC1);
    Mat classification(cnt, 1, CV_32S);
    item = first_test;
    cnt = 0;
    while (item != 0) {
        //printf("START::");
        for (int i = 0; i < 7 + 5 + 2 * HISTOGRAM_SIZE; i++) {
            features.at<float>(cnt, i) = item->final_vector[i];
            //printf("%f\n", item->final_vector[i]);
        }
        //printf("class %d\n", (int) item->result);
        classification.at<int>(cnt) = (int) item->result;
        cnt++;
        item = item->next;
    }
    
    //printf("Features rows %d\n", features.rows);
    //printf("Classification rows %d\n", classification.rows);
    float total = 0;
    float success = 0;
    
    
    for (int i = 0; i < features.rows; i++) {
        float result = rtrees->predict(features.row(i));
        //printf("Classification result %f\n", result);
        if (result != classification.at<int>(i)) {
            //printf(" ERROR: Result: %f in class %d\n", result, classification.at<int>(i));
        }else{
            success += 1.0;
            //printf("Result: %f in class %d\n", result, classification.at<int>(i));
        }
        total += 1.0;

    }
    
    printf("%d success %f\n", var_arg, (success/total)*100);
}

void trainClassifier() {
    const char * dir_pictures = PICTURE_DIR;

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
            extract_features(name, 0);
        }
        (void) closedir(dp);
    } else
        perror("Couldn't open the directory pictures");

    fclose(save_features);

    struct feature *item = first;
    int cnt = 0;
    while (item != 0) {
        cnt++;
        item = item->next;
    }
    //printf("Feature Count %d\n", cnt);

    Mat features(cnt, 7 + 5 + 3 * HISTOGRAM_SIZE, CV_32FC1);
    Mat classification(cnt, 1, CV_32S);
    item = first;
    cnt = 0;
    while (item != 0) {
        //printf("START::");
        for (int i = 0; i < 7 + 5 + 2 * HISTOGRAM_SIZE; i++) {
            features.at<float>(cnt, i) = item->final_vector[i];
            //printf("%f\n", item->final_vector[i]);
        }
        //printf("class %d\n", (int) item->result);
        classification.at<int>(cnt) = (int) item->result;
        cnt++;
        item = item->next;
    }


    rtrees = RTrees::create();
    rtrees->setMaxDepth(var_arg);
    rtrees->setMinSampleCount(5);
    rtrees->setRegressionAccuracy(0);
    rtrees->setUseSurrogates(false);
    rtrees->setMaxCategories(12);
    rtrees->setPriors(Mat());
    rtrees->setCalculateVarImportance(false);
    rtrees->setActiveVarCount(4);
    rtrees->setTermCriteria(TermCriteria(CV_TERMCRIT_EPS, 2000, 0.001));
    //printf("training classifier\n");
    rtrees->train(features, ROW_SAMPLE, classification);
    for (int i = 0; i < features.rows; i++) {
        //cout << "ROW::" << features.row(i) << endl;
    }

    //cout << "Classification::" << classification << endl;
}

void extract_features(char *name, uint8_t mode) {
    uint64_t mac;
    uint16_t id;
    sscanf(pic_name, "%lx_%d.png", &mac, &id);
    //printf("pic: %s\n", pic_name);
    //printf("pic2: %lx, %d\n", mac, id);
    //if (search_list(mac, id) == 0) {
        //printf("name: %s\n", name);
        src = imread(name, CV_LOAD_IMAGE_COLOR);
        if (src.empty() == 0) {
            cvtColor(src, src_gray, CV_BGRA2GRAY);
            blur(src_gray, src_gray, Size(3, 3));
            char *source_window = "Source";
            

            ///////////////////////////////////////
            Mat mask;
            vector<vector<Point> > contours;
            vector<Vec4i> hierarchy;

            Mat hsv, rgb;
            cvtColor(src, rgb, CV_BGR2RGB);
            cvtColor(rgb, hsv, CV_RGB2HSV);
            Mat channel[3];
            split(hsv, channel);
            //imshow("H", channel[0]);
            //imshow("L", channel[1]);
            //imshow("S", channel[2]);

            threshold(channel[1], mask, thresh, 255, THRESH_BINARY);
            //imshow("Mask", mask);
            findContours(mask, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

            
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
            //imshow("RG space", rg);
            
            
            float range[] = {0, 255};
            const float *histRange = {range};
            int buck = HISTOGRAM_SIZE;
            Mat r_hist, g_hist, b_hist;
            Mat rg_hist_sp[3];
            
            
            split(rg, rg_hist_sp);
            calcHist(&rg_hist_sp[0], 1, 0, mask, r_hist, 1, &buck, &histRange, true, true);
            calcHist(&rg_hist_sp[1], 1, 0, mask, g_hist, 1, &buck, &histRange, true, true);
            calcHist(&rg_hist_sp[2], 1, 0, mask, b_hist, 1, &buck, &histRange, true, true);

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

            //printf("%s %f %f %f %f %f %f %f %f \n", pic_name, aspectRatio, area, extend, hull_area, solidity, equiv_diameter, trian_extend, circle_extend);
            //moments
            //color histogram in HSV
            Moments mu = moments(contour, false);
            double hu[7];
            HuMoments(mu, hu);

            float hist_r[HISTOGRAM_SIZE];
            float hist_g[HISTOGRAM_SIZE];
            float hist_b[HISTOGRAM_SIZE];

            for (int i = 0; i < HISTOGRAM_SIZE; i++) {
                hist_r[i] =  r_hist.at<float>(i);
                hist_g[i] =  g_hist.at<float>(i);
                hist_b[i] =  b_hist.at<float>(i);
            }
            
            struct feature *item_found = search_list(mac, id, mode);
            //printf("item found %p\n", item_found);
            float *final_vector = 0;
            
            if(item_found == 0){
                final_vector = (float*) malloc(sizeof (float)*(7 + 5 + 3 * HISTOGRAM_SIZE));
            }else{
                final_vector = item_found->final_vector;
            }
            
            for (int i = 0; i < 7; i++) {
                final_vector[i] = (1000 * hu[i]);
            }
            final_vector[7] =  (1000 * extend);
            final_vector[8] =  (1000 * solidity);
            final_vector[9] = (equiv_diameter);
            final_vector[10] = (1000 * trian_extend);
            final_vector[11] =  (1000 * circle_extend);
            memcpy(&final_vector[12], hist_r, sizeof (float) * HISTOGRAM_SIZE);
            memcpy(&final_vector[12 + HISTOGRAM_SIZE], hist_g, sizeof (float) * HISTOGRAM_SIZE);
            memcpy(&final_vector[12 + HISTOGRAM_SIZE * 2], hist_b, sizeof (float) * HISTOGRAM_SIZE);

            //Mat features(1, 7 + 5 + 3 * HISTOGRAM_SIZE, CV_32SC1, (void*) final_vector);

            //printf("Contour Points: %ld\n", contours[winner].size());
            if(item_found == 0){
                namedWindow(source_window, CV_WINDOW_AUTOSIZE);

                imshow("MASK", mask);
            Mat drawing = Mat::zeros(mask.size(), CV_8UC3);
            Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
            drawContours(drawing, new_cont, 0, color, 2, 8, hierarchy, 0, Point());
            for (int i = 0; i < 4; i++)
                line(drawing, vertices[i], vertices[(i + 1) % 4], Scalar(0, 255, 0));
            namedWindow("Contours", CV_WINDOW_AUTOSIZE);
            resize(drawing, drawing, Size(640, 480));

            imshow("Contours", drawing);
            resize(src, src, Size(640, 480));
            imshow(source_window, src);
            }
            float res = 0;
            if(item_found == 0){
                res = (float)waitKey(0);
                res -= 1048624.0;
            }
            
            //printf("key pressed on mac %lx: %f\n\n", mac, res);

        if (item_found == 0) {
            struct feature *item = (struct feature *) calloc(1, sizeof (struct feature));
            item->final_vector = final_vector;
            item->mac = mac;
            item->id = id;
            item->result = res;
            if (mode == 0) {
                if (first == 0) {
                    first = item;
                    last = item;
                } else {
                    last->next = item;
                    last = item;
                }
                count_list++;
            } else {
                if (first_test == 0) {
                    first_test = item;
                    last_test = item;
                } else {
                    last_test->next = item;
                    last_test = item;
                }
            }
        }

            ////////////////////////////////////////////////////////
            if(item_found == 0){
            save_in_file(final_vector, res, mac, id, mode);
            }
        }
    //}
}

void save_in_file(float *final_vector, float res, uint64_t mac, uint16_t id, uint8_t mode) {
    
    
    
    char * str = " %f";
    char buffer[20];
    snprintf(buffer, 20, str, res);
    if(mode == 0){
    fputs(buffer, save_features);
    }else{
        fputs(buffer, load_test);
    }
    //printf("adding line with mac %lx and id %d\n", mac, id);
    //uint64_t mac;
    //uint16_t id;
    //sscanf(pic_name, "%lx_%d.png", &mac, &id);
    memset(buffer, 0, 20);
    snprintf(buffer, 20, " %lx %d", mac, id);
    if(mode == 0){
    fputs(buffer, save_features);
    }else{
        fputs(buffer, load_test);
    }
    /*for (int i = 0; i < (7 + 5 + 3 * HISTOGRAM_SIZE); i++) {
        memset(buffer, 0, 20);
        snprintf(buffer, 20, str, final_vector[i]);
        fputs(buffer, save_features);
    }*/
    char *end = "\n\0";
    if(mode == 0){
    fputs(end, save_features);
    }else{
        fputs(end, load_test);
    }
    
    
    //add features to linked list
}

void load_features(uint8_t mode) {
    char buffer[2000];

    FILE *file;
    if(mode == 0){
        file = save_features;
    }else{
        file = load_test;
    }
    while (feof(file) == 0) {
        //printf("Reading line %d\n", count_list);
        memset(buffer, 0, 2000);
        struct feature *item = (struct feature *) calloc(1, sizeof (struct feature));
        item->final_vector = (float*) malloc(sizeof (float)*(7 + 5 + 3 * HISTOGRAM_SIZE));
        int i = 0;

        char *str = fgets(buffer, 2000, file);
        if(str != NULL){
        char *token;
        token = strtok(buffer, " ");
        while (token != NULL) {
            float temp = 0;
            uint64_t temp2 = 0;
            if (i == 1) {
                sscanf(token, "%lx", &temp2);
            } else if(i == 2){
                sscanf(token, "%d", &temp2);
            }else {
                sscanf(token, "%f", &temp);
            }
            switch (i) {
                case 0: item->result = temp;
                    break;
                case 1: item->mac = temp2;
                    break;
                case 2: item->id = (uint16_t)temp2;
                    break;
                default:
                    item->final_vector[i - 3] = temp;
            }
            token = strtok(NULL, " ");
            i++;
        }
        count_list++;

        //printf("feature from %lx %d loaded\n", item->mac, item->id);
        
        
        if(mode == 0){
        if (first == 0) {
            first = item;
            last = item;
        } else {
            last->next = item;
            last = item;
        }
        }else{
            if (first_test == 0) {
            first_test = item;
            last_test = item;
        } else {
            last_test->next = item;
            last_test = item;
        }
        }
        //printf("finished reading line\n");
        }
    }
}

struct feature * search_list(uint64_t mac, uint16_t id, uint8_t mode){
    struct feature *item;
    if(mode == 0){
        item = first;
    }else{
        item = first_test;
    }
    while(item != 0){
        if(item->mac == mac && item->id == id){
            return item;
        }
        item = item->next;
    }
    return 0;
}