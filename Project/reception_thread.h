#ifndef RECEPTION_H
#define RECEPTION_H

#define DEBUG

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
    const int heading_idx = 12;


    while(1){

        byte_count = recvfrom(sockfd, buf, sizeof(buf), 0,
                         (struct sockaddr*)&clientaddr, (socklen_t*)&clientlen);
        if(byte_count){
            pthread_mutex_lock(&m);
            position.latitude = *(float*)(buf+latitude_idx);
            position.longitude = *(float*)(buf+longitude_idx);
            position.altitude = *(float*)(buf+altitude_idx);
            position.heading = *(float*)(buf+heading_idx);
            ready = 1;
            pthread_mutex_unlock(&m);

            #ifdef DEBUG
			printf("Received %ld bytes: ", byte_count);
            printf("Lat:%f Lon:%f Hei:%f Hea:%f\n", position.latitude,
                                                    position.longitude,
                                                    position.altitude,
                                                    position.heading);
            #endif //DEBUG
        }
    }
}



#endif //RECEPTION_H
