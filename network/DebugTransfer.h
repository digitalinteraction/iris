/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   DebugTransfer.h
 * Author: tobias
 *
 * Created on June 27, 2016, 4:28 PM
 */

#ifndef DEBUGTRANSFER_H
#define DEBUGTRANSFER_H
#include <stdint.h>
#include "UnreliableTransfer.h"

class DebugTransfer {
public:
    DebugTransfer(Packetbuffer *out, Packetbuffer *in);
    DebugTransfer(const DebugTransfer& orig);
    virtual ~DebugTransfer();
    int recv(void *buffer, size_t size, uint32_t addr);
    int send();
private:
    Packetbuffer *out;
    Packetbuffer *in;

};

#endif /* DEBUGTRANSFER_H */

