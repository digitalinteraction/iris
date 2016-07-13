/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   UnreliableTransfer.cpp
 * Author: tobias
 * 
 * Created on June 27, 2016, 2:53 PM
 */

#include "UnreliableTransfer.h"
#include "ReliableTransfer.h"
#include "Topology.h"
#include "DebugTransfer.h"
#include "SerialCon.h"
//#include "crc/crc.h"


UnreliableTransfer::UnreliableTransfer(ReliableTransfer *rel, Topology *topo, DebugTransfer *debug) {

    this->send_buf = new Packetbuffer(5);
    this->recv_buf = new Packetbuffer(6);
    this->recv_fd = recv_buf->signalfd;

    sercon = new SerialCon(send_buf, recv_buf);
    F_CRC_InicializaTabla();
    
    this->rel = rel;
    this->topo = topo;
    this->debug = debug;
    lock_recv.unlock();
    lock_send.unlock();
    
    sercon->processing = 1;    
    
    serial_comm = new std::thread(&SerialCon::slip_run, sercon);
}

UnreliableTransfer::~UnreliableTransfer() {
}

int UnreliableTransfer::send(void* buffer, size_t size, uint8_t port, uint32_t addr) {
    

    lock_send.lock();
    if(size <= 0 || buffer == 0){
        printf("Error Unreliable Transfer:: buffer or size is wrong\n");
        lock_send.unlock();
        return -1;
    }
    size_t total_size = size + sizeof (struct unreliable_packet);
    if (total_size > SIZE_LIMIT) {
        printf("Error Unreliable Transfer:: size of packet is too large\n");
        lock_send.unlock();
        return -1;
    }

    void * total_buf = malloc(total_size);

    if (total_buf < 0) {
        printf("Error Unreliable Transfer:: allocating buffer failed\n");
        lock_send.unlock();
        return -1;
    }

    struct unreliable_packet *header = (struct unreliable_packet *) total_buf;
 
    memcpy((((unsigned char*) total_buf) + sizeof (struct unreliable_packet)), buffer, size);
    header->addr = addr;
    header->port = port;
    header->size = size;
    header->crc_val = 0;
    header->crc_val = F_CRC_CalculaCheckSum((uint8_t*) total_buf, total_size);
  
    send_buf->add(total_size, addr, total_buf);
    
    free(total_buf);
    lock_send.unlock();
    return 0;
}

int UnreliableTransfer::recv() {
    //printf("recv packet\n");
    lock_recv.lock();
    struct packet *pack;
    while (recv_buf->get(&pack) == 0) {
        
        if(pack->buffer == 0 || pack->size <= sizeof(struct unreliable_packet)) {
            printf("Error Unreliable Transfer:: buffer or size is wrong: %ld %p\n", pack->size, pack->buffer);
            lock_recv.unlock();
            return -1;
        }
        

        struct unreliable_packet *header = (struct unreliable_packet *) pack->buffer;
        crc real_crc = header->crc_val;
        header->crc_val = 0;
        crc comp_crc = F_CRC_CalculaCheckSum((uint8_t*) pack->buffer, pack->size);

        if (comp_crc != real_crc) {
            
            /*printf("Data: %ld : ", sizeof(struct unreliable_packet));
            for(int i = 0; i < pack->size; i++){
                printf(" %x ", ((unsigned char *)pack->buffer)[i]);
            }
            printf("\n");*/
            printf("Error Unreliable Transfer:: %d CRC did not match %x %x\n", header->port, comp_crc, real_crc);
            free(pack->buffer);
            free(pack);
            lock_recv.unlock();
            return -1;
        }
        //void *buffer = malloc(header->size);
        void *buffer = malloc(pack->size - sizeof(struct unreliable_packet));
        memcpy(buffer, ((unsigned char*) pack->buffer + sizeof (struct unreliable_packet)), pack->size - sizeof(struct unreliable_packet));
        
        uint8_t port = header->port;
        size_t size = header->size;
        uint32_t addr = pack->addr;
         //       printf("got port %d\n", port);

        free(pack->buffer);
        free(pack);
        //printf("got port %d\n", port);
        switch(port){
            case 0:
                debug->recv(buffer, size, addr);
                break;
            case 1:
                topo->recv(buffer, size, addr);
                break;
            case 2:
                rel->recv(buffer, size, addr);
                break;
            default:
                printf("UnreliableTransfer:: port not known: %d\n", port);
        }
        free(buffer);
    }
    lock_recv.unlock();
    //printf("end recv\n");
    return 0;
}