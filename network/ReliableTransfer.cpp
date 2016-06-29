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


#define TIMEOUT   5;

ReliableTransfer::ReliableTransfer(UnreliableTransfer **unrel, Packetbuffer *out) {
    this->unrel = unrel;
    first = 0;
    last = 0;
    seq = 5;
    ack.ack = 1;
    ack.broadcast = 0;
    ack.filler = 0;
    ack.id = 0;
    last_broadcast = 0;
    list_lock.unlock();
    this->out = out;
}

ReliableTransfer::ReliableTransfer(const ReliableTransfer& orig) {
}

ReliableTransfer::~ReliableTransfer() {
}

int ReliableTransfer::recv(void* buffer, size_t size, uint8_t addr) {

    //printf("Reliable Transfer:: got packet size %ld addr %d\n", size, addr);
    struct reliable_packet *header = (struct reliable_packet*) buffer;
    //printf("Reliable Transfer:: header info %d %d %d\n", header->ack, header->broadcast, header->id);
    //printf("Content of Ack: ");
    //for (int i = 0; i < size; i++) {
    //   printf(" %x ", *(((char *) header) + i));
    //}
    //printf("\n");

    if (header->ack == 1) {
        //printf("Reliable Transfer:: ack received with id %d\n", header->id);       
        list_lock.lock();
        struct linked_header *list_item = first;
        uint8_t success = 0;
        
        if(header->broadcast == 1 && header->id == last_broadcast){
            success = 1;
        }
        while (list_item != 0 && success != 1) {
            if (list_item->packet->id == header->id) {
                success = 1;
                //TODO reset timeout to next interval
                if (list_item->prev == 0 && list_item->next == 0) {
                    first = 0;
                } else if (list_item->prev == 0) {
                    first = list_item->next;
                    first->prev = 0;
                } else if (list_item->next == 0) {
                    list_item->prev->next = 0;
                } else {
                    list_item->prev->next = list_item->next;
                    list_item->next->prev = list_item->prev;
                }
                //free(list_item->packet->buffer);
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
        free(buffer);
        
    } else {
        unsigned char* buf = ((unsigned char *)buffer) + sizeof(struct reliable_packet);
        size_t new_size = size - sizeof(struct reliable_packet);
        //printf("Reliable Transfer:: send packet to out with size %ld\n", new_size);
        //printf("Reliable Transfer::Content %s\n", buf);
        //is this thread safe?
        struct reliable_packet *cp_ack = (struct reliable_packet *)malloc(sizeof(struct reliable_packet));
        memcpy(cp_ack, &ack, sizeof(ack));
        cp_ack->id = header->id;
        //printf("size of ack: %ld\n", sizeof(struct reliable_packet));
        
        (*unrel)->send(cp_ack, sizeof (struct reliable_packet), 2, addr);

        if (header->broadcast == 1) {
            if (header->id != last_broadcast) {
                last_broadcast = header->id;
                (*unrel)->send(buffer, size, 2, (addr + 1) % 4);
                (*unrel)->send(buffer, size, 2, (addr + 2) % 4);
                (*unrel)->send(buffer, size, 2, (addr + 3) % 4);
                out->add(new_size, addr, buf);
            } else {
                free(buffer);
            }
        } else {
            out->add(new_size, addr, buf);
        }
    }
}

uint32_t ReliableTransfer::send(void *buffer, size_t size, uint8_t addr, uint8_t broadcast){
    //printf("Reliable Transfer:: sending packet size %ld\n", size);
    size_t total_size = size + sizeof(struct reliable_packet);
    void *buf = malloc(total_size);
    void *cpy_buffer = malloc(total_size);
    if (buf < 0) {
        printf("Error Reliable Transfer:: allocating buffer failed\n");
        return -1;
    }
    struct reliable_packet *header = (struct reliable_packet *)buf;
    //header->buffer = ((unsigned char*)buf + sizeof (struct reliable_packet));
    memcpy(((unsigned char*)buf + sizeof (struct reliable_packet)), buffer, size);
    header->ack = 0;
    header->filler = 0;
    header->id = seq++;
    header->broadcast = broadcast;
    
    memcpy((unsigned char*)cpy_buffer, (unsigned char*)buf, total_size);

    struct linked_header *packet = (struct linked_header *) malloc(sizeof(struct linked_header));
    packet->next = 0;
    packet->addr = addr;
    packet->packet = (struct reliable_packet*)cpy_buffer;
    packet->size = total_size;
    packet->resent_time = 0;
    clock_gettime(CLOCK_REALTIME, &(packet->timeout));
    packet->timeout.tv_sec = packet->timeout.tv_sec + TIMEOUT;
    
    list_lock.lock();
    if(first == 0){
        first = packet;
        last = packet;
    }else{
        last->next = packet;
        packet->prev = last;
    }
    list_lock.unlock();
    //printf("Reliable Transfer::sending packet size2 %ld to addr %d with id %d\n", total_size, addr, header->id);
    //printf("COntent: ");
    //for(int i = 0; i < total_size; i++){
    //    printf(" %x ", *((char *)buf+i));
    //}
    //printf("\n");
    (*unrel)->send(buf, total_size, 2, addr);
    
    return header->id;
}

//call this function every 250ms
int ReliableTransfer::check_timeouts(){
    if(first == 0){
        return 0;
    }
    struct timespec current;
    clock_gettime(CLOCK_REALTIME, &current);

    while (first != 0 && first->timeout.tv_sec < current.tv_sec) {
        if (first->resent_time < 5) {
            (*unrel)->send(first->packet, first->size, 2, first->addr);
            list_lock.lock();
            first->timeout.tv_sec = current.tv_sec + TIMEOUT;
            first->timeout.tv_nsec = current.tv_nsec;
            first->resent_time++;
            struct linked_header *temp = first;
            first = first->next;
            if (last != 0) {
                last->next = temp;
            } else if (first == 0) {
                first = temp;
                last = temp;
            }
            list_lock.unlock();
        }else{
            struct linked_header *temp = first;
            first = first->next;
            free(temp);
            if(first != 0)
                first->prev = 0;
            
            //call app to signal packet could not be transmitted
        }
    }
}