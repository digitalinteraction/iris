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

#include "network_defines.h"

class Packetbuffer {
public:
    Packetbuffer(uint8_t id);
    virtual ~Packetbuffer();
    int32_t add(uint32_t size, uint32_t addr, void *buffer);
    int32_t add(uint32_t size, uint32_t addr, void *buffer, uint8_t broadcast);

    int get(struct packet ** pack);
    int signalfd;
    uint16_t getCnt();
private:
    struct packet *first;
    struct packet *last;
    uint16_t cnt;
    std::mutex lock;
    uint8_t id;
    int32_t packet_id;
};

#endif /* PACKETBUFFER_H */

