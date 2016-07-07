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
    first = 0;
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
    
    struct packet_map *map = (struct packet_map *)buffer;
    add_device_entry(map);
    build_mapping();
    
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
        

        map.mac = mac;
        map.up = mapping[0];
        map.down = mapping[3];
        map.left = mapping[1];
        map.right = mapping[2];
        
        //print_mapping(map);
        
        (*unrel)->send((void*) &map, sizeof (struct packet_map), 1, addr);
#endif
}

void Topology::print_mapping(struct packet_map* map){
#ifdef CLIENT_SIDE
    printf("%llx\n", map->up);
    printf("%llx::", map->left);
    printf("%llx::", map->mac);
    printf("%llx\n", map->right);
    printf("%llx\n", map->down);
#else
    printf("%lx\n", map->up);
    printf("%lx::", map->left);
    printf("%lx::", map->mac);
    printf("%lx\n", map->right);
    printf("%lx\n", map->down);
#endif
}

void Topology::add_device_entry(struct packet_map* map){
    struct device_info *item = device_first;
    uint8_t success = 0;
    while(item != 0){
        if(item->mac == map->mac){
            success = 1;
        }
        if(success == 0){
            item = item->next;
        }
    }
    
    struct timespec current;
    clock_gettime(CLOCK_REALTIME, &current);
    unsigned long currenttime = current.tv_sec*1000 + current.tv_nsec/1000000;
    
    if(success == 1){
        memcpy(item->map, map, sizeof(struct packet_map));
        item->timeout = currenttime + 5000;
    }else{
        struct device_info* dev = (struct device_info*) malloc(sizeof(struct device_info));
        memset(dev, 0, sizeof(struct device_info));
        dev->map = (struct packet_map *) malloc(sizeof(struct packet_map));
        memcpy(dev->map, map, sizeof(struct packet_map));
        dev->timeout = currenttime + 5000;
        dev->mac = map->mac;
        
        if(device_first == 0){
            device_first = dev;
            device_last = dev;
        }else{
            device_last->next = dev;
            device_last = dev;
        }
        
    }
    
}

struct device_info* Topology::get_device_entry(uint64_t mac){
    struct timespec current;
    clock_gettime(CLOCK_REALTIME, &current);
    unsigned long currenttime = current.tv_sec * 1000 + current.tv_nsec / 1000000;
    
    if(mac == 0) {
        currenttime += 5000;
        struct device_info *item = device_first;
        struct device_info *ret;
        while (item != 0) {
            if (item->timeout < currenttime) {
                currenttime = item->timeout;
                ret = item;
            }
            item = item->next;
        }
        return ret;
    } else {
        
        
        struct device_info *item = device_first;
        while (item != 0) {
            if (item->mac == mac && item->timeout > currenttime) {
                return item;
            }
            item = item->next;
        }
    }
    return 0;
}



void Topology::add_unexplored_entry(uint64_t mac, unsigned long timeout){
    struct topo_unexplored* item = (struct topo_unexplored *) malloc(sizeof(struct topo_unexplored));
    memset(item, 0, sizeof(struct topo_unexplored));
    item->mac = mac;
    item->timeout = timeout;
    
    if(unexp_first == 0){
        unexp_first = item;
        unexp_last = item;
    }else{
        unexp_last->next = item;
        unexp_last = item;
    }
}

struct topo_unexplored* Topology::get_unexplored_entry(){
    struct topo_unexplored* item = unexp_first;
    struct topo_unexplored* ret = 0;
    struct timespec current;
    clock_gettime(CLOCK_REALTIME, &current);
    unsigned long currenttime = current.tv_sec * 1000 + current.tv_nsec / 1000000;
    currenttime += 5000;

    while (item != 0) {
        if (item->timeout < currenttime) {
            currenttime = item->timeout;
            ret = item;
        }
        item = item->next;
    }
    return ret;
}

struct temp_topo* Topology::search_topo(struct temp_topo*cur, uint64_t mac, uint8_t search){
    if(cur == 0){
        return 0;
    }
    struct temp_topo* ret = 0;
    if (cur->search == (search-1)) {
        cur->search++;
        struct temp_topo* temp = 0;
        ret = search_topo(cur->down, mac, search);
        ret = search_topo(cur->up, mac, search);
        ret = search_topo(cur->left, mac, search);
        ret = search_topo(cur->right, mac, search);

        if (cur->mac == mac) {
            ret = cur;
        }
    }
    return ret;
}

void Topology::build_mapping(){
    printf("starting building map\n");
    struct device_info* root = get_device_entry(0);
    add_unexplored_entry(root->mac, root->timeout);

    struct temp_topo* first = 0;
    first = (struct temp_topo *) malloc(sizeof (struct temp_topo));
    memset(first, 0, sizeof (struct temp_topo));
    first->mac = root->mac;
    struct temp_topo* current = first;
    


    while (unexp_first != 0) {
        struct topo_unexplored* item = get_unexplored_entry();
        current = search_topo(first, item->mac, first->search);
        struct device_info* cur = get_device_entry(item->mac);
        
        printf("Current mac address %lx\n", current->mac);
        
        struct device_info* temp;
        temp = get_device_entry(cur->map->down);
        if(temp != 0){
            current->down = (struct temp_topo *) malloc(sizeof (struct temp_topo));
            memset(current->down, 0, sizeof (struct temp_topo));
            current->down->mac = temp->mac;
            current->down->up = current;
            add_unexplored_entry(temp->mac, temp->timeout);
        }
        temp = get_device_entry(cur->map->up);
        if(temp != 0){
            current->up = (struct temp_topo *) malloc(sizeof (struct temp_topo));
            memset(current->up, 0, sizeof (struct temp_topo));
            current->up->mac = temp->mac;
            current->up->down = current;
            add_unexplored_entry(temp->mac, temp->timeout);
        }
        temp = get_device_entry(cur->map->left);
        if(temp != 0){
            current->left = (struct temp_topo *) malloc(sizeof (struct temp_topo));
            memset(current->left, 0, sizeof (struct temp_topo));
            current->left->mac = temp->mac;
            current->left->right = current;
            add_unexplored_entry(temp->mac, temp->timeout);
        }
        temp = get_device_entry(cur->map->right);
        if(temp != 0){
            current->right = (struct temp_topo *) malloc(sizeof (struct temp_topo));
            memset(current->right, 0, sizeof (struct temp_topo));
            current->right->mac = temp->mac;
            current->right->left = current;
            add_unexplored_entry(temp->mac, temp->timeout);
        }            
    }
    
    
}



