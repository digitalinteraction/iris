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

#define RATIO_X ((float)HIGH_OUTPUT_X/LOW_OUTPUT_X)
#define RATIO_Y ((float)HIGH_OUTPUT_Y/LOW_OUTPUT_Y)


Low_Res_Worker::Low_Res_Worker(Packetbuffer *out, NetworkControl *nc, Buffer *images_in, Buffer *requests_out) {
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

}

Low_Res_Worker::~Low_Res_Worker() {

}

void Low_Res_Worker::run() {
    while (processing) {
        RASPITEX_PATCH *patch;
        uint8_t group;
        if (images_in->get(&patch, &group) == 0) {
            counter++;
            process_image(patch->buffer, patch->size);
            nr_img++;
            //free(patch->buffer);
            //free(patch);
        }
    }
}

void Low_Res_Worker::process_image(uint8_t *image, size_t image_size) {
    //Mat is in format BGRA
    Mat img = convert(image, image_size);
    if (img.empty() == 0) {
        cleanup_list();
        //printf("image arrived image size %d\n", img.total());
        //imwrite("test.png", img);
        //BACKGROUND SUBSTRACTOR/////////////////////////////
        /*pMOG2->apply(img, mask, learning);
        cnt++;
        if (cnt == 60) {
            printf("Learning Phase done\n");
            learning = std::numeric_limits< double >::min();
        }*/
        /////////////////////////////////////////////////////
        Mat hsv;
        cvtColor(img, hsv, COLOR_BGR2HSV);
        Mat channel[3];
        split(hsv, channel);
        //channel[1];
        threshold(channel[1], mask, 40, 255, THRESH_BINARY);
        //CLEANING UP////////////////////////////////////////
        Mat kernel = Mat::ones(3, 3, CV_8U);
        Mat cleaned;
        morphologyEx(mask, cleaned, MORPH_OPEN, kernel);
        /////////////////////////////////////////////////////

        //GET SHAPE//////////////////////////////////////////
        vector<vector<Point> > *contours = new vector<vector < Point>>;
        RNG rng(12345);
        findContours(cleaned, *contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
        /////////////////////////////////////////////////////
        match_contours(contours);
        
        vector<vector<Point> > contours_list;
        Mat drawing = Mat::zeros(cleaned.size(), CV_8UC3);
        struct objects *item = first;
        int i = 0;
        while (item != 0) {
            if (item->duration > 60) {
                //Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
                //drawContours(drawing, (item->contour), i, color, 2);
                contours_list.push_back(*(item->contour));
                i++;
            }
            item = item->next;
        }
        for (int i = 0; i < contours_list.size(); i++) {
            Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
            drawContours(drawing, contours_list, i, color, 2);
        }
        send_high_requests();
        /////////////////////////////////////////////////////
        //FIND MAX/MIN POINTS////////////////////////////////
        /*int cnt = 0;
        if(contours->size() > 0 && contours->size() < 10){
            for(int i = 0;i< contours.size();i++){
                int xmin = contours[i][0].x;
                int ymin = contours[i][0].y;
                int xmax = contours[i][0].x;
                int ymax = contours[i][0].y;
                for(int j=0;j<contours[i].size();j++){
                    if(contours[i][j].x < xmin){
                        xmin = contours[i][j].x;
                    }
                    if(contours[i][j].x > xmax){
                        xmax = contours[i][j].x;
                    }
                    if(contours[i][j].y < ymin){
                        ymin = contours[i][j].y;
                    }
                    if (contours[i][j].y > ymax) {
                        ymax = contours[i][j].y;
                    }
                }
                if ((xmax - xmin) > 10 && (ymax - ymin) > 10) {
                    requests[cnt].fb = low_patch.fb;
                    requests[cnt].token = low_patch.token;
                    //printf("Low Res: %d %d %d %d\n", xmin, ymin, xmax, ymax);
                    float factorx = (float) HIGH_OUTPUT_X / LOW_OUTPUT_X;
                    float factory = (float) HIGH_OUTPUT_Y / LOW_OUTPUT_Y;
                    float x = (xmin - 5) * factorx - factorx;
                    float y = (ymin - 5) * factory - factory;
                    if (x < 0) x = 0;
                    if (y < 0) y = 0;
                    float height = (ymax + 5) * factory + factory - y;
                    float width = (xmax + 5) * factorx + factorx - x;
                    if ((y + height) > HIGH_OUTPUT_Y) height = HIGH_OUTPUT_Y - y;
                    if ((x + width) > HIGH_OUTPUT_X) width = HIGH_OUTPUT_X - x;
                    requests[cnt].x = (int) x;
                    requests[cnt].y = (int) y;
                    requests[cnt].height = (int) (width + 1.5);
                    requests[cnt].width = (int) (height + 1.5);
                    //add limits for request in ImageCapture
                    cnt++;
                }
            }
        }
        /////////////////////////////////////////////////////
        
        if(cnt > 0){
            //requests_pending = cnt;
        }*/

        ///////////////////////////////////////////////////////////////

        Mat gray;
        cvtColor(drawing, gray, COLOR_BGR2GRAY);
        if (next_send % 2 == 0) {
            send_to_server(&gray, 1, pos);
            pos++;
            if (pos == 8) {
                pos = 0;
            }
        }
        next_send++;

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

uint8_t Low_Res_Worker::match_contours(vector<vector<Point> > *contour) {

    //printf("******************start matching contours\n");
    vector<Point> *found = 0;
    struct objects *item = first;
    while (item != 0) {
        if (item->matched == 0) {

            double similarity = 1000.0;
            Point2f new_centroid = {0.0f, 0.0f};
            double new_area = 0;
            int pos_elem = 0;

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
                    }
                }
            }
            if (similarity <= 0.5 && similarity >= -0.5) {
                //printf("found match %d with similarity %f\n", item->id, similarity);
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
            }
        }
        item = item->next;
    }

    //printf("size left %d\n", contour->size());
    for (int i = 0; i < contour->size(); i++) {
        //printf("contours found: %f\n", contourArea(contour->at(i)));
        if (contourArea(contour->at(i)) > CONTOUR_LOWER_THRESHOLD) {
            Moments mu = moments(contour->at(i), false);
            Point2f mc = Point2f(mu.m10 / mu.m00, mu.m01 / mu.m00);
            //calculate similarity of shape
            float area = mu.m00;
            item = (struct objects *) malloc(sizeof (struct objects));
            item->contour = &contour->at(i);
            item->id = this->id_cnt++;
            item->expiring = 0;
            item->area = area;
            item->centroid = mc;
            item->asked = 0;
            if (first == 0) {
                //printf("added item with id %d as first\n", item->id);
                first = item;
                last = first;
                first->next = 0;
                first->prev = 0;
            } else {
                //printf("added item with id %d\n", item->id);
                item->next = 0;
                item->prev = last;
                last->next = item;
                last = item;
            }
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
        if (item->expiring == 30) {
            //printf("freeing item %p with id %d\n", item, item->id);
            if (item->prev == 0 && item->next == 0) {
                first = 0;
                last = 0;
            } else if (item->prev == 0) {
                first = item->next;
                first->prev = 0;
            } else if (item->next == 0) {
                item->prev->next = 0;
                last = item->prev;
            } else {
                item->prev->next = item->next;
                item->next->prev = item->prev;
            }
            //delete item->contour;
            free(item);
        } else {
            item->expiring++;
        }
        item = item->next;
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
        if (item->duration >= 60 && item->asked == 0) {
            //send request to high res worker
            Rect enclosing = boundingRect(*item->contour);

            RASPITEX_PATCH* temp = (RASPITEX_PATCH *) malloc(sizeof (RASPITEX_PATCH));
            memset(temp, 0, sizeof (RASPITEX_PATCH));
            //add some buffer
            temp->x = (int32_t)(((float)enclosing.x)*RATIO_X - BORDER_HIGH_RES);
            if(temp->x < 0) temp->x = 0;
            temp->y = (int32_t)(((float)enclosing.y)*RATIO_Y - BORDER_HIGH_RES);
            if(temp->y < 0) temp->y = 0;
            temp->height = (int32_t)(((float)enclosing.height)*RATIO_Y + BORDER_HIGH_RES);
            if(temp->height > HIGH_OUTPUT_Y) temp->height = HIGH_OUTPUT_Y;
            temp->width = (int32_t)(((float)enclosing.width)*RATIO_X + BORDER_HIGH_RES);
            if(temp->width > HIGH_OUTPUT_X) temp->width = HIGH_OUTPUT_X;

            if (enclosing.x == 0) {
                temp->down = 1;
            }
            if (enclosing.y == 0) {
                temp->left = 1;
            }
            if ((enclosing.x + enclosing.height) == LOW_OUTPUT_X) {
                temp->up = 1;
            }
            if ((enclosing.y + enclosing.width) == LOW_OUTPUT_Y) {
                temp->right = 1;
            }
            requests_out->add(temp, 0);
            item->asked = 1;
        }
        item = item->next;
    }

}