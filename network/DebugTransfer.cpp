/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   DebugTransfer.cpp
 * Author: tobias
 * 
 * Created on June 27, 2016, 4:28 PM
 */

#include "DebugTransfer.h"

DebugTransfer::DebugTransfer(Packetbuffer *out, UnreliableTransfer **unrel) {
    this->out = out;
    this->unrel = unrel;
}

DebugTransfer::DebugTransfer(const DebugTransfer& orig) {
}

DebugTransfer::~DebugTransfer() {
}

int DebugTransfer::recv(void* buffer, size_t size, uint32_t addr){
    //free(buffer);
    out->add(size, addr, buffer);
    //printf("DebugTransfer: recv packet %ld %ld\n", size, addr);
    return 0;
}

int DebugTransfer::send(struct packet* pack){
    if((*unrel)->send(pack->buffer, pack->size, 0, pack->addr) != 0){
        printf("Error DebugTransfer: could not send unreliable packet\n");
    }
    return 0;
}
