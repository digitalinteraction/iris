/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Topology.h
 * Author: tobias
 *
 * Created on June 27, 2016, 4:28 PM
 */

#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include "UnreliableTransfer.h"


//class UnreliableTransfer;
struct topo_buffer{
    uint64_t mac;
    uint8_t addr;
};

struct packet_map{
    uint64_t mac;
    uint64_t right;
    uint64_t left;
    uint64_t up;
    uint64_t down;
};


class Topology {
public:
    Topology(UnreliableTransfer **unrel);
    virtual ~Topology();
    int recv(void *buffer, size_t size, uint32_t addr);
    int send();
    int isalive(uint32_t addr);
    int sendlist();
private:
    UnreliableTransfer **unrel;
    uint64_t mac;
    struct timespec alive[4];
    uint64_t mapping[4];
    struct topo_buffer topo_buf;
    void print_mapping(struct packet_map* map);
    struct packet_map map;
#ifndef CLIENT_SIDE
    Packetbuffer *in_map;
    
#endif
};

#endif /* TOPOLOGY_H */

