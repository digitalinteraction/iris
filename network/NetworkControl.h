/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   NetworkControl.h
 * Author: tobias
 *
 * Created on July 6, 2016, 3:34 PM
 */

#ifndef NETWORKCONTROL_H
#define NETWORKCONTROL_H
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <malloc.h>

#include "ReliableTransfer.h"
#include "DebugTransfer.h"
#include "Topology.h"

#include "UnreliableTransfer.h"



class NetworkControl {
public:
    NetworkControl();
    NetworkControl(const NetworkControl& orig);
    virtual ~NetworkControl();
    void run();
    Packetbuffer *image_out;
    Packetbuffer *image_in;
    UnreliableTransfer *unrel;
    Topology *topo;
    ReliableTransfer *rel;
    DebugTransfer *debug;
private:
    
    int fd_list[2];
    struct timeval Timeout;
    fd_set readfs;
    int maxfd = 0;

};

#endif /* NETWORKCONTROL_H */

