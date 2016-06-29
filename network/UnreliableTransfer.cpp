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


UnreliableTransfer::UnreliableTransfer(ReliableTransfer *rel, Topology *topo, DebugTransfer *debug, uint8_t deb_switch) {
#ifdef DEBUG
    printf("UnreliableTransfer::%p %p\n", send_buf, recv_buf);
#endif
    this->send_buf = new Packetbuffer();
    this->recv_buf = new Packetbuffer();
    this->recv_fd = recv_buf->signalfd;
#ifdef DEBUG
    printf("UnreliableTransfer::%p %p\n", send_buf, recv_buf);
#endif
    sercon = new SerialCon(send_buf, recv_buf, deb_switch);
    F_CRC_InicializaTabla();
    
    this->rel = rel;
    this->topo = topo;
    this->debug = debug;
    
    sercon->processing = 1;    
    
    serial_comm = new std::thread(&SerialCon::slip_run, sercon);
}

UnreliableTransfer::~UnreliableTransfer() {
}

int UnreliableTransfer::send(void* buffer, size_t size, uint8_t port, uint8_t addr) {
    //struct unreliable_packet pack;
    //printf("Unreliable Transfer:: function send got called with following arguments: %p %ld %d %d\n", buffer, size, port, addr); fflush(stdout);

#ifdef DEBUG
    //printf("Unreliable Transfer:: function send got called with following arguments: %p %ld %d %d\n", buffer, size, port, addr); fflush(stdout);
   //printf("Unreliable Transfer:: send_buf %p\n",send_buf); fflush(stdout);
#endif
    size_t total_size = size + sizeof (struct unreliable_packet);
    if (total_size > SIZE_LIMIT) {
        printf("Error Unreliable Transfer:: size of packet is too large\n");
        return -1;
    }
#ifdef DEBUG
    //printf("Unreliable Transfer:: allocating buffer %ld\n", total_size);
    //printf("Unreliable Transfer:: send_buf %p\n", send_buf);
#endif
    void * total_buf = malloc(total_size);
    //printf("u s malloc %p\n", total_buf);
#ifdef DEBUG
    //printf("Unreliable Transfer:: allocating buffer %p\n", total_buf);
#endif

    if (total_buf < 0) {
        printf("Error Unreliable Transfer:: allocating buffer failed\n");
        return -1;
    }

    struct unreliable_packet *header = (struct unreliable_packet *) total_buf;
    //header->buffer = ((unsigned char*) total_buf + sizeof (struct unreliable_packet));
#ifdef DEBUG
    //printf("Unreliable Transfer:: header start %p, buffer start %p, total length %ld\n", header, header->buffer, total_size);
#endif
    memcpy(((unsigned char*) total_buf + sizeof (struct unreliable_packet)), buffer, size);
    header->addr = addr;
    header->port = port;
    header->size = size;
    free(buffer);
    header->crc_val = 0;
    header->crc_val = F_CRC_CalculaCheckSum((uint8_t*) total_buf, total_size);
    //header->crc_val.process_bytes(total_buf, total_size);
#ifdef DEBUG
    printf("Unreliable Transfer:: sending buffer %p size %ld addr %d\n", total_buf, total_size, addr); fflush(stdout);
    //printf("Unreliable Transfer:: address of pointer of Packetbuffer %p\n", &send_buf); fflush(stdout);
    //printf("Unreliable Transfer:: pointer of Packetbuffer %p\n", send_buf); fflush(stdout);
    printf("*****send******\n");
    for(int i = 0; i< total_size; i++){
        printf(" %x ", ((char*)total_buf)[i]);
    }
    printf("\n");
#endif
    
    send_buf->add(total_size, addr, total_buf);
#ifdef DEBUG
    //printf("Unreliable Transfer:: send finished, now freeing buffer\n");
#endif
    
    //printf("u s free %p\n", total_buf);
    free(total_buf);
    
}

int UnreliableTransfer::recv() {
    struct packet *pack;
    while (recv_buf->get(&pack) == 0) {
    //printf("Unreliable Transfer:: got package with size %ld and address %d\n", pack->size, pack->addr);

#ifdef DEBUG
        printf("Unreliable Transfer:: got package with size %ld\n", pack->size);
        printf("****recv*******\n");
        for (int i = 0; i < pack->size; i++) {
            printf(" %x ", ((char *)pack->buffer)[i]);
        }
        printf("\n");
#endif
        
        struct unreliable_packet *header = (struct unreliable_packet *) pack->buffer;
        crc real_crc = header->crc_val;
        header->crc_val = 0;
        crc comp_crc = F_CRC_CalculaCheckSum((uint8_t*) pack->buffer, pack->size);
        //boost::crc_32_type comp_crc.process_bytes(pack->buffer, pack->size);
        #ifdef DEBUG
        //printf("Unreliable Transfer:: Real CRC %d Calc CRC %d\n", real_crc, comp_crc);
#endif
        if (comp_crc != real_crc) {
            //printf("u r1 free %p\n", pack->buffer);
            //printf("u r2 free %p\n", pack);
            free(pack->buffer);
            free(pack);
            printf("Error Unreliable Transfer:: CRC did not match %x %x\n", comp_crc, real_crc);
            return -1;
        }
        //printf("UnreliableTransfer:: recv %d %d %d %ld", header->addr, header->crc_val, header->port, header->size);
        void *buffer = malloc(header->size);
        //printf("u r malloc %p\n", buffer);
        memcpy(buffer, ((unsigned char*) pack->buffer + sizeof (struct unreliable_packet)), header->size);
        
        uint8_t port = header->port;
        size_t size = header->size;
        uint8_t addr = pack->addr;
        //printf("u r3 free %p\n", pack->buffer);
        //printf("u r4 free %p\n", pack);
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
                //printf("u r5 free %p\n", buffer);
        }
    }
    
}