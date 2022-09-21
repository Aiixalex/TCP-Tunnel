#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <netdb.h>
#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAXLINE     4096    /* max text line length */
#define LISTENQ     1024    /* 2nd argument to listen() */

struct message{
    int addrlen, timelen, msglen;
    char addr[MAXLINE];
    char currtime[MAXLINE];
    char payload[MAXLINE];
};

struct msg_tunnel {
    char serveraddr[MAXLINE];
    char serverport[MAXLINE];
};

int main(int argc, char **argv)
{
    int    listenfd, connfd, clientport;
    struct sockaddr_in tunneladdr;
    struct hostent* client;

    clientport = atoi(argv[1]);
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&tunneladdr, sizeof(tunneladdr));
    tunneladdr.sin_family = AF_INET;
    tunneladdr.sin_addr.s_addr = htonl(INADDR_ANY);
    tunneladdr.sin_port = htons(clientport);

    bind(listenfd, (struct sockaddr *) &tunneladdr, sizeof(tunneladdr));

    listen(listenfd, LISTENQ);

    struct sockaddr_in clientaddr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    char clientip[MAXLINE];
    struct msg_tunnel msg_from_tunnel;
    for ( ; ; ) {
        connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &addr_size);

        strcpy(clientip, inet_ntoa(clientaddr.sin_addr));

        client = gethostbyaddr( (const void*) &clientaddr.sin_addr, sizeof(struct in_addr), AF_INET);

        if (read(connfd, &msg_from_tunnel, sizeof(msg_from_tunnel)) > 0)
        {
            char serverip[MAXLINE];
            char serverport[MAXLINE];
            strcpy(serverip, msg_from_tunnel.serveraddr);
            strcpy(serverport, msg_from_tunnel.serverport);

            int    sockfd, s, n;
            struct addrinfo hints, *result;
            // struct sockaddr_in servaddr;

            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
            s = getaddrinfo(serverip, serverport, &hints, &result);
            if (s != 0) {
                fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }

            struct hostent* server;
            struct sockaddr_in serveraddr;
            serveraddr = *(struct sockaddr_in*)result->ai_addr;
            server = gethostbyaddr( (const void *) &serveraddr.sin_addr, sizeof(struct in_addr), AF_INET);

            if ( (sockfd = socket(result->ai_family, result->ai_socktype, 0)) < 0) {
                printf("socket error\n");
                exit(EXIT_FAILURE);
            }

            if (connect(sockfd, result->ai_addr, result->ai_addrlen) < 0) {
                printf("connect error\n");
                exit(EXIT_FAILURE);
            }

            struct message msg;
            if ( (n = read(sockfd, &msg, sizeof(msg))) > 0) {
                // recvline[n] = 0;        /* null terminate */
                if (fprintf(stdout, "%s Received request from client %s port %d destined to server %s port %s.\n", 
                            msg.currtime, client->h_name, clientport, server->h_name, serverport) == EOF) {
                    printf("fprintf server name error\n");
                    exit(EXIT_FAILURE);
                }
            }
            if (n < 0) {
                printf("read error\n");
                exit(EXIT_FAILURE);
            }

            if (write(sockfd, &msg, sizeof(msg)) < 0) {
                printf("write error\n");
                exit(EXIT_FAILURE);
            }
        }

        close(connfd);
    }

    exit(EXIT_SUCCESS);
}