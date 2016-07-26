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

#define CLIENT_SIDE
#define IMAGE_PACKET 1
#define TOPO_PACKET 2
#define CONTROL_PACKET 3
#define MAX_PACKET_SIZE 60000
#define BORDER_HIGH_RES 10

#define HISTOGRAM_SIZE 32


struct low_res_header{
    uint8_t port;
    uint8_t pos;
    uint64_t mac;
    uint32_t size;
    double weight;
};

struct high_res_packet{
    double contourArea;
    double contourPerimeter;
    double Hu[7];
    uint32_t hist_r[HISTOGRAM_SIZE];
    uint32_t hist_g[HISTOGRAM_SIZE];
    uint32_t hist_b[HISTOGRAM_SIZE];
    uint16_t rect_x;
    uint16_t rect_y;
    uint16_t rect_width;
    uint16_t rect_height;
    uint16_t center_x;
    uint16_t center_y;
};

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
    void *buffer;
    struct packet *next;
};


#endif /* NETWORK_DEFINES_H */

