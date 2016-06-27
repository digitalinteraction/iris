/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ReliableTransfer.h
 * Author: tobias
 *
 * Created on June 27, 2016, 4:28 PM
 */

#ifndef RELIABLETRANSFER_H
#define RELIABLETRANSFER_H

class ReliableTransfer {
public:
    ReliableTransfer();
    ReliableTransfer(const ReliableTransfer& orig);
    virtual ~ReliableTransfer();
    int recv(void *buffer, size_t size);
private:

};

#endif /* RELIABLETRANSFER_H */

