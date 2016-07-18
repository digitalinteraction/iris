/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Packetbuffer.h
 * Author: tobias
 *
 * Created on June 27, 2016, 2:13 PM
 */

#ifndef PACKETBUFFER_H
#define PACKETBUFFER_H

#include <mutex>          // std::mutex
#include <stdint.h>
//#define DEBUG

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

class Packetbuffer {
public:
    Packetbuffer(uint8_t id);
    virtual ~Packetbuffer();
    int add(uint32_t size, uint32_t addr, void *buffer);
    int get(struct packet ** pack);
    int signalfd;
    uint16_t getCnt();
private:
    struct packet *first;
    struct packet *last;
    uint16_t cnt;
    std::mutex lock;
    uint8_t id;
};

#endif /* PACKETBUFFER_H */

