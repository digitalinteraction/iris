/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "Buffer.h"
#include "RaspiTex.h"


Buffer::Buffer(int size){
    this->size = size;
    first = 0;
    last = 0;
    cnt = 0;
    lock.unlock();
}

Buffer::~Buffer(){
}

int Buffer::add(RASPITEX_PATCH *patch, uint8_t group){
    lock.lock();
    if(patch == 0 || cnt > size){
        lock.unlock();
        return -1;
    }
    
    Buffer_Item * pack = (Buffer_Item *) malloc(sizeof (Buffer_Item));
    pack->patch = patch;
    pack->group = group;
    pack->next = 0;

    if (first == 0) {
        first = pack;
        last = pack;
    } else {
        last->next = pack;
        last = pack;
    }
    cnt++; 
    lock.unlock();

    return 0;
}


int Buffer::get(RASPITEX_PATCH **patch, uint8_t *group){
    lock.lock();
    if (first == 0) {
        lock.unlock();
        return -1;
    } else {
        Buffer_Item *temp = first;
        *patch = first->patch;
        *group = first->group;
        first = first->next;
        free(temp);
        cnt--;
    }
    lock.unlock();
    return 0;
}

uint8_t Buffer::getSize(){
    return cnt;
}