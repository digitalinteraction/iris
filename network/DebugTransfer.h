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

#define WEIGHT_ARRAY 256

class DebugTransfer {
public:
    DebugTransfer(Packetbuffer *out, UnreliableTransfer **unrel);
    DebugTransfer(const DebugTransfer& orig);
    virtual ~DebugTransfer();
    int recv(void *buffer, size_t size, uint32_t addr);
    int send(struct packet *pack);
    double get_weight();
    void update_weight();

private:
    Packetbuffer *out;
    UnreliableTransfer **unrel;
    void read_channel(int I2CFile, uint8_t sel, uint16_t *array, uint16_t *count);
    static int intcmp(const void *aa, const void *bb);
    uint16_t median(uint16_t *array);
    uint8_t writeBuf[3];
    uint8_t readBuf[2];
    int i2cfile;
    uint16_t array0[WEIGHT_ARRAY];
    uint16_t array1[WEIGHT_ARRAY];
    uint16_t array2[WEIGHT_ARRAY];
    uint16_t array3[WEIGHT_ARRAY];
    uint16_t count0;
    uint16_t count1;
    uint16_t count2;
    uint16_t count3;
    uint32_t baseline;
    


};

#endif /* DEBUGTRANSFER_H */

