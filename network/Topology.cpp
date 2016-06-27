/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Topology.cpp
 * Author: tobias
 * 
 * Created on June 27, 2016, 4:28 PM
 */

#include "Topology.h"
struct topo_buffer{
    uint64_t mac;
    uint8_t addr;
};

Topology::Topology(UnreliableTransfer *unrel) {
    this->unrel = unrel;
    FILE * file = fopen("/sys/class/net/wlan0/address", "r");
    uint8_t a,b,c,d,e,f;
    fscanf(file, "%x:%x:%x:%x:%x:%x", &a, &b, &c, &d, &e, &f);
    fclose(file);
    mac = 0;
    mac = (a<<40)|(b<<32)|(c<<24)|(d<<16)|(e<<8)|(f<<0);
    mapping[0] = 0;
    mapping[1] = 0;
    mapping[2] = 0;
    mapping[3] = 0;
}

Topology::Topology(const Topology& orig) {
}

Topology::~Topology() {
}

int Topology::send(){
    struct topo_buffer *buf = (struct topo_buffer*) malloc(sizeof(struct topo_buffer));
    buf->mac = mac;
    
    buf->addr = 2;
    unrel->send((void*) buf, sizeof(struct topo_buffer), 1, 0);
    
    buf->addr = 3;
    unrel->send((void*) buf, sizeof(struct topo_buffer), 1, 1);
    
    buf->addr = 0;
    unrel->send((void*) buf, sizeof(struct topo_buffer), 1, 2);
    
    buf->addr = 1;
    unrel->send((void*) buf, sizeof(struct topo_buffer), 1, 3);
}

int Topology::recv(void* buffer, size_t size) {
    struct topo_buffer *buf = (struct topo_buffer *) buffer;
    if (addr < 4) {
        if (mapping[buf->addr] == 0 || mapping[buf->addr] == buf->mac) {
            mapping[buf->addr] = buf->mac;
        } else {
            printf("Error Topology:: Conflicting MACs for the same spot in mapping\n");
        }
    }
    free(buffer);
}