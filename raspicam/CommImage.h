/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   CommImage.h
 * Author: tobias
 *
 * Created on July 29, 2016, 9:55 AM
 */

#ifndef COMMIMAGE_H
#define COMMIMAGE_H

#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include "../network/Packetbuffer.h"
#include "../network/NetworkControl.h"
#include "RaspiTex.h"

using namespace cv;
using namespace std;

struct waiting_response{
    uint32_t id;
    uint32_t side;
    patch_packet *item;
    patch_packet *next;
    patch_packet *prev;
    struct timeval timeout;
};


class CommImage {
public:
    CommImage(NetworkControl *nc);
    CommImage(const CommImage& orig);
    virtual ~CommImage();
    void save_to_file_image(Mat *pic);
    void save_to_file_features(feature_vector* item, uint16_t file_id);
    void check_recv_buffer(patch_packet *start);
    void send_to_server(Mat *img, uint8_t mode, uint8_t pos);
    void ask_neighbours(patch_packet* item);
    //patch_packet *search_list(patch_packet* start, patch_packet *search);
    void match_recv_list(patch_packet *start);

    uint16_t file_cnt;

private:
    Packetbuffer *image_out;
    Packetbuffer *image_in;
    Packetbuffer *unrel_out;
    Packetbuffer *unrel_in;
    //void match_answers(patch_packet *start);
    patch_packet *recv_first;
    patch_packet *recv_last;
    static void callback_rel(uint32_t id, size_t size, uint8_t reason);
    void add_packet_send(uint32_t id, uint32_t side, patch_packet *item);
    void remove_packet_send(uint32_t id);
    void cleanup_packet_send();
    NetworkControl *nc;
    struct waiting_response *first_res;
    struct waiting_response *last_res;
};

#endif /* COMMIMAGE_H */

