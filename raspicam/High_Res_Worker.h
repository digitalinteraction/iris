/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   High_Res_Worker.h
 * Author: tobias
 *
 * Created on May 12, 2016, 4:12 PM
 */

#ifndef HIGH_RES_WORKER_H
#define HIGH_RES_WORKER_H
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include "../network/Packetbuffer.h"
#include "../network/NetworkControl.h"
#include "RaspiTex.h"
#include "CommImage.h"


using namespace cv;



class Buffer;

class High_Res_Worker {
public:
    High_Res_Worker(Buffer *buffer, Packetbuffer *out_buf, Packetbuffer *in_buf, NetworkControl *nc);
    virtual ~High_Res_Worker();
    int processing;
    void run();

private:
    Buffer * buf;
    int cnt;
    uint8_t prev_group;
    Mat convert(RASPITEX_PATCH *patch);
    void find_features(RASPITEX_PATCH *patch, uint8_t group);
    int32_t identify_object(patch_packet *item);
    void combine_objects(patch_packet* dest, patch_packet* src, uint8_t dir);
    void check_objects(patch_packet *start);
    void save_contour_in_file(vector<Point> *contour);
    void match_surf_features(Mat *mask, Mat *img);
    Packetbuffer *out;
    Packetbuffer *in;
    NetworkControl *nc;
    uint8_t pos;
    uint16_t id;
    
    patch_packet *first;
    patch_packet *last;
    
    CommImage *comm;
    
    Ptr<cv::xfeatures2d::SURF> surf;
    Ptr<cv::ml::RTrees> classifier;


};

#endif /* HIGH_RES_WORKER_H */

