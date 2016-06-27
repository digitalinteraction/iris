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
#include <thread>
#include "crc/crc.h"

UnreliableTransfer::UnreliableTransfer(ReliableTransfer *rel, Topology *topo, DebugTransfer *debug) {
    send = new Packetbuffer();
    recv = new Packetbuffer();
    sercon = new SerialCon(send, recv);
    F_CRC_InicializaTabla();
    
    this->rel = rel;
    this->topo = topo;
    this->debug = debug;
    
    sercon->processing = 1;
    
    
    //TODO::set up fd if something is received, possibly with select and call recv() function
    
    
    std::thread serial_comm(&SerialCon, sercon);
    
}

UnreliableTransfer::UnreliableTransfer(const UnreliableTransfer& orig) {
}

UnreliableTransfer::~UnreliableTransfer() {
}

int UnreliableTransfer::send(void* buffer, size_t size, uint8_t port, uint8_t addr) {
    //struct unreliable_packet pack;
    size_t total_size = size + sizeof(struct unreliable_packet);
    if (total_size > SIZE_LIMIT) {
        printf("Error Unreliable Transfer:: size of packet is too large\n");
        return -1;
    }
    void * total_buf = malloc(total_size);
    if (total_buf < 0) {
        printf("Error Unreliable Transfer:: allocating buffer failed\n");
        return -1;
    }

    struct unreliable_packet *header = (struct unreliable_packet *) total_buf;
    header->buffer = (total_buf + sizeof (struct unreliable_packet));

    memcpy(header->buffer, buffer, size);
    header->addr = addr;
    header->port = port;
    header->size = size;
    header->crc = 0;
    header->crc = F_CRC_CalculaCheckSum(total_buf, total_size);
    
    send->add(total_size, addr, total_buf);
    free(total_buf);
}

int UnreliableTransfer::recv(){
    //TODO:: maybe check for data availability
    struct packet *pack;
    if(recv->get(&pack) == 0){
        struct unreliable_packet *header = (struct unreliable_packet *)pack->buffer;
        crc real_crc = header->crc;
        header->crc = 0;
        crc comp_crc = F_CRC_CalculaCheckSum(pack->buffer, pack->size);
        if(comp_crc != real_crc){
            free(pack->buffer);
            free(pack);
            printf("Error Unreliable Transfer:: CRC did not match\n");
            return -1;
        }
        
        void *buffer = malloc(header->size);
        memcpy(header->buffer, buffer, header->size);
        
        uint8_t port = header->port;
        size_t size = header->size;
        uint8_t addr = pack->addr;
        free(pack->buffer);
        free(pack);

        switch(port){
            case 0:
                debug->recv(buffer, size);
                break;
            case 1:
                topo->recv(buffer, size);
                break;
            case 2:
                rel->recv(buffer, size);
                break;
            default:
                free(buffer);
        }
    }
    
}