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


#define TIMEOUT   1;

ReliableTransfer::ReliableTransfer(UnreliableTransfer **unrel, Packetbuffer *out, Topology *topo) {
    this->unrel = unrel;
    first = 0;
    last = 0;
    seq = 5;
    ack.ack = 1;
    ack.broadcast = 0;
    ack.filler = 0;
    ack.id = 0;
    list_cnt = 0;
    last_broadcast = 0;
    this->topo = topo;
    list_lock.unlock();
    this->out = out;
}

ReliableTransfer::ReliableTransfer(const ReliableTransfer& orig) {
}

ReliableTransfer::~ReliableTransfer() {
}

int ReliableTransfer::recv(void* buffer, size_t size, uint32_t addr) {
    //printf("recv ");
    if(buffer == 0 || size <= 0){
        printf("Error ReliableTransfer: buffer or size is wrong\n");
        return -1;
    }
    
    
    struct reliable_packet *header = (struct reliable_packet*) buffer;

    if (header->ack == 1) {
        //printf(" ack ");
        list_lock.lock();
        struct linked_header *list_item = first;
        uint8_t success = 0;
        
        if(header->broadcast == 1 && header->id == last_broadcast){
            printf(" bc "); fflush(stdout);
            success = 1;
        }
        //printf(" while "); fflush(stdout);
        while (list_item != 0 && success != 1) {
            //printf(" li %p ", list_item); fflush(stdout);
            //printf(" lip %p ", list_item->packet); fflush(stdout);
            //printf(" lippn %p %p ", list_item->prev, list_item->next); fflush(stdout);
            
            if(list_item->packet != 0){
            if (list_item->packet->id == header->id) {
                success = 1;
                //printf("List prev: %p next %p\n", list_item->prev, list_item->next);
                if (list_item->prev == 0 && list_item->next == 0) {
                    //printf(" 0 "); fflush(stdout);
                    first = 0;
                    last = 0;
                } else if (list_item->prev == 0) {
                    //printf(" 1 "); fflush(stdout);
                    first = list_item->next;
                    first->prev = 0;
                } else if (list_item->next == 0) {
                    //printf(" 2 "); fflush(stdout);
                    list_item->prev->next = 0;
                    last = list_item->prev;
                } else {
                    //printf(" 3 "); fflush(stdout);
                    list_item->prev->next = list_item->next;
                    list_item->next->prev = list_item->prev;
                }
                list_cnt--;
                //printf("List First: %p Last %p\n", first, last);

                free(list_item->packet);
                free(list_item);
            }
            }
            if(success == 0){
                list_item = list_item->next;
            }
        }
        list_lock.unlock();
        if(success == 0){
            printf("Error Reliable Transfer:: Packet which does not exist acknowledged %d (maybe duplicate)\n", header->id);
        }
        
    } else {
        //printf(" real ");fflush(stdout);
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
            printf("got broadcast packet %d %d\n", header->id, last_broadcast);
            if (header->id != last_broadcast) {
                printf("resending broadcast packet\n");
                last_broadcast = header->id;
                (*unrel)->send(buffer, size, 2, (addr + 1) % 4);
                (*unrel)->send(buffer, size, 2, (addr + 2) % 4);
                (*unrel)->send(buffer, size, 2, (addr + 3) % 4);
                out->add(new_size, addr, buf);
            }
        } else {
            out->add(new_size, addr, buf);
        }
    }
    //printf("recv ends\n");
    return 0;
}

uint32_t ReliableTransfer::send(void *buffer, size_t size, uint32_t addr, uint8_t broadcast, uint32_t id){
    printf("%p %ld %d %d %d %d\n", buffer, size, addr, broadcast, list_cnt, topo->isalive(addr));
    if(buffer == 0 || size <= 0){
        printf("Error Reliable Transfer: buffer or size is wrong\n");
        if(callback != 0){
                callback(id, size, REL_ERROR_BUF_SIZE);
        }
        return -1;
    }
    
    
    if(list_cnt > 20){
        //printf("Error ReliableTransfer: list too long right now\n");
        if(callback != 0){
                callback(id, size, REL_ERROR_LIST);
        }
        return -1;
    }
    if(topo->isalive(addr) == 0){
        //printf("Error Reliable Transfer: destination is not alive\n");
        if(callback != 0){
                callback(id, size, REL_ERROR_DEAD);
        }
        return -1;
    }
    
    
    size_t total_size = size + sizeof(struct reliable_packet);
    void *buf = malloc(total_size);
    void *cpy_buffer = malloc(total_size);
    if (buf <= 0 || cpy_buffer <= 0) {
        printf("Error Reliable Transfer:: allocating buffer failed\n");
        if(callback != 0){
                callback(id, size, REL_ERROR_BUF_ALLOC);
        }
        return -1;
    }
    
    memcpy(((unsigned char*)buf + sizeof (struct reliable_packet)), buffer, size);
    
    struct reliable_packet *header = (struct reliable_packet *)buf;

    header->ack = 0;
    header->filler = 0;
    header->id = seq++;
    header->broadcast = broadcast;
    header->id = id;
    
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
    //printf("end reliable trasnfer\n");
    return 0;
}

//call this function every 250ms
int ReliableTransfer::check_timeouts(){
    if(first == 0){
        return -1;
    }
    struct timespec current;
    clock_gettime(CLOCK_REALTIME, &current);
    list_lock.lock();
    while (first != 0 && first->timeout.tv_sec <= current.tv_sec) {
        if (first->resent_time < 5) {
            printf("ReliableTransfer: retransmitting packet\n");
            // struct reliable_packet* send_temp = (struct reliable_packet*)malloc(first->size);
            // memcpy(send_temp, first->packet, first->size);
            //(*unrel)->send(send_temp, first->size, 2, first->addr);
            if(first->packet == 0){
                printf("Retransmit: empty payload\n"); fflush(stdout);
            }
            (*unrel)->send(first->packet, first->size, 2, first->addr);

            first->timeout.tv_sec = current.tv_sec + TIMEOUT;
            //first->timeout.tv_nsec = current.tv_nsec;
            first->resent_time++;
            
            struct linked_header *temp = first;
            
            
            if (first->next != 0) {
                first = first->next;
                first->prev = 0;
                temp->next = 0;
                temp->prev = last;
                last->next = temp;
                last = temp;
            }            
        } else {
            printf("ERROR::Reliable Transfer::Packet could not be transmitted\n");
            struct linked_header *temp = first;
            if(callback != 0){
                callback(temp->id, temp->size, REL_ERROR_TIMEOUT);
            }
            first = first->next;
            free(temp->packet);
            free(temp);
            if (first != 0)
                first->prev = 0;
            list_cnt--;
            
            
            //call app to signal packet could not be transmitted
        }
    }
        
    list_lock.unlock();
    return 0;
}


void ReliableTransfer::setCallback(void(*callback)(uint32_t,size_t,uint8_t)){
    this->callback = callback;
}

