/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   High_Res_Worker.h
 * Author: tobias
 *
 * Created on May 12, 2016, 4:12 PM
 */

#ifndef HIGH_RES_WORKER_H
#define HIGH_RES_WORKER_H
#include "Buffer.h"

class High_Res_Worker {
public:
    High_Res_Worker(Buffer *buffer);
    virtual ~High_Res_Worker();
    int processing;
    void run();

private:
    Buffer * buf;
    int cnt;


};

#endif /* HIGH_RES_WORKER_H */

