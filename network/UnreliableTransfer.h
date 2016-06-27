/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   UnreliableTransfer.h
 * Author: tobias
 *
 * Created on June 27, 2016, 2:53 PM
 */

#ifndef UNRELIABLETRANSFER_H
#define UNRELIABLETRANSFER_H
#include "Packetbuffer.h"
#include "SerialCon.h"
#include "ReliableTransfer.h"
#include "Topology.h"
#include "DebugTransfer.h"

struct unreliable_packet{
    size_t size;
    uint8_t port;
    uint8_t addr;
    crc crc;
    void *buffer;
};

class UnreliableTransfer {
public:
    UnreliableTransfer(ReliableTransfer *rel, Topology *topo, DebugTransfer *debug);
    UnreliableTransfer(const UnreliableTransfer& orig);
    virtual ~UnreliableTransfer();
    int send(void* buffer, size_t size, uint8_t port, uint8_t addr);
    
private:
    SerialCon *sercon;
    Packetbuffer *send;
    Packetbuffer *recv;
    
    ReliableTransfer *rel;
    Topology *topo;
    DebugTransfer *debug;
    
    int recv();
};

#endif /* UNRELIABLETRANSFER_H */

