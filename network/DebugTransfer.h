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

class UnreliableTransfer;

class DebugTransfer {
public:
    DebugTransfer();
    DebugTransfer(const DebugTransfer& orig);
    virtual ~DebugTransfer();
    int recv(void *buffer, size_t size, uint8_t addr);
    int send();
private:

};

#endif /* DEBUGTRANSFER_H */

