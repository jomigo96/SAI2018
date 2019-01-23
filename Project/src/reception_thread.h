/**
 * @file reception_thread.h
 * @author João Gonçalves, Tiago Oliveira, Carolina Serra
 * @date 23 Jan 2019
 * 
 * @brief Data reception handler
 *
 * Provides a thread to listen on a UDP socket for incomming data, and store it appropriately.
 * Coded inline.
 * */

#ifndef RECEPTION_H
#define RECEPTION_H

#define DEBUG
#define BIGENDIAN

#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include "ils.h"

// Globals
extern pthread_mutex_t m;
extern int ready;
extern struct position_gps position;

// Reverses endianess of a floating point value
/**
 *
 * @brief Reverses endianess of a floating point value
 *
 * Network transmission is by defauld big-endian, while most machines are little-endian.
 *
 * @param value floating point value (4 bytes)
 * @return same floating point value with reversed endianess
 * */
float float_swap(float value){
    union v {
        float       f;
        unsigned int    i;
    };
     
    union v val;
     
    val.f = value;
    val.i = htonl(val.i);
                   
    return val.f;
}

/**
 * @brief Data reception thread
 *
 * Opens a UDP socket to listen for incomming data and store it on a global data structure.
 * Converts the endianess of values based on the macro BIGENDIAN being defined or not.
 * Calls exit() in case there is an error opening or binding the 
 * socket (common cause is the port already being in use).
 * Runs indefinately.
 * 
 * @param ptr pointer to an uint32_t port number
 * @return Not used
 * */
void* reception_thread(void* ptr){

    // Setup UDP socket
    struct sockaddr_in serveraddr, clientaddr;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0){
        fprintf(stderr, "Error starting socket\n");
        exit(1);
    }

    // Port Reusing
    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval,
                                                                   sizeof(int));

    // Configure port
    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(*(uint32_t*)ptr);

    // Bind to given port
    if(bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) != 0){
        perror("bind");
        exit(1);
    }

    char buf[1024];
    size_t byte_count;

    int clientlen = sizeof(clientaddr);

    // Sender's API places each variable on a given position
    const int latitude_idx = 0;
    const int longitude_idx = 4;
    const int altitude_idx = 8;

    while(1){

        byte_count = recvfrom(sockfd, buf, sizeof(buf), 0,
                         (struct sockaddr*)&clientaddr, (socklen_t*)&clientlen);
        if(byte_count>0){

            buf[byte_count]=0;

            pthread_mutex_lock(&m);
#ifdef BIGENDIAN
			position.latitude = float_swap(*(float*)(buf+latitude_idx))*DEG_to_RAD;
			position.longitude = float_swap(*(float*)(buf+longitude_idx))*DEG_to_RAD;
			position.altitude = float_swap(*(float*)(buf+altitude_idx));
#else
            position.latitude =  *(float*)(buf+latitude_idx)*DEG_to_RAD;
            position.longitude = *(float*)(buf+longitude_idx)*DEG_to_RAD;
            position.altitude =  *(float*)(buf+altitude_idx);
#endif
            ready = 1;
            pthread_mutex_unlock(&m);
            
#ifdef DEBUG
            printf("Received %ld bytes: ", byte_count);
            printf("\n");

            printf("Lat:%f Lon:%f Hei:%f\n", position.latitude*RAD_to_DEG,
                                                    position.longitude*RAD_to_DEG,
                                                    position.altitude);
#endif

        }
    }
}

#endif //RECEPTION_H
