/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Low_Res_Worker.cpp
 * Author: tobias
 * 
 * Created on May 3, 2016, 10:44 AM
 */

#include "Low_Res_Worker.h"
#include "tga.h"
#include "network_defines.h"
#include <limits>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//#include "Buffer.h"

#define COLOR

#ifdef DEBUG_LOW_RES
#define deb_printf(fmt, args...) fprintf(stderr, "LOW_RES_WORKER: %d:%s(): " fmt, __LINE__, __func__, ##args)
#else
#define deb_printf(fmt, args...)
#endif


#define RATIO_X ((float)HIGH_OUTPUT_X/LOW_OUTPUT_X)
#define RATIO_Y ((float)HIGH_OUTPUT_Y/LOW_OUTPUT_Y)

const char * const classifier_names[] = {"NOTHING", "CARROT", "PEACH", "APPLE"};
int fontFace = CV_FONT_HERSHEY_PLAIN;
double fontScale = 1;
int fontThickness = 1;

Low_Res_Worker::Low_Res_Worker(Packetbuffer *out, NetworkControl *nc, Buffer *images_in, Buffer *requests_out, Buffer *class_in) {
    processing = 0;
    cnt = 0;
    learning = 0.05;
    new_low_buffer = 0;
    requests_pending = 0;
    nr_img = 0;
    id_cnt = 0;
    this->images_in = images_in;
    this->requests_out = requests_out;
    if (pthread_mutex_init(&buffer_lock, NULL) != 0) {
        printf("mutex init failed\n");
    }

    this->out = out;
    this->nc = nc;
    pos = 0;
    next_send = 0;
    this->first = 0;
    this->last = 0;
    cnt_size = 0;
    this->class_in = class_in;
    running = 0;
    //pMOG2 = cv::bgsegm::createBackgroundSubtractorGMG();

}

Low_Res_Worker::~Low_Res_Worker() {

}

void Low_Res_Worker::run() {
    while (processing) {
        running = 11;
        RASPITEX_PATCH *patch;
        uint8_t group;
        if (images_in->get(&patch, &group) == 0) {
            running = 12;
            counter++;
            process_image(patch->buffer, patch->size);
            nr_img++;
            running = 13;
            free(patch->buffer);
            free(patch);
        }
        running = 15;
        update_contours();
    }
}

