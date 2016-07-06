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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

Topology::Topology(UnreliableTransfer **unrel, uint8_t deb) {
    this->unrel = unrel;
    FILE * file = fopen("/sys/class/net/wlan0/address", "r");
    unsigned char a=0,b=0,c=0,d=0,e=0,f=0;
    fscanf(file, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &a, &b, &c, &d, &e, &f);
    fclose(file);
    mac = 0;
    mac = (((uint64_t)a)<<40)|(((uint64_t)b)<<32)|(((uint64_t)c)<<24)|(((uint64_t)d)<<16)|(((uint64_t)e)<<8)|(((uint64_t)f)<<0);
    printf("Topology:: got MAC %llx\n", mac);
    alive[0].tv_sec = 0;
    alive[1].tv_sec = 0;
    alive[2].tv_sec = 0;
    alive[3].tv_sec = 0;
    mapping[0] = 0;
    mapping[1] = 0;
    mapping[2] = 0;
    mapping[3] = 0;
    
    topo_buf.addr = 0;
    topo_buf.mac = mac;
    this->deb = deb;
    
    
}


Topology::~Topology() {
}

int Topology::send(){
    if (deb == 0) {
        (*unrel)->send((void*) &topo_buf, sizeof (struct topo_buffer), 1, 0);

        (*unrel)->send((void*) &topo_buf, sizeof (struct topo_buffer), 1, 1);

        (*unrel)->send((void*) &topo_buf, sizeof (struct topo_buffer), 1, 2);

        (*unrel)->send((void*) &topo_buf, sizeof (struct topo_buffer), 1, 3);
    }
}

int Topology::recv(void* buffer, size_t size, uint32_t addr) {

    struct topo_buffer *buf = (struct topo_buffer *) buffer;
    if (deb == 0) {
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
    }else{
        printf("got mac list from %d\n", addr);
        //do something else
    }
    
}

int Topology::isalive(uint32_t addr){
    struct timespec current;
    clock_gettime(CLOCK_REALTIME, &current);
    if(current.tv_sec <= (alive[addr].tv_sec+2)){
        return 1;
    }else{
        return 0;
    }
}

int Topology::sendlist() {
    if (deb == 0) {
        uint32_t addr;
        if (inet_aton("172.16.0.1", (in_addr *) & addr) == 0) {
            printf("inet_aton() failed\n");
        }
        (*unrel)->send((void*) &mapping, sizeof (mapping), 1, addr);
    }
}