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
    unsigned char a=0,b=0,c=0,d=0,e=0,f=0;
    fscanf(file, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &a, &b, &c, &d, &e, &f);
    fclose(file);
    mac = 0;
    mac = (((uint64_t)a)<<40)|(((uint64_t)b)<<32)|(((uint64_t)c)<<24)|(((uint64_t)d)<<16)|(((uint64_t)e)<<8)|(((uint64_t)f)<<0);
    printf("Topology:: got MAC %lx\n", mac);
    alive[0].tv_sec = 0;
    alive[1].tv_sec = 0;
    alive[2].tv_sec = 0;
    alive[3].tv_sec = 0;
    mapping[0] = 0;
    mapping[1] = 0;
    mapping[2] = 0;
    mapping[3] = 0;
}


Topology::~Topology() {
}

//call this function every 250ms
int Topology::send(){
    
#ifdef DEBUG
    printf("Topology::Sending requests\n");
#endif
    
    struct topo_buffer *buf0 = (struct topo_buffer*) malloc(sizeof (struct topo_buffer));
    struct topo_buffer *buf1 = (struct topo_buffer*) malloc(sizeof (struct topo_buffer));
    struct topo_buffer *buf2 = (struct topo_buffer*) malloc(sizeof (struct topo_buffer));
    struct topo_buffer *buf3 = (struct topo_buffer*) malloc(sizeof (struct topo_buffer));

    buf0->mac = mac;
    buf1->mac = mac;
    buf2->mac = mac;
    buf3->mac = mac;

    buf0->addr = 0;//2;
    (*unrel)->send((void*) buf0, sizeof (struct topo_buffer), 1, 0);

    buf1->addr = 1;//3;
    (*unrel)->send((void*) buf1, sizeof (struct topo_buffer), 1, 1);

    buf2->addr = 2;//0;
    (*unrel)->send((void*) buf2, sizeof(struct topo_buffer), 1, 2);
    
    buf3->addr = 3;//1;
    (*unrel)->send((void*) buf3, sizeof(struct topo_buffer), 1, 3);
    free(buf0);
    free(buf1);
    free(buf2);
    free(buf3);
}

int Topology::recv(void* buffer, size_t size, uint8_t addr) {

    //printf("Topology:: received an alive packet at time %ld from addr %d\n", temp.tv_sec, addr);
    struct topo_buffer *buf = (struct topo_buffer *) buffer;
    printf("Topology: received a keep alive packet from %d on port %d\n", buf->addr, addr);
    printf("MAC: %x\n", buf->mac);
    if (addr < 4) {
        if (mapping[addr] == 0 || mapping[addr] == buf->mac) {
            mapping[addr] = buf->mac;
            struct timespec current;
            clock_gettime(CLOCK_REALTIME, &current);
            alive[addr].tv_sec = current.tv_sec;
        } else {
            printf("Error Topology:: Conflicting MACs for the same spot in mapping\n");
        }
    }
    //printf("t free %p\n", buffer);
}

int Topology::isalive(uint8_t addr){
    struct timespec current;
    clock_gettime(CLOCK_REALTIME, &current);
    if(current.tv_sec <= (alive[addr].tv_sec+2)){
        return 1;
    }else{
        return 0;
    }
}