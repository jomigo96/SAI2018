#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>

struct position_gps{

    double latitude;
    double longitude;
    double altitude;
};


pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
int ready = 0;
struct position_gps position;
//uint32_t last_timestamp = 0;

void* data_reception(void* ptr){


    // Setup UDP socket
    struct sockaddr_in serveraddr, clientaddr;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0){
        fprintf(stderr, "Error starting socket\n");
        exit(1);
    }

    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
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
    while(1){
    
        byte_count = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&clientaddr, (socklen_t*)&clientlen);
        if(byte_count){
        
            pthread_mutex_lock(&m);
            memcpy(&position, buf, sizeof(position));
            ready = 1;
            pthread_cond_broadcast(&cv);
            pthread_mutex_unlock(&m);
        }
    
    }

    pthread_mutex_lock(&m);
    position.altitude = 1000;
    position.latitude = 1;
    position.longitude = 65;
    ready = 1;
    pthread_cond_broadcast(&cv);
    pthread_mutex_unlock(&m);
    pthread_exit(NULL);

}

int main(int argc, char **argv){


    if(argc != 2){
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    uint32_t port = atoi(argv[1]);

    pthread_t tid;
    pthread_create(&tid, NULL, data_reception, &port);


    struct sigaction action;
    sigaction(SIGINT, NULL, &action);
    SDL_Init(SDL_INIT_EVERYTHING);
    sigaction(SIGINT, &action, NULL); //SDL overwrites the CTRL-C signal

    SDL_Window* window;
    window =  SDL_CreateWindow("ILS display", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 480, 360, 0); 
    if(window == NULL){
        fprintf(stderr, "SDL error\n");
        exit(-1);
    }

    char text[1024];
 
    while(1){
        // Passive wait
        pthread_mutex_lock(&m);
        while(!ready){
            pthread_cond_wait(&cv, &m);
        }
        ready = 0;
        sprintf(text, "%lf;%lf;%lf", position.latitude, position.longitude, position.altitude);
        // Or copy to a local buffer
        pthread_mutex_unlock(&m);
        // Write out
        fprintf(stdout, "%s\n", text);

    }

    //void* result;
    //pthread_join(tid, &result);

    return 0;
}
