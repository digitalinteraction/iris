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

Topology::Topology(UnreliableTransfer **unrel) {
    this->unrel = unrel;
    FILE * file = fopen("/sys/class/net/wlan0/address", "r");
    uint64_t a,b,c,d,e,f;
    fscanf(file, "%lx:%lx:%lx:%lx:%lx:%lx", &a, &b, &c, &d, &e, &f);
    fclose(file);
    mac = 0;
    mac = (a<<40)|(b<<32)|(c<<24)|(d<<16)|(e<<8)|(f<<0);
    printf("Topology:: got MAC %lx\n", mac);
}


Topology::~Topology() {
}

//call this function every 250ms
int Topology::send(){
    
#ifdef DEBUG
    printf("Topology::Sending requests\n");
#endif
    mapping[0] = 0;
    mapping[1] = 0;
    mapping[2] = 0;
    mapping[3] = 0;

    struct topo_buffer *buf0 = (struct topo_buffer*) malloc(sizeof (struct topo_buffer));
    struct topo_buffer *buf1 = (struct topo_buffer*) malloc(sizeof (struct topo_buffer));
    struct topo_buffer *buf2 = (struct topo_buffer*) malloc(sizeof (struct topo_buffer));
    struct topo_buffer *buf3 = (struct topo_buffer*) malloc(sizeof (struct topo_buffer));

    buf0->mac = mac;
    buf1->mac = mac;
    buf2->mac = mac;
    buf3->mac = mac;
#ifdef DEBUG
    printf("Topology:: sending %p %ld\n", buf, sizeof (struct topo_buffer));
#endif
    //printf("Topology:: Sending 4 packets\n");
    buf0->addr = 2;
    (*unrel)->send((void*) buf0, sizeof (struct topo_buffer), 1, 0);

    buf1->addr = 3;
    (*unrel)->send((void*) buf1, sizeof (struct topo_buffer), 1, 1);

    buf2->addr = 0;
    (*unrel)->send((void*) buf2, sizeof(struct topo_buffer), 1, 2);
    
    buf3->addr = 1;
    (*unrel)->send((void*) buf3, sizeof(struct topo_buffer), 1, 3);
}

int Topology::recv(void* buffer, size_t size, uint8_t addr) {

    //printf("Topology:: received an alive packet at time %ld from addr %d\n", temp.tv_sec, addr);
    struct topo_buffer *buf = (struct topo_buffer *) buffer;
    if (addr < 4) {
        if (mapping[buf->addr] == 0 || mapping[buf->addr] == buf->mac) {
            mapping[buf->addr] = buf->mac;
            clock_gettime(CLOCK_REALTIME, &(alive[buf->addr]));
        } else {
            printf("Error Topology:: Conflicting MACs for the same spot in mapping\n");
        }
    }
    //printf("t free %p\n", buffer);
}

int Topology::isalive(uint8_t addr){
    struct timespec current;
    clock_gettime(CLOCK_REALTIME, &current);
    if(current.tv_sec < (alive[addr].tv_sec+5)){
        return 1;
    }else{
        return 0;
    }
}