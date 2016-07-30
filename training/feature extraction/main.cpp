#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <stdio.h>
#include <string.h>

using namespace cv;
using namespace std;

int main(int argc, char** argv ){
    Mat img;
    img = imread(argv[1], CV_LOAD_IMAGE_COLOR);
    
        
        Mat rgb;
        cvtColor(img, rgb, COLOR_RGBA2RGB);
        //marker richtig plazieren... auch am rand
        Mat marker = Mat::zeros(rgb.size(), CV_32SC1);
        Size img_size = rgb.size();
        printf("patch size: %d %d\n", img_size.width, img_size.height);
        circle(marker, Point(img_size.width/2, img_size.height/2), 20, CV_RGB(1,1,1),-1);
        circle(marker, Point(0,0), 5, CV_RGB(255,255,255), -1);
        circle(marker, Point(0,img_size.height), 5, CV_RGB(255,255,255), -1);
        circle(marker, Point(img_size.width,0), 5, CV_RGB(255,255,255), -1);
        circle(marker, Point(img_size.width,img_size.height), 5, CV_RGB(255,255,255), -1);
        imshow("Marker v1", marker*10000);
        waitKey(0);

        watershed(rgb, marker);
        Mat mark = Mat::zeros(marker.size(), CV_8UC1);
        marker.convertTo(mark, CV_8UC1);
        bitwise_not(mark, mark);
        imshow("Markers_v2", mark);
        //bitwise_not(mark, mark);
        
        vector<Mat> bgr_planes;
        split(rgb, bgr_planes);
        
        
        float range[] = {0,256};
        const float *histRange = {range};
        int buck = 32;
        Mat b_hist, g_hist, r_hist;
        
        //Histogram
        calcHist(&bgr_planes[0], 1, 0, mark, b_hist, 1, &buck, &histRange, true, true);
        calcHist(&bgr_planes[1], 1, 0, mark, g_hist, 1, &buck, &histRange, true, true);
        calcHist(&bgr_planes[2], 1, 0, mark, r_hist, 1, &buck, &histRange, true, true);
        
        //HuMoments
        vector<vector<Point> > contours;
        RNG rng(12345);
        findContours(mark, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
        double largest = 0;
        int largest_index = 0;
        for(int i = 0; i < contours.size(); i++){
            double a = contourArea(contours[i], false);
            if(a > largest){
                largest = a;
                largest_index = i;
            }
        }
        Moments mu = moments(contours[largest_index], false);
        double hu[7];
        HuMoments(mu, hu);
        
        //contourArea
        double area = contourArea(contours[largest_index]);
        
        //contour Perimeter
        double perimeter = arcLength(contours[largest_index], true);
        
        cout << "Area: " << area << endl;
        cout << "Perimeter: " << perimeter << endl;
        cout << "HuMoments: " << hu[0] << " " << hu[1] << " " << hu[2] << " " << hu[3] << " "
                << hu[4] << " " << hu[5] << " " << hu[6] << " " << hu[7] << endl;
        cout << "Histogram R: " << endl;
        for(int i = 0; i < 32; i++){
            cout << " " << r_hist.at<float>(i);
        }
        cout << endl;
        cout << "Histogram G: " << endl;
        for(int i = 0; i < 32; i++){
            cout << " " << g_hist.at<float>(i);
        }
        cout << endl;
        cout << "Histogram B: " << endl;
        for(int i = 0; i < 32; i++){
            cout << " " << b_hist.at<float>(i);
        }
        cout << endl;
         waitKey(0);
        return 0;
}