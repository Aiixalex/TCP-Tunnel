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
    int    listenfd, connfd, portnum;
    struct sockaddr_in servaddr;
    struct hostent* client;

    portnum = atoi(argv[1]);
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(portnum);

    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

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
            break;
        }
    }

    close(connfd);

    printf("Server ip: %s\nServer Port: %s\n", msg_from_tunnel.serveraddr, msg_from_tunnel.serverport);


}