/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Topology.cpp
 * Author: tobias
 * 
 * Created on June 27, 2016, 4:28 PM
 */

#include "Topology.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

Topology::Topology(UnreliableTransfer **unrel) {
    this->unrel = unrel;
    FILE * file = fopen("/sys/class/net/wlan0/address", "r");
    unsigned char a=0,b=0,c=0,d=0,e=0,f=0;
    fscanf(file, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &a, &b, &c, &d, &e, &f);
    fclose(file);
    mac = 0;
    mac = (((uint64_t)a)<<40)|(((uint64_t)b)<<32)|(((uint64_t)c)<<24)|(((uint64_t)d)<<16)|(((uint64_t)e)<<8)|(((uint64_t)f)<<0);
    printf("Topology:: got MAC %llx\n", mac);
    alive[0].tv_sec = 0;
    alive[1].tv_sec = 0;
    alive[2].tv_sec = 0;
    alive[3].tv_sec = 0;
    mapping[0] = 0;
    mapping[1] = 0;
    mapping[2] = 0;
    mapping[3] = 0;
    
    topo_buf.addr = 0;
    topo_buf.mac = mac;
#ifndef CLIENT_SIDE
    in_map = new Packetbuffer();
#endif
    
}


Topology::~Topology() {
}

int Topology::send(){
#ifdef CLIENT_SIDE
        (*unrel)->send((void*) &topo_buf, sizeof (struct topo_buffer), 1, 0);

        (*unrel)->send((void*) &topo_buf, sizeof (struct topo_buffer), 1, 1);

        (*unrel)->send((void*) &topo_buf, sizeof (struct topo_buffer), 1, 2);

        (*unrel)->send((void*) &topo_buf, sizeof (struct topo_buffer), 1, 3);
#endif
}

int Topology::recv(void* buffer, size_t size, uint32_t addr) {
#ifdef CLIENT_SIDE
    struct topo_buffer *buf = (struct topo_buffer *) buffer;
    if (addr < 4) {
        //printf("adding mac %llx to list\n", buf->mac);
        if (mapping[addr] == 0 || mapping[addr] == buf->mac) {
            mapping[addr] = buf->mac;
            struct timespec current;
            clock_gettime(CLOCK_REALTIME, &current);
            alive[addr].tv_sec = current.tv_sec;
        } else {
            printf("Error Topology:: Conflicting MACs for the same spot in mapping\n");
        }
    }
#else
    //address trasnlation
    struct in_addr in;
    in.s_addr = addr;
    char *hostaddrp = inet_ntoa(in);
    printf("Topology: got packet from %s\n", hostaddrp);
    //address trasnlation end
    struct packet_map *map = (struct packet_map *)malloc(sizeof(struct packet_map));

    memcpy(map, buffer, sizeof(struct packet_map));
        print_mapping((struct packet_map*)buffer);

    in_map->add(sizeof(struct packet_map), addr, map);
    
#endif
    
}

int Topology::isalive(uint32_t addr){
    struct timespec current;
    clock_gettime(CLOCK_REALTIME, &current);
    if(current.tv_sec <= (alive[addr].tv_sec+2)){
        return 1;
    }else{
        return 0;
    }
}

int Topology::sendlist() {
#ifdef CLIENT_SIDE
        uint32_t addr;
        if (inet_aton("172.16.0.1", (in_addr *) & addr) == 0) {
            printf("inet_aton() failed\n");
        }
        //insert case of timeout/invalid entry
        struct packet_map *map = (struct packet_map *)malloc(sizeof(struct packet_map));
        memset(map, 0, sizeof(struct packet_map));
        printf("1: %llx\n", mac);
        printf("2: %llx\n", mapping[0]);
        printf("3: %llx\n", mapping[1]);
        printf("4: %llx\n", mapping[2]);
        printf("5: %llx\n", mapping[3]);

        map->mac = mac;
        map->up = mapping[0];
        map->down = mapping[3];
        map->left = mapping[1];
        map->right = mapping[2];
        
        print_mapping(map);
        
        (*unrel)->send((void*) &map, sizeof (struct packet_map), 1, addr);
        free(map);
#endif
}

void Topology::print_mapping(struct packet_map* map){
    printf("%llx\n", map->up);
    printf("%llx::", map->left);
    printf("%llx::", map->mac);
    printf("%llx\n", map->right);
    printf("%llx\n", map->down);
}