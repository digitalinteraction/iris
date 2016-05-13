/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   High_Res_Worker.cpp
 * Author: tobias
 * 
 * Created on May 12, 2016, 4:12 PM
 */

#include "High_Res_Worker.h"
#include "tga.h"

High_Res_Worker::High_Res_Worker(Buffer *buffer) {
    buf = buffer;
    cnt = 0;
}

High_Res_Worker::~High_Res_Worker() {
}

void High_Res_Worker::run(){
    while(processing){
        RASPITEX_PATCH *patch;
        if(buf->get(&patch) == 0){
            printf("got patch %d %d %d\n", patch->width, patch->height, patch->size);
            char tmp[] = "testing00.tga";
            tmp[8] = (cnt % 10)+'0';
            tmp[7] = (cnt/10)+'0';
            FILE *fp = fopen(tmp, "wb");
            write_tga(fp, patch->height, patch->width, patch->buffer, patch->size);
            fclose(fp);
            cnt++;
            
            buf->release();
        }
    }
}