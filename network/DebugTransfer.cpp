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

DebugTransfer::DebugTransfer(Packetbuffer *out, Packetbuffer *in) {
    this->out = out;
    this->in = in;
}

DebugTransfer::DebugTransfer(const DebugTransfer& orig) {
}

DebugTransfer::~DebugTransfer() {
}

int DebugTransfer::recv(void* buffer, size_t size, uint32_t addr){
    //free(buffer);
    out->add(size, addr, buffer);
    printf("DebugTransfer: recv packet %ld %ld\n", size, addr);
    return 0;
}

int DebugTransfer::send(){
    printf("Error DebugTransfer:: send not implemented yet\n");
}
