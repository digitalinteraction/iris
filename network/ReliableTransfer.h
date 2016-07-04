/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ReliableTransfer.h
 * Author: tobias
 *
 * Created on June 27, 2016, 4:28 PM
 */

#ifndef RELIABLETRANSFER_H
#define RELIABLETRANSFER_H

#include <stdint.h>
#include "UnreliableTransfer.h"
#include <mutex>
#include "Topology.h"

//class UnreliableTransfer;

struct reliable_packet{
    uint32_t id;
    uint8_t broadcast;
    uint8_t ack;
    uint16_t filler;
};

struct linked_header{
    struct reliable_packet* packet;
    struct timespec timeout;
    uint8_t addr;
    struct linked_header* next;
    struct linked_header* prev;
    size_t size;
    uint8_t resent_time;
    
};

class ReliableTransfer {
public:
    ReliableTransfer(UnreliableTransfer **unrel, Packetbuffer *out, Topology *topo);
    ReliableTransfer(const ReliableTransfer& orig);
    virtual ~ReliableTransfer();
    int recv(void *buffer, size_t size, uint8_t addr);
    uint32_t send(void *buffer, size_t size, uint8_t addr, uint8_t broadcast);
    int check_timeouts();
    int send_acks();
    volatile uint32_t list_cnt;
private:
    Packetbuffer *out;
    Topology *topo;
    UnreliableTransfer **unrel;
    struct linked_header*first;
    struct linked_header*last;
    uint16_t seq;
    uint32_t last_broadcast;
    struct reliable_packet ack;
    int timer;
    std::mutex list_lock;
};

#endif /* RELIABLETRANSFER_H */

