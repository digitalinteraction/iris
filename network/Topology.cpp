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

Topology::Topology(UnreliableTransfer **unrel, Packetbuffer* out_map) {
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
    //first = 0;
    this->out_map = out_map;
    
    device_first = 0;
    device_last = 0;
    unexp_first = 0;
    unexp_last = 0;
    
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
        return 0;
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
    printf("ATopology: got packet from %s with size %ld\n", hostaddrp, size);
    
    struct packet_map *map = (struct packet_map *)buffer;
    add_device_entry(map);
#endif
    return 0;
    
}

int Topology::isalive(uint32_t addr) {
    if (addr < 4) {
        struct timespec current;
        clock_gettime(CLOCK_REALTIME, &current);
        if (current.tv_sec <= (alive[addr].tv_sec + 2)) {
            return 1;
        } else {
            return 0;
        }
    }else{
        return 1;
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
        return 0;
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
    while(item != 0 && success == 0){
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
        //printf("updating packet %lx with time %ld\n", item->mac, item->timeout);

    }else{
        struct device_info* dev = (struct device_info*) malloc(sizeof(struct device_info));
        memset(dev, 0, sizeof(struct device_info));
        dev->map = (struct packet_map *) malloc(sizeof(struct packet_map));
        memcpy(dev->map, map, sizeof(struct packet_map));
        dev->timeout = currenttime + 5000;
        dev->mac = map->mac;
        //printf("inserting packet %lx with time %ld\n", dev->mac, dev->timeout);
        if(device_first == 0){
            device_first = dev;
            device_last = dev;
            dev->next = 0;
            //printf("insert as start\n");
        }else{
            device_last->next = dev;
            device_last = dev;
            dev->next = 0;
        }
        
    }
    
}

struct device_info* Topology::get_device_entry(uint64_t mac){
    struct timespec current;
    clock_gettime(CLOCK_REALTIME, &current);
    unsigned long currenttime = current.tv_sec * 1000 + current.tv_nsec / 1000000;
    struct device_info *ret = 0;
    struct device_info *item = device_first;

    if (mac == 0) {
        currenttime += 5000;
        while (item != 0) {
            //printf("time comp: %ld %ld\n", item->timeout, currenttime);
            if (item->timeout <= currenttime) {
                currenttime = item->timeout;
                ret = item;
            }
            item = item->next;
        }
    } else {
        while (item != 0) {
            if (item->mac == mac && item->timeout > currenttime) {
                ret = item;
            }
            item = item->next;
        }
    }
    return ret;
}

void Topology::set_devices_reach(){
    struct device_info *item = device_first;
    while(item != 0){
        item->reachable = 0;
        item = item->next;
    }
}



void Topology::add_unexplored_entry(uint64_t mac, unsigned long timeout){
    //printf("add unexplored entry %lx, %ld\n", mac, timeout);

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
    struct topo_unexplored* old = 0;
    struct topo_unexplored* old_temp = 0;
    struct timespec current;
    clock_gettime(CLOCK_REALTIME, &current);
    unsigned long currenttime = current.tv_sec * 1000 + current.tv_nsec / 1000000;
    currenttime += 5000;

    int cnt = 0;
    while (item != 0) {
        //printf("search unexplore: %ld %ld\n", item->timeout, currenttime);
        if (item->timeout <= currenttime) {
            currenttime = item->timeout;
            ret = item;
            old = old_temp;
        }
        old_temp = item;
        item = item->next;
        cnt++;
    }
    if(ret == unexp_first){
        //printf("got last element\n");
        unexp_first = ret->next;
    }
    if(ret != 0 && old != 0){
        old->next = ret->next;
    }
    return ret;
}

struct temp_topo* Topology::search_topo(struct temp_topo*cur, uint64_t mac, uint8_t search){
    if(cur == 0){
        return 0;
    }
    struct temp_topo* ret = 0;
    struct temp_topo* temp = 0;
    //printf("search topo: %d %d\n", cur->search, search);
    //printf("searching fro %lx in %lx\n", mac, cur->mac);
    if (cur->search != search) {
        cur->search = search;
        temp = search_topo(cur->down, mac, search);
        if (temp != 0)
            ret = temp;
        temp = search_topo(cur->up, mac, search);
        if (temp != 0)
            ret = temp;
        temp = search_topo(cur->left, mac, search);
        if (temp != 0)
            ret = temp;
        temp = search_topo(cur->right, mac, search);
        if (temp != 0)
            ret = temp;

        if (cur->mac == mac) {
            //printf("found it!\n");
            ret = cur;
        }
    }
    //printf("return %p\n", ret);
    return ret;
}

void Topology::build_mapping(){
    //printf("starting building map\n");
    struct device_info* root = get_device_entry(0);
    //printf("root: %lx %ld\n", root->mac, root->timeout);
    add_unexplored_entry(root->mac, root->timeout);
    
    struct temp_topo* first = 0;
    first = (struct temp_topo *) malloc(sizeof (struct temp_topo));
    memset(first, 0, sizeof (struct temp_topo));
    first->mac = root->mac;
    struct temp_topo* current = first;
    uint8_t search_var = 1;

    while (unexp_first != 0) {
        //printf("get first item!\n");
        struct topo_unexplored* item = get_unexplored_entry();
        //printf("item :%p\n", item);
        if (item != 0) {
            current = search_topo(first, item->mac, search_var);
            if (current != 0) {
                search_var++;
                //printf("current: %p\n", current);
                struct device_info* cur = get_device_entry(item->mac);
                if (cur != 0) {
                    cur->reachable = 1;
                    free(item);
                    //printf("Current mac address %lx\n", current->mac);

                    struct device_info* temp;
                    temp = get_device_entry(cur->map->down);
                    if (temp != 0 && temp->reachable == 0) {
                        current->down = (struct temp_topo *) malloc(sizeof (struct temp_topo));
                        memset(current->down, 0, sizeof (struct temp_topo));
                        current->down->mac = temp->mac;
                        current->down->up = current;
                        add_unexplored_entry(temp->mac, temp->timeout);
                    }
                    temp = get_device_entry(cur->map->up);
                    if (temp != 0 && temp->reachable == 0) {
                        current->up = (struct temp_topo *) malloc(sizeof (struct temp_topo));
                        memset(current->up, 0, sizeof (struct temp_topo));
                        current->up->mac = temp->mac;
                        current->up->down = current;
                        add_unexplored_entry(temp->mac, temp->timeout);
                    }
                    temp = get_device_entry(cur->map->left);
                    if (temp != 0 && temp->reachable == 0) {
                        current->left = (struct temp_topo *) malloc(sizeof (struct temp_topo));
                        memset(current->left, 0, sizeof (struct temp_topo));
                        current->left->mac = temp->mac;
                        current->left->right = current;
                        add_unexplored_entry(temp->mac, temp->timeout);
                    }
                    temp = get_device_entry(cur->map->right);
                    if (temp != 0 && temp->reachable == 0) {
                        current->right = (struct temp_topo *) malloc(sizeof (struct temp_topo));
                        memset(current->right, 0, sizeof (struct temp_topo));
                        current->right->mac = temp->mac;
                        current->right->left = current;
                        add_unexplored_entry(temp->mac, temp->timeout);
                    }
                }
            }
        }
    }
    set_devices_reach();
    min_x = 0; min_y = 0; max_x = 0; max_y = 0;
    calc_topo(first, search_var, 0, 0, 0);
    search_var++;
    uint8_t offx = abs(min_x);
    uint8_t offy = abs(min_y);
    min_x = 0; min_y = 0; max_x = 0; max_y = 0;
    
    calc_topo(first, search_var, offx, offy, 0);
    search_var++;

    uint8_t num_elem = (abs(min_x + max_x) + 1) * (abs(min_y + max_y) + 1);
    size_t total_size = sizeof (struct topo_header) + num_elem * sizeof (struct topo_list);
    void *buf = malloc(total_size);
    //printf("m alloc %p size %ld\n", buf, total_size);
    //printf("allocating %p with size %ld, header is %ld\n", buf, sizeof(struct topo_header) + num_elem*sizeof(struct topo_list), sizeof(struct topo_header));
    struct topo_header* header = (struct topo_header*) buf;
    printf("needed array of %d %d\n", abs(min_x) + max_x+1, abs(min_y) + max_y+1);

    header->sizex = abs(min_x) + max_x + 1;
    header->sizey = abs(min_y) + max_y + 1;
    header->port = TOPO_PACKET;
    start = (unsigned char*) buf + sizeof (struct topo_header);
    calc_topo(first, search_var, offx, offy, 2);
    search_var++;
    //write it to buffer
    calc_topo(first, search_var, offx, offy, 1);
    search_var++;
    out_map->add(total_size, 5, buf);
    //printf("m free %p\n", buf);
    free(buf);
}


void Topology::calc_topo(struct temp_topo* cur, uint8_t search, int8_t x, int8_t y, uint8_t mode){
    if(cur != 0) {
        if (search != cur->search) {
            cur->search = search;
            cur->x = x;
            cur->y = y;
            if (mode == 0) {
                //printf("Pos: (%d %d) mac %lx\n", cur->x, cur->y, cur->mac);

                if (min_x > x)
                    min_x = x;
                if (max_x < x)
                    max_x = x;
                if (min_y > y)
                    min_y = y;
                if (max_y < y)
                    max_y = y;
                
            }
            if(mode == 2){
                //printf("max_y: %d %ld %ld\n", max_y, x*((max_y+1)*sizeof(struct topo_list)), (max_y+1)*sizeof(struct topo_list));
                struct topo_list* own = (struct topo_list*) (start + x*(max_y+1)*sizeof(struct topo_list) + y*sizeof(struct topo_list));
                //printf("adding element %d %d on pos %p with mac %ld %ld\n", x,y,own,cur->mac,sizeof(struct topo_list));
                own->mac = cur->mac;
                own->x = x;
                own->y = y;
            }
            calc_topo(cur->down, search, x, y + 1, mode);
            calc_topo(cur->up, search, x, y - 1, mode);
            calc_topo(cur->left, search, x - 1, y, mode);
            calc_topo(cur->right, search, x+1, y, mode);
            if(mode == 1){
                //printf("t free %p\n", cur);
                free(cur);
            }
        }

    }

}





