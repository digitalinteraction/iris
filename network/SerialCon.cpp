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
#include <time.h>
#include <stdlib.h>
//#include "socket/socket.c"

SerialCon::SerialCon(Packetbuffer *sendbuf, Packetbuffer *recvbuf, uint8_t deb) {
    //if (deb == 1) {
        fd_array[0] = init_serial(0);
        fd_array[1] = init_serial(1);
        fd_array[2] = init_serial(2);
        fd_array[3] = init_serial(3);
    //} /*else {
        //fd_array[0] = init_serial(1);
       // fd_array[1] = init_serial(3);
        //fd_array[2] = init_serial(5);
        //fd_array[3] = init_serial(7);
    //}*/



    
    slen=sizeof(server_addr);

    if ((fd_array[4]=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1){
        printf("Error opening udp socket\n");
    }

    memset((char *) &server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DEBUG_PORT);
    if (inet_aton(DEBUG_SERVER, &server_addr.sin_addr)==0) {
     printf("inet_aton() failed\n");
    }



    
    
    send_buf = sendbuf;
    recv_buf = recvbuf;
    fd_array[5] = send_buf->signalfd;
    if (fd_array[5] == -1) {
        printf("Error in eventfd creation\n");
    }
    //fd_array[5] = pipe_array[1];

    int i, max = 0;
    for (i = 0; i < 6; i++) {
        if (fd_array[i] >= 0) {
            if (fd_array[i] > max) {
                max = fd_array[i];
            }
        } else {
            printf("File Descriptor %d could not be initialized; Error Code %d\n", i, fd_array[i]);
        }
    }
    maxfd = max + 1;
    memset(recv_buf0, 0, SIZE_LIMIT);
    state0 = 0;
    memset(recv_buf1, 0, SIZE_LIMIT);
    state1 = 0;
    memset(recv_buf2, 0, SIZE_LIMIT);
    state2 = 0;
    memset(recv_buf3, 0, SIZE_LIMIT);
    state3 = 0;
    
    srand(time(NULL));
}

SerialCon::~SerialCon() {
}

int SerialCon::slip_send(unsigned char *p, uint16_t len, uint32_t nr) {
    //printf("SerialCon:: sending packet out of nr %d fd %d\n", nr, fd_array[nr]);
    if (len <= 0 || p == 0) {
        return -1;
    }
    
    //enable udp transfer based on nr
    if(nr > 256){
        sendto(fd_array[4], p, len, 0, (struct sockaddr *)&server_addr, slen);
    }
    
    
    
    ssize_t ret = 0;

    write(fd_array[nr], &end, 1);

    while (len--) {

        switch (*p) {
            case END:
                //send_char(ESC);
                //send_char(ESC_END);
                ret = write(fd_array[nr], &esc, 1);
                ret += write(fd_array[nr], &esc_end, 1);
                if (ret != 2) {
                    return -1;
                }
                break;
            case ESC:
                //send_char(ESC);
                //send_char(ESC_ESC);
                ret = write(fd_array[nr], &esc, 1);
                ret += write(fd_array[nr], &esc_esc, 1);
                if (ret != 2) {
                    return -1;
                }
                break;
            default:
                //send_char(*p);
                ret = write(fd_array[nr], p, 1);
                if (ret != 1) {
                    return -1;
                }
        }
        p++;
    }
    //send_char(END);
    ret = write(fd_array[nr], &end, 1);
    if (ret != 1) {
        return -1;
    }
    return 0;
}

int SerialCon::slip_recv(unsigned char *p, unsigned char c, int *state, int*size) {
    switch (c) {
        case END:
            return 0;
            break;
        case ESC:
            if (*state == 0) {
                *state = 1;
            } else {
                //printf("Error SerialCon: double ESC\n");
            }
            break;
        case ESC_END:
            if (*state == 1) {
                c = END;
                *state = 0;
            }
        case ESC_ESC:
            if (*state == 1) {
                c = ESC;
                *state = 0;
            }
    default:
            if ((*size) < SIZE_LIMIT) {
                p[(*size)++] = c;
            } else {
                *state = 0;
                *size = 0;
            }
    }
    return 1;
}

void SerialCon::slip_run() {
    int i;
    struct timeval Timeout;
    Timeout.tv_usec = 500;
    Timeout.tv_sec = 0;
    int res;
    //char test[] = "testing";


    while (processing) {
        FD_ZERO(&readfs);
        FD_ZERO(&writefs);
        FD_ZERO(&exceptfs);
        res = 0;

        for (i = 0; i < 6; i++) {
            FD_SET(fd_array[i], &readfs);
        }
        Timeout.tv_usec = 5000;
        Timeout.tv_sec = 0;
        res = select(maxfd, &readfs, &writefs, &exceptfs, &Timeout);
        if (res <= 0) {
        } else {

            if (FD_ISSET(fd_array[0], &readfs)) {
                unsigned char c;
                while(read(fd_array[0], &c, 1) > 0){
                    if (slip_recv(recv_buf0, c, &state0, &size0) == 0) {
                        if (size0 > 0) {
 
                            int ret = recv_buf->add(size0, 0, recv_buf0);
                            if (ret != 0) {
                               // printf("Error SerialCon: inserting buffer in Packetbuffer not successful\n");
                            }
                        }
                        state0 = 0;
                        size0 = 0;
                    }
                }
            }
            if (FD_ISSET(fd_array[1], &readfs)) {
                unsigned char c;
                while(read(fd_array[1], &c, 1) > 0){
                    if (slip_recv(recv_buf1, c, &state1, &size1) == 0) {
                        if (size1 > 0) {
 
                            int ret = recv_buf->add(size1, 1, recv_buf1);
                            if (ret != 0) {
                                //printf("Error SerialCon: inserting buffer in Packetbuffer not successful\n");
                            }
                        }
                        state1 = 0;
                        size1 = 0;
                    }
                }
            }
            if (FD_ISSET(fd_array[2], &readfs)) {
                unsigned char c;
                while(read(fd_array[2], &c, 1) > 0){
                    if (slip_recv(recv_buf2, c, &state2, &size2) == 0) {
                        if (size2 > 0) {
 
                            int ret = recv_buf->add(size2, 2, recv_buf2);
                            if (ret != 0) {
                                //printf("Error SerialCon: inserting buffer in Packetbuffer not successful\n");
                            }
                        }
                        state2 = 0;
                        size2 = 0;
                    }
                }
            }
            if (FD_ISSET(fd_array[3], &readfs)) {
                unsigned char c;
                while(read(fd_array[3], &c, 1) > 0){
                    if (slip_recv(recv_buf3, c, &state3, &size3) == 0) {
                        if (size3 > 0) {
 
                            int ret = recv_buf->add(size3, 3, recv_buf3);
                            if (ret != 0) {
                                //printf("Error SerialCon: inserting buffer in Packetbuffer not successful\n");
                            }
                        }
                        state3 = 0;
                        size3 = 0;
                    }
                }
            }
            if (FD_ISSET(fd_array[5], &readfs)) {

                uint64_t val = 0;
                read(fd_array[5], &val, sizeof (uint64_t));
                for (int i = 0; i < val; i++) {
                    struct packet * pack;
                    if (send_buf->get(&pack) == 0) {
                        if (pack->size > 0) {
                            /*
                            int r = rand() % 10000;
                            if (r == 5423) {
                                //printf("error inserted\n");
                                ((unsigned char*)pack->buffer)[0] = 0xFF;
                            }*/
                            int ret = slip_send((unsigned char*) pack->buffer, pack->size, pack->addr);
                            if (ret != 0) {
                                //printf("Error SerialCon: sending packet did not work\n");
                            }
                        }
                        free(pack->buffer);
                        free(pack);

                    } else {
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

int SerialCon::init_serial(int nr) {
    int tty = -1;
    struct termios * temp;
    char * name = (char*) malloc(sizeof (NAME_TTY));
    switch (nr) {
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
    snprintf(name, sizeof (NAME_TTY) + 1, NAME_TTY, nr);
    //printf("%s\n", name);
    memset(temp, 0, sizeof (tio0));
    temp->c_iflag = 0;
    temp->c_oflag = 0;
    temp->c_cflag = (CS8 | CREAD | CLOCAL)&~(PARENB|CSTOPB|CRTSCTS); // 8n1, see termios.h for more information
    temp->c_lflag &= ~(ICANON | ECHO | ISIG);
    temp->c_cc[VMIN] = 1;
    temp->c_cc[VTIME] = 5;
    tty = open(name, O_RDWR | O_NONBLOCK | O_NOCTTY);
    free(name);
    //cfsetospeed(temp, B9600); // 115200 baud
    //cfsetispeed(temp, B9600); // 115200 baud
    cfsetospeed(temp, B1000000); // 115200 baud
    cfsetispeed(temp, B1000000); // 115200 baud
    tcsetattr(tty, TCSANOW, temp);
    //sleep(2); //required to make flush work, for some reason
    //sleep(2);
    tcflush(tty, TCIOFLUSH);
    //printf("Serial port %d has been initialized\n", tty);
    return tty;
}
