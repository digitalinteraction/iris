/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   CommImage.cpp
 * Author: tobias
 * 
 * Created on July 29, 2016, 9:55 AM
 */

#include "CommImage.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

#include "Buffer.h"

#ifdef DEBUG_COMM_IMAGE
#define deb_printf(format, ...) printf("ComImage::" format)
#else
#define deb_printf(format, ...) {}
#endif


CommImage::CommImage(NetworkControl *nc) {
    this->image_in = nc->image_out;
    this->image_out = nc->image_in;
    this->unrel_in = nc->unrel_out;
    this->unrel_out = nc->unrel_in;
    this->nc = nc;
    
    recv_first = 0;
    recv_last = 0;
    file_cnt = 0;
}

CommImage::CommImage(const CommImage& orig) {
}

CommImage::~CommImage() {
}

void CommImage::save_to_file_image(Mat *pic){
#ifdef DEBUG_COMM_IMAGE
    printf("save_to_file_image: pic %d\n", pic->total());
#endif
    char buf_pic[100];
    snprintf(buf_pic, 100, "patches/%ld%d.png", nc->topo->mac, file_cnt);
    imwrite(buf_pic, *pic);
}

void CommImage::save_to_file_features(feature_vector* item, uint16_t file_id){
#ifdef DEBUG_COMM_IMAGE
    printf("save_to_file_features: id %d\n", file_id);
#endif
    char buf_fea[100];
    snprintf(buf_fea, 100, "features/%ld%d.feature", nc->topo->mac, file_id);
    FILE * fp = fopen(buf_fea, "w");
    fwrite(item, sizeof(patch_packet), 1, fp);
    fwrite("\n", sizeof(char), 1, fp);
    fclose(fp);
}

void CommImage::send_to_server(Mat *img, uint8_t mode, uint8_t pos) {
#ifdef DEBUG_COMM_IMAGE
    printf("send_to_server: %d %d\n", mode, pos);
#endif
    if (nc->unrel->send_buf->getCnt() < 70) {

        if ((img->total() * img->elemSize()) != 0) {
            uint32_t addr;
            if (inet_aton("172.16.0.1", (in_addr *) & addr) == 0) {
                printf("inet_aton() failed\n");
            }

            uint32_t new_size = img->total() * img->elemSize();
            size_t part_size = new_size / 8;

            uint32_t size = part_size + sizeof (struct low_res_header);
            struct low_res_header * header = (struct low_res_header *) malloc(size);
            memcpy((((unsigned char *) header) + sizeof (struct low_res_header)), (void*) (img->data + pos * part_size), part_size);
            header->mac = nc->topo->mac;
            header->port = IMAGE_PACKET;
            header->pos = pos;
            header->mac = nc->topo->mac;
            header->size = part_size;
            header->weight = nc->debug->get_weight();
            unrel_out->add(size, addr, (void*) header);
            free(header);
        }
    }
}

void CommImage::ask_neighbours(patch_packet* item){
#ifdef DEBUG_COMM_IMAGE
    printf("ask_neighbours: %d\n", item->id);
#endif
    size_t size = sizeof(::patch_packet);
    deb_printf("Size first %d\n", size);
    if(item->feature != 0){
        size += sizeof(::feature_vector);
        size += sizeof(cv::Point2i)*(item->feature->contour->size());
    }
    deb_printf("Size of Buffer %d and Contour %d\n", size, item->feature->contour->size());
    patch_packet *send_packet = (patch_packet *)malloc(size);
    memcpy(send_packet, item, sizeof(patch_packet));
    send_packet->feature->contour_size = item->feature->contour->size();
    deb_printf("Contour Size\n", send_packet->feature->contour_size);
    if(item->feature != 0){
        memcpy(send_packet+sizeof(patch_packet), item->feature, sizeof(feature_vector));
        Point2i *p = (Point2i*)item->feature->contour->data();
        Point2i *dest = (Point2i*) (send_packet+sizeof(patch_packet) + sizeof(feature_vector));
        for(int i = 0; i < item->feature->contour->size(); i++){
            Point2i temp = item->feature->contour->at(i);
            deb_printf("adding point %d %d\n", temp.x, temp.y);
            //memcpy(dest[i], p[i], sizeof(Point2i));
            dest[i].x = (int)p[i].x;
            dest[i].y = (int)p[i].y;
        }
    }
    deb_printf("Memcopied points\n");
    if(((int)item->down) == 1){
        image_out->add(size, DOWN_SIDE, (void *) send_packet);
    }
    deb_printf(" %p added buffer with %d %p to %d address\n", image_out, size, send_packet, DOWN_SIDE);
    if(((int)item->up) == 1){
        image_out->add(size, UP_SIDE, (void *) send_packet);
    }
    deb_printf(" %p added buffer with %d %p to %d address\n", image_out, size, send_packet, UP_SIDE);

    if(((int)item->left) == 1){
        image_out->add(size, LEFT_SIDE, (void *) send_packet);
    }
    deb_printf(" %p added buffer with %d %p to %d address\n", image_out, size, send_packet, LEFT_SIDE);

    if(((int)item->right) == 1){
        image_out->add(size, RIGHT_SIDE, (void *) send_packet);
    }
    deb_printf(" %p added buffer with %d %p to %d address\n", image_out, size, send_packet, RIGHT_SIDE);

    item->state = 1;
}


