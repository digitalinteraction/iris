/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "Buffer.h"
#include "RaspiTex.h"


Buffer::Buffer(int size){
    array = new Buffer_Item[size];
    this->size = size;
    for(int i = 0; i< size;i++){
        array[i].patch = 0;
        array[i].status = 0;
        array[i].lock.unlock();
    }
    
    add_pos = 0;
    get_pos = 0;
}

Buffer::~Buffer(){
    delete[] array;
}

int Buffer::add(RASPITEX_PATCH *patch){
    add_pos++;
    if(add_pos == size){
        add_pos = 0;
    }
    
    array[add_pos].lock.lock();
    if(array[add_pos].status == 0){
        array[add_pos].patch = patch;
        array[add_pos].status = 1;
        //printf("added buffer %d\n", add_pos);
    }else{
        array[add_pos].lock.unlock();
        add_pos--;
        return 1;
    }
    array[add_pos].lock.unlock();
    return 0;
}

int Buffer::free_space(){
    int temp_pos = add_pos+1;
    if(temp_pos == size){
        temp_pos = 0;
    }
    
    if(array[temp_pos].status == 0){
        return 0;
    }
    
    return 1;
}

int Buffer::get(RASPITEX_PATCH **patch){
    get_pos++;
    if(get_pos == size){
        get_pos = 0;
    }
    
    array[get_pos].lock.lock();
    if(array[get_pos].status == 1){
        *patch = array[get_pos].patch;
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
        free(array[get_pos].patch->buffer);
        free(array[get_pos].patch);
        array[get_pos].patch = 0;
        array[get_pos].status = 0;
    }else{
        array[get_pos].lock.unlock();
        return 1;
    }

    array[get_pos].lock.unlock();
    return 0;
}