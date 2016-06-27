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


class UnreliableTransfer;

class Topology {
public:
    Topology();
    Topology(const Topology& orig);
    virtual ~Topology();
    int recv(void *buffer, size_t size);
    int send();
private:
    UnreliableTransfer *unrel;
    uint64_t mac;
    
    uint64_t mapping[4];
};

#endif /* TOPOLOGY_H */

