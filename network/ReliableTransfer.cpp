/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ReliableTransfer.cpp
 * Author: tobias
 * 
 * Created on June 27, 2016, 4:28 PM
 */

#include "ReliableTransfer.h"
#include "UnreliableTransfer.h"
#include <time.h>
#include <stdio.h>
#include <string.h>


#define TIMEOUT   2;

ReliableTransfer::ReliableTransfer(UnreliableTransfer **unrel, Packetbuffer *out, Topology *topo) {
    this->unrel = unrel;
    first = 0;
    last = 0;
    seq = 5;
    ack.ack = 1;
    ack.broadcast = 0;
    ack.filler = 0;
    ack.id = 0;
    last_broadcast = 0;
    this->topo = topo;
    list_lock.unlock();
    this->out = out;
}

ReliableTransfer::ReliableTransfer(const ReliableTransfer& orig) {
}

ReliableTransfer::~ReliableTransfer() {
}

int ReliableTransfer::recv(void* buffer, size_t size, uint8_t addr) {
    if(buffer == 0 || size <= 0){
        printf("Error ReliableTransfer: buffer or size is wrong\n");
        return -1;
    }
    struct reliable_packet *header = (struct reliable_packet*) buffer;

    if (header->ack == 1) {
        list_lock.lock();
        struct linked_header *list_item = first;
        uint8_t success = 0;
        
        if(header->broadcast == 1 && header->id == last_broadcast){
            success = 1;
        }
        while (list_item != 0 && success != 1) {
            if (list_item->packet->id == header->id) {
                success = 1;
                if (list_item->prev == 0 && list_item->next == 0) {
                    first = 0;
                } else if (list_item->prev == 0) {
                    first = list_item->next;
                    first->prev = 0;
                } else if (list_item->next == 0) {
                    list_item->prev->next = 0;
                    last = list_item->prev;
                } else {
                    list_item->prev->next = list_item->next;
                    list_item->next->prev = list_item->prev;
                }
                list_cnt--;

                free(list_item->packet);
                free(list_item);
            }
            if(success == 0)
                list_item = list_item->next;
        }
        list_lock.unlock();
        if(success == 0){
            printf("Error Reliable Transfer:: Packet which does not exist acknowledged (maybe duplicate)\n");
        }
        
    } else {
        unsigned char* buf = ((unsigned char *)buffer) + sizeof(struct reliable_packet);
        size_t new_size = size - sizeof(struct reliable_packet);
        struct reliable_packet *cp_ack = (struct reliable_packet *)malloc(sizeof(struct reliable_packet));
        memcpy(cp_ack, &ack, sizeof(ack));
        cp_ack->id = header->id;
        if(header->broadcast == 1){
            cp_ack->broadcast = 1;
        }
        
        (*unrel)->send(cp_ack, sizeof (struct reliable_packet), 2, addr);
        free(cp_ack);

        if (header->broadcast == 1) {
            if (header->id != last_broadcast) {
                last_broadcast = header->id;
                (*unrel)->send(buffer, size, 2, (addr + 1) % 4);
                (*unrel)->send(buffer, size, 2, (addr + 2) % 4);
                (*unrel)->send(buffer, size, 2, (addr + 3) % 4);
                out->add(new_size, addr, buf);
            } else {
            }
        } else {
            out->add(new_size, addr, buf);
        }
    }
}

uint32_t ReliableTransfer::send(void *buffer, size_t size, uint8_t addr, uint8_t broadcast){
    
    if(buffer == 0 || size <= 0){
        printf("Error Reliable Transfer: buffer or size is wrong\n");
        return -1;
    }
    
    
    //add topology info to enable send
    if(list_cnt > 20){
        //printf("Error ReliableTransfer: list too long right now\n");
        return -1;
    }
    if(topo->isalive(addr) == 0){
        //printf("Error Reliable Transfer: destination is not alive\n");
        return -1;
    }
    
    
    size_t total_size = size + sizeof(struct reliable_packet);
    void *buf = malloc(total_size);
    void *cpy_buffer = malloc(total_size);
    if (buf < 0) {
        printf("Error Reliable Transfer:: allocating buffer failed\n");
        return -1;
    }
    
    memcpy(((unsigned char*)buf + sizeof (struct reliable_packet)), buffer, size);
    
    struct reliable_packet *header = (struct reliable_packet *)buf;

    header->ack = 0;
    header->filler = 0;
    header->id = seq++;
    header->broadcast = broadcast;
    
    memcpy((unsigned char*)cpy_buffer, (unsigned char*)buf, total_size);

    struct linked_header *packet = (struct linked_header *) malloc(sizeof(struct linked_header));

    packet->next = 0;
    packet->prev = 0;
    packet->addr = addr;
    packet->packet = (struct reliable_packet*)cpy_buffer;
    packet->size = total_size;
    packet->resent_time = 0;
    
    struct timespec current;
    clock_gettime(CLOCK_REALTIME, &current);
    packet->timeout.tv_sec = current.tv_sec + TIMEOUT;
    
    list_lock.lock();
    if(first == 0){
        first = packet;
        last = packet;
    }else{
        last->next = packet;
        packet->prev = last;
        last = packet;   
    }
    list_cnt++;
    list_lock.unlock();
 
    (*unrel)->send(buf, total_size, 2, addr);
    free(buf);
    
    return header->id;
}

//call this function every 250ms
int ReliableTransfer::check_timeouts(){
    if(first == 0){
        return 0;
    }
    struct timespec current;
    clock_gettime(CLOCK_REALTIME, &current);

    while (first != 0 && first->timeout.tv_sec <= current.tv_sec) {
        if (first->resent_time < 5) {
            printf("ReliableTransfer: retransmitting packet\n");
            struct reliable_packet* send_temp = (struct reliable_packet*)malloc(first->size);
            list_lock.lock();
            memcpy(send_temp, first->packet, first->size);
            (*unrel)->send(send_temp, first->size, 2, first->addr);
            first->timeout.tv_sec = current.tv_sec + TIMEOUT;
            //first->timeout.tv_nsec = current.tv_nsec;
            first->resent_time++;
            struct linked_header *temp = first;
            first = first->next;
            if (last != 0) {
                last->next = temp;
                last = temp;
            } else if (first == 0) {
                first = temp;
                last = temp;
            }
            list_lock.unlock();
        }else{
            printf("ERROR::Reliable Transfer::Packet could not be transmitted\n");
            struct linked_header *temp = first;
            first = first->next;
            free(temp->packet);
            free(temp);
            if(first != 0)
                first->prev = 0;
            list_cnt--;
            //call app to signal packet could not be transmitted
        }
    }
}