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
#define deb_printf(fmt, args...) fprintf(stderr, "CommImage: %d:%s(): " fmt, __LINE__, __func__, ##args)
#else
#define deb_printf(fmt, args...)
#endif


CommImage::CommImage(NetworkControl *nc) {
    this->image_in = nc->image_out;
    this->image_out = nc->image_in;
    this->unrel_in = nc->unrel_out;
    this->unrel_out = nc->unrel_in;
    this->nc = nc;
    
    recv_first = 0;
    recv_last = 0;
    file_cnt = 300;
    
    first_res = 0;
    last_res = 0;
    CommImage::static_call = this;
    lock.unlock();
    
    nc->rel->setCallback(&callback_rel);
}

CommImage::CommImage(const CommImage& orig) {
}

CommImage::~CommImage() {
}

void CommImage::save_to_file_image(Mat *pic){
    deb_printf("save_to_file_image: pic %d\n", pic->total());
    char buf_pic[100];
    snprintf(buf_pic, 100, "patches/%llx_%d.png", nc->topo->mac, file_cnt);
    deb_printf("Filename: %s\n", buf_pic);
    cvtColor(*pic, *pic, CV_RGB2BGR);
    imwrite(buf_pic, *pic);
}

void CommImage::save_to_file_features(feature_vector* item, uint16_t file_id){
    char buf_fea[100];
    snprintf(buf_fea, 100, "features/%lx_%d.feature", nc->topo->mac, file_id);
    FILE * fp = fopen(buf_fea, "w");
    fwrite(item, sizeof(patch_packet), 1, fp);
    fwrite("\n", sizeof(char), 1, fp);
    fclose(fp);
}

