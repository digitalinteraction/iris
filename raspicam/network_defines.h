/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   network_defines.h
 * Author: tobias
 *
 * Created on July 20, 2016, 1:19 PM
 */

#ifndef NETWORK_DEFINES_H
#define NETWORK_DEFINES_H

#include <vector>
#include <opencv2/opencv.hpp>


#define CLIENT_SIDE
#define IMAGE_PACKET 1
#define TOPO_PACKET 2
#define CONTROL_PACKET 3
#define MAX_PACKET_SIZE 60000
#define BORDER_HIGH_RES 30

#define HISTOGRAM_SIZE 32

#define LEFT_SIDE 3
#define DOWN_SIDE 1
#define RIGHT_SIDE 0
#define UP_SIDE 2

//debug variables
//#define DEBUG_COMM_IMAGE
//#define DEBUG_HIGH_RES
//#define DEBUG_LOW_RES


#define REL_ERROR_BUF_SIZE 0
#define REL_ERROR_LIST 1
#define REL_ERROR_DEAD 2
#define REL_ERROR_TIMEOUT 3
#define REL_ERROR_BUF_ALLOC 4



struct low_res_header{
    uint8_t port;
    uint8_t pos;
    uint64_t mac;
    uint32_t size;
    double weight;
};

typedef struct feat_vect{
    float hist_h[HISTOGRAM_SIZE];
    float hist_s[HISTOGRAM_SIZE];
    float hist_v[HISTOGRAM_SIZE];
    uint32_t contour_size;
    std::vector<cv::Point> *contour;
}feature_vector;


typedef struct high_res_packet{
    uint32_t file_cnt;
    struct timespec timeout;
    uint16_t id;
    uint64_t mac;
    uint8_t addr;
    uint16_t rect_x;
    uint16_t rect_y;
    uint16_t rect_width;
    uint16_t rect_height;
    uint16_t center_x;
    uint16_t center_y;
    feature_vector *feature;
    struct high_res_packet *up;
    struct high_res_packet *down;
    struct high_res_packet *left;
    struct high_res_packet *right;
    uint8_t state;
    struct high_res_packet *next;
    struct high_res_packet *prev;
} patch_packet;

struct topo_list{
    uint8_t x;
    uint8_t y;
    uint64_t mac;
};

struct topo_header{
    uint8_t port;
    uint8_t sizex;
    uint8_t sizey;
};

struct packet{
    size_t size;
    uint32_t addr;
    uint8_t broadcast;
    uint32_t id;
    void *buffer;
    struct packet *next;
};


#endif /* NETWORK_DEFINES_H */

