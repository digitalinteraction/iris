/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SerialCon.cpp
 * Author: tobias
 * 
 * Created on June 21, 2016, 5:27 PM
 */

#include "SerialCon.h"


SerialCon::SerialCon(Packetbuffer *sendbuf, Packetbuffer *recvbuf) {
    fd_array[0] = init_serial(0);
    fd_array[1] = init_serial(1);
    fd_array[2] = init_serial(2);
    fd_array[3] = init_serial(3);
    send_buf = sendbuf;
    recv_buf = recvbuf;
    fd_array[4] = send_buf->signalfd;
    if(fd_array[4] == -1){
        printf("Error in eventfd creation\n");
    }
    //fd_array[5] = pipe_array[1];

    int i, max=0;
    for(i=0;i<5;i++){
            if(fd_array[i] >= 0){
                if(fd_array[i]>max){
                    max = fd_array[i];
                }
            }else{
                printf("File Descriptor %d could not be initialized; Error Code %d\n", i, fd_array[i]);
            }
    }
    maxfd = max;
    memset(recv_buf0, 0, SIZE_LIMIT);
    state0 = 0;
    memset(recv_buf1, 0, SIZE_LIMIT);
    state1 = 0;
    memset(recv_buf2, 0, SIZE_LIMIT);
    state2 = 0;
    memset(recv_buf3, 0, SIZE_LIMIT);
    state3 = 0;
    
}


SerialCon::~SerialCon() {
}

int SerialCon::slip_send(unsigned char *p, uint16_t len, int nr) {
    write(fd_array[nr], &end,1);

    while (len--) {
        switch (*p) {
            case END:
                //send_char(ESC);
                //send_char(ESC_END);
                write(fd_array[nr], &esc,1);
                write(fd_array[nr], &esc_end,1);
                break;
            case ESC:
                //send_char(ESC);
                //send_char(ESC_ESC);
                write(fd_array[nr], &esc,1);
                write(fd_array[nr], &esc_esc,1);
                break;
            default:
                //send_char(*p);
                write(fd_array[nr], p,1);
        }
        p++;
    }
    //send_char(END);
    write(fd_array[nr], &end,1);
}

int SerialCon::slip_recv(unsigned char *p, int fd, int *state, int*size) {
    
    unsigned char c = 0;
    while(read(fd, &c, 1)){
        switch (c) {
            case END:
                if(*state == 0){
                    *state = 1; 
                }else if(*state == 1){
                    *state = 2;
                    p[*size++] = 0;
                    return 0;
                }
            case ESC:
                read(fd, &c, 1);
                switch (c) {
                    case ESC_END:
                        c = END;
                        break;
                    case ESC_ESC:
                        c = ESC;
                        break;
                }
            default:
                if ((*size) < SIZE_LIMIT)
                    p[*size++] = c;
        }
    }
    return 1;
}

void SerialCon::slip_run(){
    int i;
    struct timeval Timeout;
    Timeout.tv_usec = 500;
    Timeout.tv_sec = 2;
    int res;
    //char test[] = "testing";
    

    while(processing){
        res = 0;
        for(i=4;i<5;i++){
            FD_SET(fd_array[i], &readfs);
        }
        Timeout.tv_usec = 500;
        Timeout.tv_sec = 2;
        res = select(maxfd, &readfs, NULL, NULL, &Timeout);
        if(res == 0){
            printf("Timeout for Communication\n");
        }else{
            if(FD_ISSET(fd_array[0], &readfs)){
                //printf("Received something on uart 0\n");
                if(state0 != 2){
                    if(slip_recv(recv_buf0, fd_array[0], &state0, &size0) == 0){
                        recv_buf->add(size0, 0, recv_buf0);
                        state0 = 0;
                        size0 = 0;
                    }
                }
            }
            if(FD_ISSET(fd_array[1], &readfs)){
                //printf("Received something on uart 1\n");
                if(state1 != 2){
                    if(slip_recv(recv_buf1, fd_array[1], &state1, &size1) == 0){
                        recv_buf->add(size1, 0, recv_buf1);
                        state1 = 0;
                        size1 = 0;
                    }
                }
            }
            if(FD_ISSET(fd_array[2], &readfs)){
                //printf("Received something on uart 2\n");
                if(state2 != 2){
                    if(slip_recv(recv_buf2, fd_array[2], &state2, &size2) == 0){
                        recv_buf->add(size1, 0, recv_buf1);
                        state2 = 0;
                        size2 = 0;
                    }
                }
            }
            if(FD_ISSET(fd_array[3], &readfs)){
                //printf("Received something on uart 3\n");
                if(state3 != 2){
                    if(slip_recv(recv_buf3, fd_array[3], &state3, &size3) == 0){
                        recv_buf->add(size1, 0, recv_buf1);
                        state3 = 0;
                        size3 = 0;
                    }
                }
            }
            if(FD_ISSET(fd_array[4], &readfs)){
                //slip_send()
                //read()
                
                //u = 1;
                //s = write(fd_array[4], &u, sizeof(uint64_t));
                
                uint64_t val = 0;
                read(fd_array[4], &val, sizeof(uint64_t));
                printf("Pipe send something, value received: %ld \n", val);
                for(int i = 0; i < val; i++){
                    struct packet * pack;
                    if(send_buf->get(&pack) == 0){
                        slip_send((unsigned char*)pack->buffer, pack->size, pack->addr);
                    }else{
                        printf("Error SerialCon: Buffer send_buffer is empty although select indicates data in buffer\n");
                    }
                }
            }
            
        }
        //slip_send(test, 7, 0);
        //slip_send(test, 7, 1);
        //slip_send(test, 7, 2);
        //slip_send(test, 7, 3);
    }
}

int SerialCon::init_serial(int nr){
    int tty = -1;
    struct termios * temp;
    char * name = (char*)malloc(sizeof(NAME_TTY));
    switch(nr) {
        case 0:
            temp = &tio0;
            break;
        case 1:
            temp = &tio1;
            break;
        case 2:
            temp = &tio2;
            break;
        case 3:
            temp = &tio3;
            break;
    }
    snprintf(name, sizeof(NAME_TTY)+1, NAME_TTY, nr);
    printf("%s\n", name);
    memset(temp, 0, sizeof (tio0));
    temp->c_iflag = 0;
    temp->c_oflag = 0;
    temp->c_cflag = CS8 | CREAD | CLOCAL; // 8n1, see termios.h for more information
    temp->c_lflag = 0;
    temp->c_cc[VMIN] = 1;
    temp->c_cc[VTIME] = 5;
    tty = open(name, O_RDWR | O_NONBLOCK | O_NOCTTY);
    free(name);
    cfsetospeed(temp,B115200);            // 115200 baud
    cfsetispeed(temp,B115200);            // 115200 baud
    //cfsetospeed(temp, B4000000); // 115200 baud
    //cfsetispeed(temp, B4000000); // 115200 baud
    tcsetattr(tty,TCSANOW,temp);
    return tty;
}