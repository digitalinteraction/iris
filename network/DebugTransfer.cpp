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
#include <linux/i2c-dev.h>
#include <fcntl.h>

DebugTransfer::DebugTransfer(Packetbuffer *out, UnreliableTransfer **unrel) {
    this->out = out;
    this->unrel = unrel;
    i2cfile = open("/dev/i2c-1", O_RDWR);
    ioctl(i2cfile, I2C_SLAVE, 0x48);
    for(int i = 0; i < WEIGHT_ARRAY; i++){
        array0[i] = 0;
        array1[i] = 0;
        array2[i] = 0;
        array3[i] = 0;
    }
    count0 = 0; count1 = 0; count2 = 0; count3 = 0;
    
    for(int i = 0; i < WEIGHT_ARRAY; i++){
        update_weight();
    }
    baseline = (median(array0)+median(array1)+median(array2)+median(array3))/4;
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

double DebugTransfer::get_weight(){
    uint16_t temp_array[WEIGHT_ARRAY];
    memcpy(temp_array, array0, WEIGHT_ARRAY);
    uint16_t ch0 = median(temp_array);
    memcpy(temp_array, array1, WEIGHT_ARRAY);
    uint16_t ch1 = median(temp_array);
    memcpy(temp_array, array2, WEIGHT_ARRAY);
    uint16_t ch2 = median(temp_array);
    memcpy(temp_array, array3, WEIGHT_ARRAY);
    uint16_t ch3 = median(temp_array);
    
    uint32_t sum = ch0+ch1+ch2+ch3;
    
    sum = sum - baseline;
    if(sum < 0)
        sum = 0;
    
    double x = (double)sum;
    double f0 = 1.3782624288086005E+000;
    double f1 = 1.4627313573521233E-002;
    double f2 = 7.6822624506915741E-007;
    double weight = f0 + f1*x + f2*pow(x, 2.0);
    return weight;
}

//call this function every 10ms or something like that
uint16_t DebugTransfer::update_weight(){
    read_channel(i2cfile, 0, array0, &count0);
    read_channel(i2cfile, 1, array1, &count1);
    read_channel(i2cfile, 2, array2, &count1);
    read_channel(i2cfile, 3, array3, &count1);
}

void DebugTransfer::read_channel(int I2CFile, uint8_t sel, uint16_t *array, uint16_t *count) {

    
    uint16_t val;
    writeBuf[0] = 1; 
   
    //0b1100 [001] 1 -> 001 (4.096V) -> 000 (6.144V)
    switch (sel) {
        case 0: writeBuf[1] = 0b11000011;
            break;
        case 1: writeBuf[1] = 0b11010011;
            break;
        case 2: writeBuf[1] = 0b11100011;
            break;
        case 3: writeBuf[1] = 0b11110011;
            break;
    }
    writeBuf[2] = 0b11100011; 

    readBuf[0] = 0;
    readBuf[1] = 0;

    write(I2CFile, writeBuf, 3);

    while ((readBuf[0] & 0x80) == 0) 
    {
        read(I2CFile, readBuf, 2); 
    }

    writeBuf[0] = 0;
    write(I2CFile, writeBuf, 1);

    read(I2CFile, readBuf, 2);

    val = (readBuf[0] << 8) | readBuf[1];
    
    array[*count] = val;
    *count++;
    if(*count == WEIGHT_ARRAY){
        *count = 0;
    }
    
}

int DebugTransfer::intcmp(const void *aa, const void *bb)
{
    const uint16_t *a = aa, *b = bb;
    return (*a < *b) ? -1 : (*a > *b);
}

uint16_t DebugTransfer::median(uint16_t *array){
	qsort(array, WEIGHT_ARRAY, sizeof(uint16_t), intcmp);
	return (array[WEIGHT_ARRAY/2] + array[WEIGHT_ARRAY/2+1])/2;
}