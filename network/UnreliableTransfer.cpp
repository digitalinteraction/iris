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
#ifdef DEBUG
    printf("UnreliableTransfer::%p %p\n", send_buf, recv_buf);
#endif
    this->send_buf = new Packetbuffer();
    this->recv_buf = new Packetbuffer();
#ifdef DEBUG
    printf("UnreliableTransfer::%p %p\n", send_buf, recv_buf);
#endif
    sercon = new SerialCon(send_buf, recv_buf);
    F_CRC_InicializaTabla();
    
    this->rel = rel;
    this->topo = topo;
    this->debug = debug;
    
    sercon->processing = 1;
    
    
    //TODO::set up fd if something is received, possibly with select and call recv() function
    
    
    serial_comm = new std::thread(&SerialCon::slip_run, sercon);
}

UnreliableTransfer::~UnreliableTransfer() {
}

int UnreliableTransfer::send(void* buffer, size_t size, uint8_t port, uint8_t addr) {
    //struct unreliable_packet pack;
#ifdef DEBUG
    printf("Unreliable Transfer:: function send got called with following arguments: %p %ld %d %d\n", buffer, size, port, addr); fflush(stdout);
    printf("Unreliable Transfer:: send_buf %p\n",send_buf); fflush(stdout);
#endif
    size_t total_size = size + sizeof (struct unreliable_packet);
    if (total_size > SIZE_LIMIT) {
        printf("Error Unreliable Transfer:: size of packet is too large\n");
        return -1;
    }
#ifdef DEBUG
    printf("Unreliable Transfer:: allocating buffer %ld\n", total_size);
    printf("Unreliable Transfer:: send_buf %p\n", send_buf);
#endif
    void * total_buf = malloc(total_size);
#ifdef DEBUG
    printf("Unreliable Transfer:: allocating buffer %p\n", total_buf);
#endif

    if (total_buf < 0) {
        printf("Error Unreliable Transfer:: allocating buffer failed\n");
        return -1;
    }

    struct unreliable_packet *header = (struct unreliable_packet *) total_buf;
    header->buffer = ((unsigned char*) total_buf + sizeof (struct unreliable_packet));
#ifdef DEBUG
    printf("Unreliable Transfer:: header start %p, buffer start %p, total length %ld\n", header, header->buffer, total_size);
#endif
    memcpy(header->buffer, buffer, size);
    header->addr = addr;
    header->port = port;
    header->size = size;
    header->crc_val = 0;
    header->crc_val = F_CRC_CalculaCheckSum((uint8_t*) total_buf, total_size);
    //header->crc_val.process_bytes(total_buf, total_size);
#ifdef DEBUG
    printf("Unreliable Transfer:: sending buffer %p size %ld addr %d\n", total_buf, total_size, addr); fflush(stdout);
    printf("Unreliable Transfer:: address of pointer of Packetbuffer %p\n", &send_buf); fflush(stdout);
    printf("Unreliable Transfer:: pointer of Packetbuffer %p\n", send_buf); fflush(stdout);
#endif
    send_buf->add(total_size, addr, total_buf);
#ifdef DEBUG
    printf("Unreliable Transfer:: send finished, now freeing buffer\n");
#endif
    free(total_buf);
}

int UnreliableTransfer::recv(){
    //TODO:: maybe check for data availability
    struct packet *pack;
    if(recv_buf->get(&pack) == 0){
        struct unreliable_packet *header = (struct unreliable_packet *)pack->buffer;
        crc real_crc = header->crc_val;
        header->crc_val = 0;
        crc comp_crc = F_CRC_CalculaCheckSum((uint8_t*)pack->buffer, pack->size);
        //boost::crc_32_type comp_crc.process_bytes(pack->buffer, pack->size);
        if(comp_crc != real_crc){
            free(pack->buffer);
            free(pack);
            printf("Error Unreliable Transfer:: CRC did not match %x %x\n", comp_crc, real_crc);
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
                debug->recv(buffer, size, addr);
                break;
            case 1:
                topo->recv(buffer, size, addr);
                break;
            case 2:
                rel->recv(buffer, size, addr);
                break;
            default:
                free(buffer);
        }
    }
    
}