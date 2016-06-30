/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   serial_test.cpp
 * Author: tobias
 *
 * Created on June 23, 2016, 4:09 PM
 */

#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <malloc.h>

#include "../network/ReliableTransfer.h"
#include "../network/DebugTransfer.h"
#include "../network/Topology.h"

#include "../network/UnreliableTransfer.h"


using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
    UnreliableTransfer *unrel;
    Packetbuffer *image_out = new Packetbuffer();
    ReliableTransfer *rel = new ReliableTransfer(&unrel, image_out);
    Topology *topo = new Topology(&unrel);
    DebugTransfer *debug = new DebugTransfer();
    printf("serial_test:: got arguments %s %s\n", argv[0], argv[1]);
    if(!strcmp(argv[1], "-a")){
        printf("starting in mode 1\n");
        unrel = new UnreliableTransfer(rel, topo, debug, 1);
    }else{
        printf("starting in mode 0\n");
        unrel = new UnreliableTransfer(rel, topo, debug, 0);
    }
    
    Packetbuffer *out = new Packetbuffer();
    
    char sendstring[] = "Testing Reliable Transfer\n";
    size_t size = sizeof(sendstring);

    sleep(2);
    
    int fd_list[2];
    struct timeval Timeout;
    Timeout.tv_usec = 1000;
    Timeout.tv_sec = 0;
    fd_list[0] = unrel->recv_fd;
    fd_list[1] = out->signalfd;
    printf("serial_test:: Connected to recv fd %d and %d\n", fd_list[0], fd_list[1]);
    int res = 0;
    int maxfd = 0;
    fd_set readfs;

    for (int i = 0; i < 2; i++) {
        if (fd_list[i] > maxfd) {
            maxfd = fd_list[i];
        }
    }
    
    struct timespec current;
    clock_gettime(CLOCK_REALTIME, &current);
    unsigned long currenttime = current.tv_sec*1000 + current.tv_nsec/1000000;
    unsigned long nextsend = currenttime + 20;
    unsigned long nextprint = currenttime +1000;
    unsigned long nextcheck = currenttime +20;

    //next_send.tv_nsec = (current.tv_nsec+20000000)%1000000000;
    //next_send.tv_sec = current.tv_sec;
    int sendpk = 0;
    
    
    while (true) {
        //rel->check_timeouts();
        FD_ZERO(&readfs);
        clock_gettime(CLOCK_REALTIME, &current);
        currenttime = current.tv_sec*1000 + current.tv_nsec/1000000;
        
        for (int i = 0; i < 1; i++) {
            FD_SET(fd_list[i], &readfs);
        }
        Timeout.tv_usec = 1000;
        Timeout.tv_sec = 0;
        res = select(maxfd, &readfs, NULL, NULL, &Timeout);
        //printf("serial_test:: select res %d\n", res);

        if (res > 0) {
            //printf("serial_test:: fd_isset %d\n", FD_ISSET(fd_list[0], &readfs));
            if (FD_ISSET(fd_list[0], &readfs)) {
                uint64_t val = 0;
                read(fd_list[0], &val, sizeof (uint64_t));
                
                //printf("0val : %ld\n", val);
                //printf("call unrel recv\n");
                unrel->recv();
            }
            if (FD_ISSET(fd_list[1], &readfs)) {
                //printf("out received something\n");
                uint64_t val = 0;
                read(fd_list[1], &val, sizeof (uint64_t));
                //printf("1val : %ld\n", val);
            }
            //printf("end\n");

        }
        
        if(currenttime > nextcheck){
            rel->check_timeouts();
            nextcheck = currenttime + 2;
        }
        
        if(currenttime > nextsend){
            //topo->send();
            
            rel->send((void*)sendstring, size, 0, 0);
            nextsend = currenttime + 20;
            sendpk+=1;
            //next_send.tv_nsec = (current.tv_nsec+20000000)%1000 000 000;
            //next_send.tv_sec = current.tv_sec;
        }
        
        if(currenttime > nextprint){
            struct mallinfo mi = mallinfo();
            printf("serial_test:: memory: %d send packets: %d list_cnt: %d\n", mi.uordblks, sendpk, rel->list_cnt);
            sendpk = 0;
            nextprint = currenttime+1000;
        }
        
        
        
        struct packet *pack = 0;
        while(image_out->get(&pack) == 0){
            //printf("free pack buffer %p pack %p\n", pack->buffer, pack);
            free(pack->buffer);
            free(pack);
        }
        //sleep(2);

    }

    unrel->serial_comm->join();
    return 0;
}

