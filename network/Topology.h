/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Topology.h
 * Author: tobias
 *
 * Created on June 27, 2016, 4:28 PM
 */

#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include "UnreliableTransfer.h"


//class UnreliableTransfer;
struct topo_buffer{
    uint64_t mac;
    uint8_t addr;
};

struct packet_map{
    uint64_t mac;
    uint64_t right;
    uint64_t left;
    uint64_t up;
    uint64_t down;
};

//temp topo
struct temp_topo{
    uint8_t search;
    uint64_t mac;
    struct temp_topo* left;
    struct temp_topo* up;
    struct temp_topo* right;
    struct temp_topo* down;
    int8_t x;
    int8_t y;
};

struct device_info{
    //time
    struct packet_map* map;
    uint64_t mac;
    unsigned long timeout;
    uint8_t reachable;
    struct device_info* next;
};

struct topo_unexplored{
    uint64_t mac;
    unsigned long timeout;
    struct topo_unexplored* next;
};



class Topology {
public:
    Topology(UnreliableTransfer **unrel, Packetbuffer *out_map);
    virtual ~Topology();
    int recv(void *buffer, size_t size, uint32_t addr);
    int send();
    int isalive(uint32_t addr);
    int sendlist();
    void build_mapping();
    uint64_t mac;


private:
    UnreliableTransfer **unrel;
    struct timespec alive[4];
    uint64_t mapping[4];
    struct topo_buffer topo_buf;
    void print_mapping(struct packet_map* map);
    void add_device_entry(struct packet_map* map);
    struct device_info* get_device_entry(uint64_t mac);
    void add_unexplored_entry(uint64_t mac, unsigned long timeout);
    struct topo_unexplored* get_unexplored_entry();
    struct temp_topo* search_topo(struct temp_topo*cur, uint64_t mac, uint8_t search);
    void calc_topo(struct temp_topo* cur, uint8_t search, int8_t x, int8_t y, uint8_t mode);
    void set_devices_reach();
    struct packet_map map;
    Packetbuffer *out_map;
    
    struct device_info* device_first;
    struct device_info* device_last;
    struct topo_unexplored* unexp_first;
    struct topo_unexplored* unexp_last;

    uint64_t **map_array;
    unsigned char * start;
    
    int8_t min_x;
    int8_t max_x;
    int8_t min_y;
    int8_t max_y;

};

#endif /* TOPOLOGY_H */

