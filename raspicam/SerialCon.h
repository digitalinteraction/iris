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

//SLIP
#define END             0300    /* indicates end of packet */
#define ESC             0333    /* indicates byte stuffing */
#define ESC_END         0334    /* ESC ESC_END means END data byte */
#define ESC_ESC         0335    /* ESC ESC_ESC means ESC data byte */
#define SIZE_LIMIT      2048
#define NAME_TTY       "/dev/ttyUSB%d"


class SerialCon {
public:
    SerialCon();
    SerialCon(const SerialCon& orig);
    virtual ~SerialCon();
    int processing;
    int slip_run();
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
    int fd_array[5];
    fd_set readfs;
    int maxfd;
    
    char recv_buf0[SIZE_LIMIT];
    int state0;
    int size0;
    char recv_buf1[SIZE_LIMIT];
    int state1;
    int size1;
    char recv_buf2[SIZE_LIMIT];
    int state2;
    int size2;
    char recv_buf3[SIZE_LIMIT];
    int state3;
    int size3;
    
    char recv_ipc[SIZE_LIMIT];
    
    int slip_send(char *p, uint16_t len, int nr);
    int slip_recv(char *p, int fd, int *state, int*size);
    int init_serial(int nr);

};

#endif /* SERIALCON_H */