patch_packet * CommImage::search_list(patch_packet* start, patch_packet *search){
#ifdef DEBUG_COMM_IMAGE
    printf("search_list: %d %d\n", search->mac, search->id);
#endif
    patch_packet *item = start;
    patch_packet *ret = 0;
    uint8_t success = 0;
    while(item != 0 && success == 0){
        if(search->id == item->id && search->mac == item->mac){
            success = 1;
            ret = item;
        }
        item = item->next;
    }
    return ret;
}

void CommImage::check_recv_buffer(patch_packet *start) {
    struct packet *pack;
    while (image_in->get(&pack) == 0) {
        patch_packet *item = (patch_packet*)pack->buffer;
        if (item->feature != 0) {
            item->feature = (feature_vector*) (((char *) pack->buffer) + sizeof (patch_packet));
            item->feature->contour = new vector<Point>;
            Point *pt = (Point*) ((((char *) pack->buffer) + sizeof (patch_packet)) + sizeof (feature_vector));
            for (int i = 0; i < item->feature->contour_size; i++) {
                item->feature->contour->push_back(*(pt + i));
            }
        }
        item->addr = pack->addr;
        if (recv_first == 0) {
            recv_first = item;
            recv_last = recv_first;
            recv_first->next = 0;
            recv_first->prev = 0;
        } else {
            item->next = 0;
            item->prev = recv_last;
            recv_last->next = item;
            recv_last = item;
        }
    }
}

void CommImage::match_recv_list(patch_packet *start){
#ifdef DEBUG_COMM_IMAGE
    printf("match_recv_list\n");
#endif
    match_answers(start);
    patch_packet *item = recv_first;
    while(item != 0){
        patch_packet *comp = start;
        while(comp != 0){
            if(comp->state == 0){
                if(((int)comp->down) == 1 && item->addr == DOWN_SIDE){
                    uint16_t ipos1 = comp->rect_x;
                    uint16_t ipos2 = comp->rect_x + comp->rect_width;
                    uint16_t epos1 = item->rect_x;
                    uint16_t epos2 = item->rect_x + item->rect_width;
                    if((epos1 >= ipos1 && epos1 <= ipos2) ||
                            (epos2 >= ipos1 && epos2 <= ipos2) ||
                            (epos1 <= ipos1 && epos2 >= ipos2)){
                        comp->down = item;
                        item->prev->next = item->next;
                        item->next->prev = item->prev;
                    }
                }
                if(((int)comp->up) == 1 && item->addr == UP_SIDE){
                    uint16_t ipos1 = comp->rect_x;
                    uint16_t ipos2 = comp->rect_x + comp->rect_width;
                    uint16_t epos1 = item->rect_x;
                    uint16_t epos2 = item->rect_x + item->rect_width;
                    if((epos1 >= ipos1 && epos1 <= ipos2) ||
                            (epos2 >= ipos1 && epos2 <= ipos2) ||
                            (epos1 <= ipos1 && epos2 >= ipos2)){
                        comp->up = item;
                        item->prev->next = item->next;
                        item->next->prev = item->prev;
                    }
                }
                if(((int)comp->left) == 1 && item->addr == LEFT_SIDE){
                    uint16_t ipos1 = comp->rect_y;
                    uint16_t ipos2 = comp->rect_y + comp->rect_height;
                    uint16_t epos1 = item->rect_y;
                    uint16_t epos2 = item->rect_y + item->rect_height;
                    if((epos1 >= ipos1 && epos1 <= ipos2) ||
                            (epos2 >= ipos1 && epos2 <= ipos2) ||
                            (epos1 <= ipos1 && epos2 >= ipos2)){
                        comp->left = item;
                        item->prev->next = item->next;
                        item->next->prev = item->prev;
                    }
                }
                if(((int)comp->right) == 1 && item->addr == RIGHT_SIDE){
                    uint16_t ipos1 = comp->rect_y;
                    uint16_t ipos2 = comp->rect_y + comp->rect_height;
                    uint16_t epos1 = item->rect_y;
                    uint16_t epos2 = item->rect_y + item->rect_height;
                    if((epos1 >= ipos1 && epos1 <= ipos2) ||
                            (epos2 >= ipos1 && epos2 <= ipos2) ||
                            (epos1 <= ipos1 && epos2 >= ipos2)){
                        comp->right = item;
                        item->prev->next = item->next;
                        item->next->prev = item->prev;
                    }
                }
            }
            comp = comp->next;
        }
        item = item->next;
    }
}

void CommImage::match_answers(patch_packet *start) {
#ifdef DEBUG_COMM_IMAGE
    printf("match_answers\n");
#endif
    patch_packet *item = recv_first;
    while (item != 0) {
        patch_packet *match = search_list(start, item);
        if (match != 0) {
            switch (item->addr) {
                case DOWN_SIDE:
                    if (((int) match->down) == 1) {
                        match->down = item;
                    }
                    break;
                case UP_SIDE:
                    if (((int) match->up) == 1) {
                        match->up = item;
                    }
                    break;
                case LEFT_SIDE:
                    if (((int) match->left) == 1) {
                        match->left = item;
                    }
                    break;
                case RIGHT_SIDE:
                    if (((int) match->right) == 1) {
                        match->right = item;
                    }
                    break;
            }
            item->next->prev = item->prev;
            item->prev->next = item->next;
        }
        item = item->next;
    }
}
