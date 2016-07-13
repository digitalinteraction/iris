/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   NetworkControl.cpp
 * Author: tobias
 * 
 * Created on July 6, 2016, 3:34 PM
 */

#include "NetworkControl.h"

NetworkControl::NetworkControl() {
    
    image_out = new Packetbuffer(1);
    unrel_out = new Packetbuffer(2);
    unrel_in = new Packetbuffer(3);
    topo = new Topology(&unrel, unrel_out);
    rel = new ReliableTransfer(&unrel, image_out, topo);
    debug = new DebugTransfer(unrel_out, &unrel);
    unrel = new UnreliableTransfer(rel, topo, debug);
    
    image_in = new Packetbuffer(4);


    Timeout.tv_usec = 1000;
    Timeout.tv_sec = 0;
    fd_list[0] = unrel->recv_fd;
    fd_list[1] = image_in->signalfd;
    fd_list[2] = unrel_in->signalfd;
    
    for (int i = 0; i < 3; i++) {
        if (fd_list[i] > maxfd) {
            maxfd = fd_list[i];
        }
    }
    maxfd++;
    struct timespec current;
    clock_gettime(CLOCK_REALTIME, &current);
    unsigned long currenttime = current.tv_sec*1000 + current.tv_nsec/1000000;
    nextcheck = currenttime +20;
    toposend = currenttime +1000;
    buildtopo = currenttime +3000;
    nextprint = currenttime + 1000;
}

NetworkControl::NetworkControl(const NetworkControl& orig) {
}

NetworkControl::~NetworkControl() {
}

void NetworkControl::run(){
    struct timespec current;
   
    //unsigned long nextsend = currenttime + 500;
    //unsigned long nextprint = currenttime +1000;
    
    
    
        printf("nc running start\n");
        FD_ZERO(&readfs);
        clock_gettime(CLOCK_REALTIME, &current);
        unsigned long currenttime = current.tv_sec*1000 + current.tv_nsec/1000000;
        
        for (int i = 0; i < 3; i++) {
            FD_SET(fd_list[i], &readfs);
        }
        Timeout.tv_usec = 1000;
        Timeout.tv_sec = 0;
        int res = select(maxfd, &readfs, NULL, NULL, &Timeout);

        if (res > 0) {
            if (FD_ISSET(fd_list[0], &readfs)) {
                uint64_t val = 0;
                read(fd_list[0], &val, sizeof (uint64_t));
                unrel->recv();
            }
            if (FD_ISSET(fd_list[1], &readfs)) {
                uint64_t val = 0;
                read(fd_list[1], &val, sizeof (uint64_t));
                struct packet* pack;
                //printf("recieved something fro image_in\n");
                while(image_in->get(&pack) == 0){
                    //printf("sending out image packets\n");
                    if(rel->send(pack->buffer, pack->size, pack->addr, pack->broadcast) != 0){
                        printf("Error NetworkControl: sending reliable packet not working\n");
                    }
                    free(pack->buffer);
                    free(pack);
                }
            }
            if (FD_ISSET(fd_list[2], &readfs)) {
                uint64_t val = 0;
                read(fd_list[2], &val, sizeof (uint64_t));
                struct packet* pack;
                //printf("recieved something fro image_in\n");
                while(unrel_in->get(&pack) == 0){
                    //printf("sending out image packets\n");
                    debug->send(pack);
                    //if(unrel->send(pack->buffer, pack->size, 0, pack->addr) != 0){
                     //   printf("Error NetworkControl: sending unreliable packet not working\n");
                    //}
                    free(pack->buffer);
                    free(pack);
                }
            }
        }
        printf("A\n");
        if(currenttime > nextcheck){
            rel->check_timeouts();
            nextcheck = currenttime + 20;
        }
        printf("B\n");
        if(currenttime > toposend){
            toposend = currenttime + 100;
            topo->send();
        }
        printf("C\n");
#ifndef CLIENT_SIDE
        if(currenttime > buildtopo){
            //printf("build mapping\n");
            topo->build_mapping();
            buildtopo = currenttime + 2456;

        }
#endif
       printf("D\n");
#ifdef CLIENT_SIDE
        if(currenttime > nextprint){
            struct mallinfo mi = mallinfo();
            printf("Hosts alive: %d %d %d %d\n", topo->isalive(0), topo->isalive(1), topo->isalive(2),topo->isalive(3));
            topo->sendlist();
            nextprint = currenttime + 1000;
        }

        struct packet* pack;
        while (image_in->get(&pack) == 0) {
            //printf("sending the conventional way, size: %ld addr: %d\n", pack->size, pack->addr);
            if (unrel->send(pack->buffer, pack->size, 0, pack->addr) != 0) {
                printf("Error NetworkControl: sending reliable packet not working\n");
            }
            free(pack->buffer);
            free(pack);
        }
        printf("E\n");
#endif
        //delete on attaching other class
        /*struct packet *pack = 0;
        while(image_out->get(&pack) == 0){
            //printf("n free %p %p\n", pack, pack->buffer);
            free(pack->buffer);
                free(pack);
                printf("got mapping from topology\n");
            }*/
        //end delete
        printf("nc running end\n");
    

    //unrel->serial_comm->join();
}


void NetworkControl::run_inf(){
    while(true){
        run();
    }
}