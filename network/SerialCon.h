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

#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>



#include "Packetbuffer.h"

//SLIP
#define END             192
#define ESC             219
#define ESC_END         220
#define ESC_ESC         221
#define SIZE_LIMIT      65000
#define NAME_TTY       "/dev/ttyUSB%d"

#define DEBUG_SERVER    "172.16.0.1"
#define DEBUG_PORT      1234
//#define NAME_TTY       "/dev/tnt%d"
//#define NAME_TTY       "/home/tobias/virtualTTY%d"


class SerialCon {
public:
    SerialCon(Packetbuffer *sendbuf, Packetbuffer *recvbuf);
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
    unsigned char recv_buf4[SIZE_LIMIT];

    
    unsigned char recv_ipc[SIZE_LIMIT];
    
    Packetbuffer *send_buf;
    Packetbuffer *recv_buf;
    
    unsigned char end = END;
    unsigned char esc = ESC;
    unsigned char esc_end = ESC_END;
    unsigned char esc_esc = ESC_ESC;
    
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int slen;
    int clen;
    
    
    
    int slip_send(unsigned char *p, uint16_t len, uint32_t nr);
    int slip_recv(unsigned char *p, unsigned char c, int *state, int*size);
    int init_serial(int nr);
    
};

#endif /* SERIALCON_H */

