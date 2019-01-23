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

int main(int argc, char **argv){

    if(argc != 3){
        fprintf(stderr, "usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }
    uint32_t port = atoi(argv[2]);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, argv[1], &(server_addr.sin_addr));
    bzero(&(server_addr.sin_zero), 8);

    printf("Sends a message when you hit enter.");

    float value=0;
    char buf[200];
    int n;

    while(1){
        while(getchar()!='\n')
            ;
        memcpy(buf, &value, sizeof(value));
        memcpy(buf+sizeof(value), &value, sizeof(value));
        memcpy(buf+2*sizeof(value), &value, sizeof(value));
        memcpy(buf+3*sizeof(value), &value, sizeof(value));
        value += 1.0;
        n=sendto(sockfd, buf, 4*sizeof(value), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
        printf("Sent %d bytes\n", n);
    }
}
