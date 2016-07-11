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

using namespace cv;




class Low_Res_Worker {
public:
    Low_Res_Worker(Packetbuffer *out, NetworkControl *nc);
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
    void send_to_server(uint8_t* image, size_t image_size);
    Mat mask;
    Ptr<BackgroundSubtractor> pMOG2;
    Mat previous;
    int cnt;
    int nr_img;
    float learning;
    Packetbuffer *out;
    NetworkControl *nc;
};

#endif /* LOW_RES_WORKER_H */

