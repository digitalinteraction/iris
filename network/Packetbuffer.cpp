/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Packetbuffer.cpp
 * Author: tobias
 * 
 * Created on June 27, 2016, 2:13 PM
 */

#include "Packetbuffer.h"
#include <sys/eventfd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <cstring>

Packetbuffer::Packetbuffer(uint8_t id) {
    signalfd = eventfd(0, EFD_SEMAPHORE);
    first = 0;
    last = 0;
    cnt = 0;
    lock.unlock();
    this->id = id;
    packet_id = 1;
#ifdef DEBUG
    printf("Packetbuffer:: startup completed\n");
#endif
}

Packetbuffer::~Packetbuffer() {
}

int32_t Packetbuffer::add(uint32_t size, uint32_t addr, void* buffer) {
    lock.lock();
    if(size <= 0 || buffer == 0 || cnt > 100){
        //printf("Error Packetbuffer add: size %d, addr %d, buffer %p, cnt %d, id %d\n", size, addr, buffer, cnt, id);
        lock.unlock();
        return -1;
    }
    
    struct packet * pack = (struct packet *) malloc(sizeof (struct packet));
    pack->addr = addr;
    pack->size = size;
    pack->buffer = (void*) malloc(pack->size);
    pack->next = 0;
    pack->id = packet_id;
    memcpy(pack->buffer, buffer, pack->size);

    if (first == 0) {
        first = pack;
        last = pack;
    } else {
        last->next = pack;
        last = pack;
    }
    cnt++;
    packet_id++;
    if(packet_id < 1){
        packet_id = 1;
    }
    
    uint64_t u = 1;

    write(signalfd, &u, sizeof (uint64_t));
    lock.unlock();

    return packet_id;
}

int32_t Packetbuffer::add(uint32_t size, uint32_t addr, void* buffer, uint8_t broadcast) {
    lock.lock();
    if(size <= 0 || buffer == 0 || cnt > 100){
        //printf("Error Packetbuffer add: size %d, addr %d, buffer %p, cnt %d, id %d\n", size, addr, buffer, cnt, id);
        lock.unlock();
        return -1;
    }
    
    struct packet * pack = (struct packet *) malloc(sizeof (struct packet));
    pack->broadcast = broadcast;
    pack->addr = addr;
    pack->size = size;
    pack->buffer = (void*) malloc(pack->size);
    pack->next = 0;
    pack->id = packet_id;
    memcpy(pack->buffer, buffer, pack->size);

    if (first == 0) {
        first = pack;
        last = pack;
    } else {
        last->next = pack;
        last = pack;
    }
    cnt++;
    packet_id++;
    if(packet_id < 0){
        packet_id = 0;
    }
    
    uint64_t u = 1;

    write(signalfd, &u, sizeof (uint64_t));
    lock.unlock();

    return packet_id;
}

int Packetbuffer::get(struct packet** pack) {
    lock.lock();
    if (first == 0) {
        lock.unlock();
        return -1;
    } else {
        *pack = first;
        first = first->next;
        cnt--;
    }
    lock.unlock();
    return 0;
}

uint16_t Packetbuffer::getCnt(){
    return cnt;
}
    