void Low_Res_Worker::process_image(uint8_t *image, size_t image_size) {
    //Mat is in format BGRA
    Mat img = convert(image, image_size);
    if (img.empty() == 0) {
        running = 10;
        deb_printf("cleaning up list\n");
        cleanup_list();
        running = 1;
        Mat hsv, rgb;
        cvtColor(img, rgb, COLOR_RGBA2RGB);
        cvtColor(rgb, hsv, COLOR_RGB2HSV);
        Mat channel[3];
        split(hsv, channel);
        threshold(channel[1], mask, 50, 255, THRESH_BINARY);
        deb_printf("thresholded image\n");
        //pMOG2->apply(img, mask);
        //CLEANING UP////////////////////////////////////////
        Mat kernel = Mat::ones(3, 3, CV_8U);
        Mat cleaned;
        morphologyEx(mask, cleaned, MORPH_OPEN, kernel);
        /////////////////////////////////////////////////////
        running = 2;
        //GET SHAPE//////////////////////////////////////////
        vector<vector<Point> > *contours = new vector<vector < Point>>;
        RNG rng(12345);
        findContours(cleaned, *contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
        /////////////////////////////////////////////////////
        deb_printf("matching contours\n");
        match_contours(contours, low_patch.token, low_patch.fb);
        vector<vector<Point> > contours_list;
        Mat drawing = Mat::zeros(cleaned.size(), CV_8UC3);
        struct objects *item = first;
        int i = 0;
        while (item != 0) {
            if (item->duration > 50) {
                //Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
                //drawContours(drawing, (item->contour), i, color, 2);
                //printf("item duration %d\n", item->duration);
                contours_list.push_back(*(item->contour));
                i++;
            }
            item = item->next;
        }
        running = 3;
        for (int i = 0; i < contours_list.size(); i++) {
            Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
            drawContours(img, contours_list, i, color, 2);
        }
        item = first;
        while(item != 0){
            if(item->duration > 50){
                if(item->classification >= 0){
                    const char * text = classifier_names[item->classification];
                    //printf("image classification is %d %s\n", item->classification, text);
                    Point2f middle = item->centroid;
                    int baseline = 0;
                    Size textSize = getTextSize(text, fontFace, fontScale, fontThickness, &baseline);
                    middle.y = middle.y+textSize.height/2 + 2;
                    middle.x = middle.x - textSize.width/2;
                    putText(img, text, middle, fontFace, fontScale, Scalar::all(255), fontThickness, 8, true);
                }
                if(item->object >= 0 || item->object == -2){
                    char text[20];
                    if(item->object == -2){
                        snprintf(text, 20, "Item saved");
                    }else{
                        snprintf(text, 20, "Object %d", item->object);
                    }
                    Point2f middle = item->centroid;
                    int baseline = 0;
                    Size textSize = getTextSize(text, fontFace, fontScale, fontThickness, &baseline);
                    middle.y = middle.y-textSize.height/2 - 2;
                    middle.x = middle.x - textSize.width/2;
                    putText(img, text, middle, fontFace, fontScale, Scalar::all(255), fontThickness, 8, true);
                }
            }
            item = item->next;
        }
        running = 4;
        deb_printf("sending request for high res image\n");
        send_high_requests();

        deb_printf("sending image data to server\n");
        Mat gray;
        cvtColor(img, gray, COLOR_BGR2GRAY);
        if (next_send % 2 == 0) {
            
#ifdef COLOR
            Mat send_img;
            cvtColor(img, send_img, COLOR_BGRA2RGB);
            send_to_server(&send_img, 3, pos);
#else
            send_to_server(&gray, 1, pos);
#endif
            pos++;
            if (pos == 8) {
                pos = 0;
            }
        }
        next_send++;
        running = 5;
    } else {
        printf("Failed to convert camera image to Mat\n");
    }
}

Mat Low_Res_Worker::convert(uint8_t* image, size_t image_size) {
    if (image_size == (LOW_OUTPUT_X * LOW_OUTPUT_Y * 4)) {
        Mat mat_image(LOW_OUTPUT_Y, LOW_OUTPUT_X, CV_8UC4, (void*) image);
        return mat_image;
    } else {
        Mat null;
        return null;
    }
}

void Low_Res_Worker::send_to_server(Mat *img, uint8_t mode, uint8_t pos) {
    //printf("sending image to server %ld with pos %d \n", image_size, pos);
    if (nc->unrel->send_buf->getCnt() < 70) {

        if ((img->total() * img->elemSize()) != 0) {
            uint32_t addr;
            if (inet_aton("172.16.0.1", (in_addr *) & addr) == 0) {
                printf("inet_aton() failed\n");
            }

            uint32_t new_size = img->total() * img->elemSize();

            size_t part_size = new_size / 8;
            //int ret = 0;
            //for (int i = 0; i < 10; i++) {
            //ret = 0;
            printf("sending packet with size %d %d\n", new_size, part_size);
            uint32_t size = part_size + sizeof (struct low_res_header);
            struct low_res_header * header = (struct low_res_header *) malloc(size);
            memcpy((((unsigned char *) header) + sizeof (struct low_res_header)), (void*) (img->data + pos * part_size), part_size);
            header->mac = nc->topo->mac;
            header->port = IMAGE_PACKET;
            header->pos = pos;
            header->mac = nc->topo->mac;
            header->size = part_size;
            header->weight = nc->debug->get_weight();
            //ret = out->add(size, addr, (void*) header);
            out->add(size, addr, (void*) header);
            //if (ret != 0)
            //    i--;
            free(header);

            //}
            //printf("Send 10 packets: buffer length: %d\n", out->getCnt());
        }

    }
}

uint8_t Low_Res_Worker::match_contours(vector<vector<Point> > *contour, uint8_t token, uint8_t fb) {

    deb_printf("******************start matching contours\n");
    vector<Point> *found = 0;
    struct objects *item = first;
    while (item != 0) {
        if (item->matched == 0) {

            double similarity = 1000.0;
            Point2f new_centroid = {0.0f, 0.0f};
            double new_area = 0;
            int pos_elem = 0;
            float movement = 0;

            for (int i = 0; i < contour->size(); i++) {
                if (contourArea(contour->at(i)) > CONTOUR_LOWER_THRESHOLD) {
                    //calculate centroid
                    Moments mu = moments(contour->at(i), false);
                    Point2f mc = Point2f(mu.m10 / mu.m00, mu.m01 / mu.m00);

                    //calculate similarity of shape
                    double sim_shapes = matchShapes(*(item->contour), contour->at(i), CV_CONTOURS_MATCH_I1, 0);

                    float area = mu.m00;

                    float diff_area = (item->area - area) / (item->area);
                    Point2f diff_xy = item->centroid - mc;
                    float diff_dist = sqrt(diff_xy.x * diff_xy.x + diff_xy.y * diff_xy.y);

                    double total_sim = 0.003 * diff_dist + 0.01 * diff_area + sim_shapes;

                    if (total_sim < similarity) {
                        similarity = total_sim;
                        found = &contour->at(i);
                        new_centroid = mc;
                        new_area = area;
                        pos_elem = i;
                        movement = diff_dist;
                    }
                }
            }
            if (similarity <= 0.2 && similarity >= -0.2) {
                deb_printf("found match %d with similarity %f\n", item->id, similarity);
                item->contour = new vector<Point>;
                *item->contour = *found;
                contour->erase(contour->begin() + pos_elem);
                item->area = new_area;
                item->centroid = new_centroid;
                item->expiring = 0;
                item->matched = 1;
                if (item->duration != 255) {
                    item->duration++;
                }
                item->movement = movement;

                if (movement <= MOVEMENT_ALLOWED) {
                    if (item->move_cnt != 255){
                        item->move_cnt++;
                    }
                }else{
                    item->move_cnt = 0;
                }
                deb_printf("end of similarity matching\n");
            }
        }
        item = item->next;
    }

    deb_printf("size left %d\n", contour->size());
    for (int i = 0; i < contour->size(); i++) {
        deb_printf("contours found: %f\n", contourArea(contour->at(i)));
        if (contourArea(contour->at(i)) > CONTOUR_LOWER_THRESHOLD) {
            Moments mu = moments(contour->at(i), false);
            Point2f mc = Point2f(mu.m10 / mu.m00, mu.m01 / mu.m00);
            //calculate similarity of shape
            float area = mu.m00;
            item = (struct objects *) malloc(sizeof (struct objects));
            item->contour = &contour->at(i);
            item->id = this->id_cnt++;
            item->expiring = 0;
            item->duration = 0;
            item->area = area;
            item->centroid = mc;
            item->asked = 0;
            item->fb = fb;
            item->token = token;
            item->next = 0;
            item->prev = 0;
            item->classification = -1;
            item->object = -1;
            if (first == 0) {
                deb_printf("added item with id %d as first\n", item->id);
                first = item;
                last = first;
                first->next = 0;
                first->prev = 0;
            } else {
                deb_printf("added item with id %d\n", item->id);
                item->next = 0;
                item->prev = last;
                last->next = item;
                last = item;
            }
            cnt_size++;
        }
    }

    item = first;
    while (item != 0) {
        item->matched = 0;
        item = item->next;
    }
    return 0;
}

void Low_Res_Worker::cleanup_list() {
    struct objects *item = first;
    while (item != 0) {
        uint8_t success = 0;
        struct objects *freeitem = 0;
        if (item->expiring == 30) {
            deb_printf("freeing item %p with id %d\n", item, item->id);
            //printf("freeing item %p with id %d\n", item, item->id);
            if (item->prev == 0 && item->next == 0) {
                deb_printf("setting first and last to 0, removing %p\n", item);
                first = 0;
                last = 0;
            } else if (item->prev == 0) {
                deb_printf("setting %p to first, removing %p\n", item->next, item);
                first = item->next;
                first->prev = 0;
            } else if (item->next == 0) {
                deb_printf("setting %p to last, removing %p\n", item->prev, item);
                item->prev->next = 0;
                last = item->prev;
            } else {
                deb_printf("setting %p as prev and %p as next, removing %p\n", item->prev, item->next, item);
                item->prev->next = item->next;
                item->next->prev = item->prev;
            }
            //delete item->contour;
            cnt_size--;
            success = 1;
            freeitem = item;
        } else {
            item->expiring++;
        }
        item = item->next;
        if(success == 1){
            //deb_printf("delete contour %p and %p\n", freeitem->contour, freeitem);
            //delete *freeitem->contour;
            free(freeitem);
        }
    }

}

/*
void Low_Res_Worker::ask_neighbours(struct objects *item) {
    if (item->asked_ext < 2) {
        int ret = 0;
        Rect enclosing = boundingRect(*item->contour);
        if (enclosing.x == 0) {
            //ask neighbour below 3
            ret += send_to_neighbour(enclosing.y, enclosing.y + enclosing.width, 3, item->id);
        }
        if (enclosing.y == 0) {
            //ask neighbour to the left 1
            ret += send_to_neighbour(enclosing.x, enclosing.x + enclosing.height, 1, item->id);

        }
        if ((enclosing.x + enclosing.height) == LOW_OUTPUT_X) {
            //ask neighbour up 0
            ret += send_to_neighbour(enclosing.y, enclosing.y + enclosing.width, 0, item->id);
        }
        if ((enclosing.y + enclosing.width) == LOW_OUTPUT_Y) {
            //ask neighbour right 2
            ret += send_to_neighbour(enclosing.x, enclosing.x + enclosing.height, 2, item->id);
        }
        if (ret == 0) {
            item->asked_ext = 2;
        } else {
            item->asked_ext = 1;
        }
    }
}

int Low_Res_Worker::send_to_neighbour(uint16_t pos1, uint16_t pos2, uint8_t addr, uint16_t id) {
    struct low_res_request temp;
    temp.pos1 = pos1;
    temp.pos2 = pos2;
    temp.id = id;
    temp.request = 0;
    return nc->image_in->add(sizeof (struct low_res_request), (uint32_t) addr, &temp);
}

void Low_Res_Worker::check_list() {
    struct objects *item = first;
    while (item != 0) {
        if (item->duration > 60) {
            if (item->asked_int == 0) {
                item->asked_int = 1;
                //send request to high res thread
                ask_neighbours(item);
            }
            if (item->asked_ext == 1) {
                ask_neighbours(item);
            }
        }
    }
}

void recv_packet(struct packet * pack){
    struct low_res_request *header = (struct low_res_request*) pack->buffer;
    switch(header->request){
        case LOW_RES_REQUEST_PACK:
            //search contours for match
            //request feature vector from high res thread
            //send feature vector back
            //no need to ask neighbour, mark contour as externally asked
            break;
        case LOW_RES_REPLY_PACK:
            
            break;
        default:
            break;
    }
}
    */

void Low_Res_Worker::send_high_requests(){
    struct objects *item = first;
    while (item != 0) {
        if (item->duration >= 30 && item->asked == 0 && item->move_cnt > 15) {
            //send request to high res worker
            Rect enclosing = boundingRect(*item->contour);

            RASPITEX_PATCH* temp = (RASPITEX_PATCH *) malloc(sizeof (RASPITEX_PATCH));
            memset(temp, 0, sizeof (RASPITEX_PATCH));
            temp->id = item->id;
            temp->token = item->token;
            temp->fb = item->fb;
            //add some buffer
            temp->x = (int32_t)(((float)enclosing.x)*RATIO_X - BORDER_HIGH_RES);
            if(temp->x < 0) temp->x = 0;
            temp->y = (int32_t)(((float)enclosing.y)*RATIO_Y - BORDER_HIGH_RES);
            if(temp->y < 0) temp->y = 0;
            temp->height = (int32_t)(((float)enclosing.height)*RATIO_Y + BORDER_HIGH_RES);
            if(temp->height > HIGH_OUTPUT_Y) temp->height = HIGH_OUTPUT_Y;
            temp->width = (int32_t)(((float)enclosing.width)*RATIO_X + BORDER_HIGH_RES);
            if(temp->width > HIGH_OUTPUT_X) temp->width = HIGH_OUTPUT_X;

            if (enclosing.x < 5) {
                temp->right = 1;
            }
            if (enclosing.y < 5) {
                temp->down = 1;
            }
            if ((enclosing.x + enclosing.width) > (LOW_OUTPUT_X-5)) {
                temp->left = 1;
            }
            if ((enclosing.y + enclosing.height) > (LOW_OUTPUT_Y-5)) {
                temp->up = 1;
            }
            printf("sending request for %d %d %d %d or %d %d %d %d\n", enclosing.x, enclosing.y, enclosing.height, enclosing.width, temp->x, temp->y, temp->height, temp->width);
            printf("with sides %d %d %d %d\n", temp->down, temp->up, temp->left, temp->right);
            requests_out->add(temp, 0);
            item->asked = 1;
        }
        item = item->next;
    }

}

void Low_Res_Worker::update_contours() {
    struct classification_result *class_item = 0;
    uint8_t group = 0;
    while (class_in->get((RASPITEX_PATCH**)&class_item, &group) == 0) {
        if (class_item != 0) {
            struct objects *item = first;
            while (item != 0) {
                if (class_item->id == item->id) {
                    if(class_item->classification != -1){
                        item->classification = class_item->classification;
                    }
                    if(class_item->object != -1){
                        item->object = class_item->object;
                    }
                }
                item = item->next;
            }
        }
        free(class_item);
    }

}