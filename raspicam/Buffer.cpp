/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "Buffer.h"


Buffer::Buffer(int size){
    array = new Buffer_Item[size];
    this->size = size;
    for(int i = 0; i< size;i++){
        array[i].buffer = 0;
        array[i].buffer_size = 0;
        array[i].id = i;
        array[i].status = 0;
        array[i].lock.unlock();
    }
    
    add_pos = 0;
    get_pos = 0;
}

Buffer::~Buffer(){
    delete[] array;
}

int Buffer::add(uint8_t *buffer, size_t buffer_size, int light){
    add_pos++;
    if(add_pos == size){
        add_pos = 0;
    }
    
    array[add_pos].lock.lock();
    if(array[add_pos].status == 0){
        array[add_pos].buffer = buffer;
        array[add_pos].buffer_size = buffer_size;
        array[add_pos].status = 1;
        array[add_pos].light = light;
        //printf("added buffer %d\n", add_pos);
    }else{
        array[add_pos].lock.unlock();
        add_pos--;
        return 1;
    }
    array[add_pos].lock.unlock();
    return 0;
}

int Buffer::get(uint8_t **buffer, size_t *buffer_size, int *light){
    get_pos++;
    if(get_pos == size){
        get_pos = 0;
    }
    
    array[get_pos].lock.lock();
    if(array[get_pos].status == 1){
        *buffer = array[get_pos].buffer;
        *buffer_size = array[get_pos].buffer_size;
        *light = array[get_pos].light;
        array[get_pos].status = 2;
        //printf("gotten buffer %d\n", get_pos);
    }else{
        array[get_pos].lock.unlock();
        get_pos--;
        return 1;
    }
    array[get_pos].lock.unlock();
    return 0;
}

int Buffer::release(){
    array[get_pos].lock.lock();
    if(array[get_pos].status == 2){
        free(array[get_pos].buffer);
        array[get_pos].buffer = 0;
        array[get_pos].buffer_size = 0;
        array[get_pos].status = 0;
        //printf("release buffer %d\n", get_pos);
    }else{
        array[get_pos].lock.unlock();
        return 1;
    }

    array[get_pos].lock.unlock();
    return 0;
}