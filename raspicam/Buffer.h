/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Buffer.h
 * Author: tobias
 *
 * Created on May 2, 2016, 4:16 PM
 */
#include <iostream>       // std::cout
#include <thread>         // std::thread
#include <mutex>          // std::mutex
#include "RaspiTex.h"


typedef struct{
    RASPITEX_PATCH *patch;
    std::mutex lock;
    int status;
} Buffer_Item;


class Buffer{
public:
    Buffer(int size);
    ~Buffer();
    int add(RASPITEX_PATCH *patch);
    int get(RASPITEX_PATCH **patch);
    int free_space();
    int release();
private:
    Buffer_Item *array;
    int add_pos;
    int get_pos;
    int size;
};