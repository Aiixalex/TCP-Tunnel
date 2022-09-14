#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAXLINE     4096    /* max text line length */
// #define DAYTIME_PORT 3333

struct message{
    int addrlen, timelen, msglen;
    char addr[MAXLINE];
    char currtime[MAXLINE];
    char payload[MAXLINE];
};

int main(int argc, char **argv)
{
    int    sockfd, n, s;
    // char   recvline[MAXLINE + 1];
    struct addrinfo hints, *result;
    struct message msg;
    // struct sockaddr_in servaddr;

    if (argc != 3) {
        printf("usage: client <IPaddress> <PortNumber>\n");
        exit(1);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
    s = getaddrinfo(argv[1], argv[2], &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }


    if ( (sockfd = socket(result->ai_family, result->ai_socktype, 0)) < 0) {
        printf("socket error\n");
        exit(EXIT_FAILURE);
    }

    // portnum = atoi(argv[2]);

    // bzero(&servaddr, sizeof(servaddr));
    // servaddr.sin_family = AF_INET;
    // servaddr.sin_port = htons(portnum);  /* daytime server */
    // if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
    //     printf("inet_pton error for %s\n", argv[1]);
    //     exit(1);
    // }

    if (connect(sockfd, result->ai_addr, result->ai_addrlen) < 0) {
        printf("connect error\n");
        exit(EXIT_FAILURE);
    }

    while ( (n = read(sockfd, &msg, sizeof(msg))) > 0) {
        // recvline[n] = 0;        /* null terminate */
        fputs(msg.addr, stdout);
        fputs(msg.currtime, stdout);
        fputs(msg.payload, stdout);
        // if (fputs(recvline, stdout) == EOF) {
        //     printf("fputs error\n");
        //     exit(EXIT_FAILURE);
        // }
    }
    if (n < 0) {
        printf("read error\n");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
