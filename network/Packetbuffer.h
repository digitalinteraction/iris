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
#define DEBUG

struct packet{
    size_t size;
    uint8_t addr;
    void *buffer;
    struct packet *next;
};

class Packetbuffer {
public:
    Packetbuffer();
    virtual ~Packetbuffer();
    int add(uint32_t size, uint8_t addr, void *buffer);
    int get(struct packet ** pack);
    int signalfd;
private:
    struct packet *first;
    struct packet *last;
    uint16_t cnt;
    std::mutex lock;
};

#endif /* PACKETBUFFER_H */
