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

Packetbuffer::Packetbuffer() {
    signalfd = eventfd(0, 0);
    first = 0;
    last = 0;
    cnt = 0;
    lock.unlock();
    id = rand()%20;
#ifdef DEBUG
    printf("Packetbuffer:: startup completed\n");
#endif
}

Packetbuffer::~Packetbuffer() {
}

int Packetbuffer::add(uint32_t size, uint8_t addr, void* buffer) {
#ifdef DEBUG
    //printf("Packetbuffer:: adding item size %d\n", size); fflush(stdout);
#endif
    //printf("Packetbuffer %d:: adding item size %d\n", id, size); fflush(stdout);

    lock.lock();
    //printf("Packetbuffer:: %.*s\n", size, (char*)buffer);
    struct packet * pack = (struct packet *) malloc(sizeof (struct packet));
    //printf("p malloc %p\n", pack);
    pack->addr = addr;
    pack->size = size;
    pack->buffer = (void*) malloc(pack->size);
    pack->next = 0;
    memcpy(pack->buffer, buffer, pack->size);

    if (first == 0) {
        first = pack;
        last = pack;
    } else {
        last->next = pack;
        last = pack;
    }
    cnt++;
    uint64_t u = 1;
#ifdef DEBUG
    //printf("Packetbuffer:: signal to %x that something got added to buffer\n", signalfd);
#endif
    lock.unlock();
    write(signalfd, &u, sizeof (uint64_t));
    return 0;
}

int Packetbuffer::get(struct packet** pack) {
    //printf("Packetbuffer %d:: one packet delivered\n", id);
#ifdef DEBUG
   //printf("someone called get\n");
#endif
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
