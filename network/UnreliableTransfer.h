/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   UnreliableTransfer.h
 * Author: tobias
 *
 * Created on June 27, 2016, 2:53 PM
 */

#ifndef UNRELIABLETRANSFER_H
#define UNRELIABLETRANSFER_H
#include <stdint.h>
#include "Packetbuffer.h"
#include <thread>

#include "crc/crc.h"
//#include <boost/crc.hpp>

class ReliableTransfer;
class Topology;
class DebugTransfer;
class SerialCon;

struct unreliable_packet{
    size_t size;
    uint8_t port;
    uint8_t addr;
    //boost::crc_32_type crc_val;
    crc crc_val;
    //void *buffer;
};

class UnreliableTransfer {
public:
    UnreliableTransfer(ReliableTransfer *rel, Topology *topo, DebugTransfer *debug, uint8_t deb_switch);
    virtual ~UnreliableTransfer();
    int send(void* buffer, size_t size, uint8_t port, uint8_t addr);
    int recv();
    std::thread *serial_comm;
    int recv_fd;
private:
    Packetbuffer *send_buf;
    Packetbuffer *recv_buf;
    
    ReliableTransfer *rel;
    Topology *topo;
    DebugTransfer *debug;
    SerialCon *sercon;

    
};

#endif /* UNRELIABLETRANSFER_H */

