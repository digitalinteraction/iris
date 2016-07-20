/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Low_Res_Worker.h
 * Author: tobias
 *
 * Created on May 3, 2016, 10:44 AM
 */

#ifndef LOW_RES_WORKER_H
#define LOW_RES_WORKER_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/bgsegm.hpp>
#include "opencv2/photo.hpp"
#include <opencv2/features2d.hpp>
#include "RaspiTex.h"
#include <pthread.h>
#include "../network/Packetbuffer.h"
#include "../network/NetworkControl.h"
#include <stdio.h>
#include <string.h>
#include <vector>

#include "Buffer.h"

using namespace cv;
using namespace std;

struct objects{
    vector<Point> contour;
    uint8_t id;
    struct objects *next;
};

//class Buffer;

class Low_Res_Worker {
public:
    Low_Res_Worker(Packetbuffer *out, NetworkControl *nc, Buffer *images_in);
    ~Low_Res_Worker();
    void run();
    int processing;
    int counter;
    
    int new_low_buffer;
    pthread_mutex_t buffer_lock;
    RASPITEX_PATCH low_patch;
    
    int requests_pending;
    RASPITEX_PATCH requests[10];
private:
    void process_image(uint8_t *image, size_t image_size);
    Mat convert(uint8_t *image, size_t image_size);
    int interprete_params(double mean, double sum);
    //void send_to_server(uint8_t* image, size_t image_size, uint8_t mode, uint8_t pos);
    void send_to_server(Mat *img, uint8_t mode, uint8_t pos);
    uint8_t match_contours(vector<Point> *contour);
    Mat mask;
    Ptr<BackgroundSubtractor> pMOG2;
    Mat previous;
    int cnt;
    int nr_img;
    float learning;
    Packetbuffer *out;
    NetworkControl *nc;
    uint8_t pos;
    uint8_t next_send;

    Buffer *images_in;
    struct objects *first;
    struct objects *last;
    uint8_t id_cnt;
    
    uint8_t thresH_low;
    uint8_t thresH_high;
    uint8_t thresS_low;
    uint8_t thresS_high;
    uint8_t thresV_low;
    uint8_t thresV_high;
};

#endif /* LOW_RES_WORKER_H */