void CommImage::send_to_server(Mat *img, uint8_t mode, uint8_t pos) {
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

void CommImage::ask_neighbours(patch_packet* item) {
    lock.lock();
    if (item->state == 0 && (((int)item->left) == 1 || ((int)item->right) == 1 ||
            ((int)item->up) == 1 || ((int)item->down) == 1)) {
        deb_printf("patch %p\n", item);
        size_t size = sizeof (patch_packet);
        deb_printf("Size first %d\n", size);
        if (item->feature != 0) {
            size += sizeof (feature_vector);
            size += sizeof(uint32_t)*(item->feature->contour->size())*2;
        }
        deb_printf("Size of Buffer %d and Contour %d\n", size, item->feature->contour->size());
        patch_packet *send_packet = (patch_packet *) malloc(size);
        deb_printf("patch packet %p\n", send_packet);
        memcpy(send_packet, item, sizeof (patch_packet));

        if (item->feature != 0) {
            send_packet->feature = (feature_vector*) (((char*) send_packet) + sizeof (patch_packet));
            deb_printf("feature vector %p\n", send_packet->feature);
            memcpy(((char*)send_packet + sizeof (patch_packet)), item->feature, sizeof (feature_vector));
            send_packet->feature->contour_size = item->feature->contour->size();
            deb_printf("Contour Size %d\n", send_packet->feature->contour_size);
            uint32_t *dest = (uint32_t*) (((char*)send_packet) + sizeof (patch_packet) + sizeof (feature_vector));
            for (int i = 0; i < item->feature->contour->size(); i++) {
                //deb_printf("accessing elem %d from contour vector %p\n", i, item->feature->contour);
                Point2i temp = item->feature->contour->at(i);
                //deb_printf("writing at pos %p with add %d\n", dest,i);
                uint32_t *array = dest+2*i;
                //deb_printf("adding point %d %d %f %f at pos %p\n", temp.x, temp.y, array);
                array[0] = temp.x;
                array[1] = temp.y;
            }
        }
        if (((int) item->down) == 1 && nc->topo->isalive(DOWN_SIDE) == 1) {
            int32_t res = image_out->add(size, DOWN_SIDE, (void *) send_packet);
            add_packet_send((uint32_t)res, DOWN_SIDE, item);
            deb_printf(" %p added buffer with %d %p to %d address\n", image_out, size, send_packet, DOWN_SIDE);
        }else{
            item->down == (patch_packet *)0;
        }
        if (((int) item->up) == 1 && nc->topo->isalive(UP_SIDE) == 1) {
            int32_t res = image_out->add(size, UP_SIDE, (void *) send_packet);
            add_packet_send((uint32_t)res, UP_SIDE, item);
            deb_printf(" %p added buffer with %d %p to %d address\n", image_out, size, send_packet, UP_SIDE);
        }else{
            item->up == (patch_packet *)0;
        }

        if (((int) item->left) == 1 && nc->topo->isalive(LEFT_SIDE) == 1) {
            int32_t res = image_out->add(size, LEFT_SIDE, (void *) send_packet);
            add_packet_send((uint32_t)res, LEFT_SIDE, item);
            deb_printf(" %p added buffer with %d %p to %d address\n", image_out, size, send_packet, LEFT_SIDE);
        }else{
            item->left == (patch_packet *)0;
        }

        if (((int) item->right) == 1 && nc->topo->isalive(RIGHT_SIDE) == 1) {
            int32_t res = image_out->add(size, RIGHT_SIDE, (void *) send_packet);
            add_packet_send((uint32_t)res, RIGHT_SIDE, item);
            deb_printf(" %p added buffer with %d %p to %d address\n", image_out, size, send_packet, RIGHT_SIDE);
        }else{
            item->right == (patch_packet *)0;
        }
        free(send_packet);
        item->state = 1;
    }
    lock.unlock();
}

/*
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
 * */

void CommImage::check_recv_buffer(patch_packet *start) {
    lock.lock();
    struct packet *pack;
    while (image_in->get(&pack) == 0) {
        
        deb_printf("GOT PACKET with size %d from addr %d pointer %p\n", pack->size, pack->addr, pack->buffer);
        patch_packet *item = (patch_packet*)pack->buffer;
        if (item->feature != 0) {
            deb_printf("item attributes: MAC %llx ID %d\n", item->mac, item->id);
            deb_printf("features found, old address %p\n", item->feature);
            item->feature = (feature_vector*) (((char *) pack->buffer) + sizeof (patch_packet));
            deb_printf("new address %p\n", item->feature);
            item->feature->contour = new vector<Point>;
            deb_printf("new contour vector initialized, contour size %d\n", item->feature->contour_size);
            uint32_t *pt = (uint32_t*) ((((char *) pack->buffer) + sizeof (patch_packet)) + sizeof (feature_vector));
            for (int i = 0; i < item->feature->contour_size; i++) {
                uint32_t *temp = pt+2*i;
                //deb_printf("Point reading out at address %p\n", temp);
                //deb_printf("Point values %d %d\n", temp[0], temp[1]);
                Point2i point(temp[0], temp[1]);
                item->feature->contour->push_back(point);
            }
        }
        deb_printf("adding item to asked item\n");
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
        deb_printf("done adding item to asked item\n");
    }
    lock.unlock();
}

void CommImage::match_recv_list(patch_packet *start) {

    lock.lock();
    //match_answers(start);
    patch_packet *item = recv_first;
    while (item != 0) {
        patch_packet *comp = start;
        uint8_t success = 0;
        while (comp != 0 && success == 0) {
            if (comp->state != 2) {
                if (((int) comp->down) == 1 && item->addr == DOWN_SIDE) {
                    uint16_t ipos1 = comp->rect_x;
                    uint16_t ipos2 = comp->rect_x + comp->rect_width;
                    uint16_t epos1 = item->rect_x;
                    uint16_t epos2 = item->rect_x + item->rect_width;
                    if ((epos1 >= ipos1 && epos1 <= ipos2) ||
                            (epos2 >= ipos1 && epos2 <= ipos2) ||
                            (epos1 <= ipos1 && epos2 >= ipos2)) {
                        deb_printf("adding packet %p to down side of %p\n", item, comp);
                        deb_printf("Down Position: %d %d %d %d\n", ipos1, ipos2, epos1, epos2);

                        deb_printf("added\n");
                        comp->down = item;
                        if (item->prev == 0 && item->next == 0) {
                            recv_first = 0;
                            recv_last = 0;
                        } else if (item->prev == 0) {
                            recv_first = item->next;
                            recv_first->prev = 0;
                        } else if (item->next == 0) {
                            item->prev->next = 0;
                            recv_last = item->prev;
                        } else {
                            item->prev->next = item->next;
                            item->next->prev = item->prev;
                        }
                        success = 1;
                    }
                } else if (((int) comp->up) == 1 && item->addr == UP_SIDE) {
                    uint16_t ipos1 = comp->rect_x;
                    uint16_t ipos2 = comp->rect_x + comp->rect_width;
                    uint16_t epos1 = item->rect_x;
                    uint16_t epos2 = item->rect_x + item->rect_width;

                    if ((epos1 >= ipos1 && epos1 <= ipos2) ||
                            (epos2 >= ipos1 && epos2 <= ipos2) ||
                            (epos1 <= ipos1 && epos2 >= ipos2)) {
                        deb_printf("adding packet %p to up side of %p\n", item, comp);
                        deb_printf("UP Position: %d %d %d %d\n", ipos1, ipos2, epos1, epos2);

                        deb_printf("added\n");
                        comp->up = item;
                        if (item->prev == 0 && item->next == 0) {
                            recv_first = 0;
                            recv_last = 0;
                        } else if (item->prev == 0) {
                            recv_first = item->next;
                            recv_first->prev = 0;
                        } else if (item->next == 0) {
                            item->prev->next = 0;
                            recv_last = item->prev;
                        } else {
                            item->prev->next = item->next;
                            item->next->prev = item->prev;
                        }
                        success = 1;
                    }
                } else if (((int) comp->left) == 1 && item->addr == LEFT_SIDE) {
                    uint16_t ipos1 = comp->rect_y;
                    uint16_t ipos2 = comp->rect_y + comp->rect_height;
                    uint16_t epos1 = item->rect_y;
                    uint16_t epos2 = item->rect_y + item->rect_height;

                    if ((epos1 >= ipos1 && epos1 <= ipos2) ||
                            (epos2 >= ipos1 && epos2 <= ipos2) ||
                            (epos1 <= ipos1 && epos2 >= ipos2)) {
                        deb_printf("adding packet %p to left side of %p\n", item, comp);
                        deb_printf("Left Position: %d %d %d %d\n", ipos1, ipos2, epos1, epos2);

                        deb_printf("added\n");
                        comp->left = item;
                        if (item->prev == 0 && item->next == 0) {
                            recv_first = 0;
                            recv_last = 0;
                        } else if (item->prev == 0) {
                            recv_first = item->next;
                            recv_first->prev = 0;
                        } else if (item->next == 0) {
                            item->prev->next = 0;
                            recv_last = item->prev;
                        } else {
                            item->prev->next = item->next;
                            item->next->prev = item->prev;
                        }
                        success = 1;
                    }
                } else if (((int) comp->right) == 1 && item->addr == RIGHT_SIDE) {
                    uint16_t ipos1 = comp->rect_y;
                    uint16_t ipos2 = comp->rect_y + comp->rect_height;
                    uint16_t epos1 = item->rect_y;
                    uint16_t epos2 = item->rect_y + item->rect_height;

                    if ((epos1 >= ipos1 && epos1 <= ipos2) ||
                            (epos2 >= ipos1 && epos2 <= ipos2) ||
                            (epos1 <= ipos1 && epos2 >= ipos2)) {
                        deb_printf("adding packet %p to right side of %p\n", item, comp);
                        deb_printf("Right Position: %d %d %d %d\n", ipos1, ipos2, epos1, epos2);

                        deb_printf("added\n");
                        comp->right = item;
                        if (item->prev == 0 && item->next == 0) {
                            recv_first = 0;
                            recv_last = 0;
                        } else if (item->prev == 0) {
                            recv_first = item->next;
                            recv_first->prev = 0;
                        } else if (item->next == 0) {
                            item->prev->next = 0;
                            recv_last = item->prev;
                        } else {
                            item->prev->next = item->next;
                            item->next->prev = item->prev;
                        }
                        success = 1;
                    }
                }
            }
            comp = comp->next;
        }
        item = item->next;
    }
    lock.unlock();
}
/*
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
*/
void CommImage::callback_rel(uint32_t id, size_t size, uint8_t reason){
    deb_printf("id: %d size: %d reason: %d\n", id, size, reason);
    if(static_call != 0)
        static_call->remove_packet_send(id);
}


//maybe use lock for access to list because of callback
void CommImage::add_packet_send(uint32_t id, uint32_t side, patch_packet* item){
    deb_printf("id: %d side: %d patch %p\n", id, side, item);
    struct waiting_response* response = (struct waiting_response*) malloc(sizeof(struct waiting_response));
    response->id = id;
    response->item = item;
    response->side = side;
    response->next = 0;
    response->prev = 0;
    struct timespec current;
    clock_gettime(CLOCK_REALTIME, &current);
    response->timeout.tv_sec = current.tv_sec+5;
    
    if(first_res == 0){
        first_res = response;
        last_res = response;
    }else{
        last_res->next = response;
        response->prev = last_res;
        last_res = response;
    }
}

void CommImage::remove_packet_send(uint32_t id){
    lock.lock();
    deb_printf("id: %d\n", id);
    struct waiting_response *item = first_res;
    uint8_t success = 0;
    struct waiting_response *freeitem = 0;
    while(item != 0 && success == 0){
        deb_printf("%d %d\n", id, item->id);
        if(item->id == id){
            deb_printf("side: %d, item %p\n", item->side, item->item);
            switch(item->side){
                case DOWN_SIDE: item->item->down = (patch_packet *)0; break;
                case UP_SIDE: item->item->up = (patch_packet *)0; break;
                case LEFT_SIDE: item->item->left = (patch_packet *)0; break;
                case RIGHT_SIDE: item->item->right = (patch_packet *)0; break;
            }
            deb_printf("finished switch statement\n");
            success = 1;
            freeitem = item;
            if (item->prev == 0 && item->next == 0) {
                first_res = 0;
                last_res = 0;
            } else if (item->prev == 0) {
                first_res = item->next;
                first_res->prev = 0;
            } else if (item->next == 0) {
                item->prev->next = 0;
                last_res = item->prev;
            } else {
                item->prev->next = item->next;
                item->next->prev = item->prev;
            }
        }
        item = item->next;
    }
    deb_printf("freeing item\n");
    if(success == 1 && freeitem != 0){
        deb_printf("free item %p\n", freeitem);
        free(freeitem);
    }
    deb_printf("freeing item2\n");

    lock.unlock();
}

void CommImage::cleanup_packet_send(){
    lock.lock();
    struct waiting_response *item = first_res;
    struct timespec current;
    clock_gettime(CLOCK_REALTIME, &current);
    while(item != 0){
        uint8_t success = 0;
        struct waiting_response *freeitem = 0;
        if (item->timeout.tv_sec < current.tv_sec) {
            freeitem = item;
            if (item->prev == 0 && item->next == 0) {
                first_res = 0;
                last_res = 0;
            } else if (item->prev == 0) {
                first_res = item->next;
                first_res->prev = 0;
            } else if (item->next == 0) {
                item->prev->next = 0;
                last_res = item->prev;
            } else {
                item->prev->next = item->next;
                item->next->prev = item->prev;
            }
            success = 1;
        }
        item = item->next;
        if (success == 1) {
            deb_printf("cleaning up item %p with id %d", freeitem, freeitem->id);
            free(freeitem);
        }
    }
    lock.unlock();
}