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

class Topology {
public:
    Topology(UnreliableTransfer **unrel);
    virtual ~Topology();
    int recv(void *buffer, size_t size, uint8_t addr);
    int send();
    int isalive(uint8_t addr);
private:
    UnreliableTransfer **unrel;
    uint64_t mac;
    struct timespec alive[4];
    uint64_t mapping[4];
};

#endif /* TOPOLOGY_H */

