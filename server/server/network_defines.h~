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

struct low_res_header{
    uint8_t port;
    uint8_t pos;
    uint64_t mac;
    uint32_t size;
    double weight;
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

