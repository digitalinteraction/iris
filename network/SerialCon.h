/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SerialCon.h
 * Author: tobias
 *
 * Created on June 21, 2016, 5:27 PM
 */

#ifndef SERIALCON_H
#define SERIALCON_H
#include <termios.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <cstring>

#include <arpa/inet.h>
#include <sys/socket.h>



#include "Packetbuffer.h"

//SLIP
#define END             192
#define ESC             219
#define ESC_END         220
#define ESC_ESC         221
#define SIZE_LIMIT      2048
#define NAME_TTY       "/dev/ttyUSB%d"

#define DEBUG_SERVER    "172.16.0.1"
#define DEBUG_PORT      10000
//#define NAME_TTY       "/dev/tnt%d"
//#define NAME_TTY       "/home/tobias/virtualTTY%d"


class SerialCon {
public:
    SerialCon(Packetbuffer *sendbuf, Packetbuffer *recvbuf, uint8_t deb);
    virtual ~SerialCon();
    int processing;
    void slip_run();
    int send_array[2];
    int pipe_array[2];
    

private:
    int tty_fd;
    struct termios tio0;
    struct termios tio1;
    struct termios tio2;
    struct termios tio3;
    char *name_tty0;
    char *name_tty1;
    char *name_tty2;
    char *name_tty3;
    int fd_array[6];
    fd_set readfs;
    fd_set writefs;
    fd_set exceptfs;
    int maxfd;
    
    unsigned char recv_buf0[SIZE_LIMIT];
    int state0;
    int size0;
    unsigned char recv_buf1[SIZE_LIMIT];
    int state1;
    int size1;
    unsigned char recv_buf2[SIZE_LIMIT];
    int state2;
    int size2;
    unsigned char recv_buf3[SIZE_LIMIT];
    int state3;
    int size3;
    
    unsigned char recv_ipc[SIZE_LIMIT];
    
    Packetbuffer *send_buf;
    Packetbuffer *recv_buf;
    
    unsigned char end = END;
    unsigned char esc = ESC;
    unsigned char esc_end = ESC_END;
    unsigned char esc_esc = ESC_ESC;
    
    struct sockaddr_in client_addr;
    struct sockaddr_in server_addr;
    socklen_t slen;
    
    
    int slip_send(unsigned char *p, uint16_t len, uint32_t nr);
    int slip_recv(unsigned char *p, unsigned char c, int *state, int*size);
    int init_serial(int nr);
    struct sockaddr_in *setup_sockaddr_in(struct sockaddr_in *addr, int port, char *addr_string);
    int setup_socket(int *sockfd);
    int bind_socket(int sockfd, struct sockaddr_in *addr);
    int send_data_raw(int sockfd, unsigned char buffer[], unsigned int buffer_length, struct sockaddr_in *addr, const socklen_t slen);
    int recv_data_raw(int sockfd, unsigned char buffer[], int *recv_len, unsigned int buffer_length, struct sockaddr_in *addr, socklen_t slen);

};

#endif /* SERIALCON_H */

