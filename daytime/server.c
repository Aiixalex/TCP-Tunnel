#include <netinet/in.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAXLINE     4096    /* max text line length */
#define LISTENQ     1024    /* 2nd argument to listen() */
// #define DAYTIME_PORT 3333

struct message{
    int addrlen, timelen, msglen;
    char addr[MAXLINE];
    char currtime[MAXLINE];
    char payload[MAXLINE];
};

void generate_message(struct message* msg)
{
    time_t ticks = time(NULL);
    strcpy(msg->currtime, ctime(&ticks));
    msg->timelen = strlen(msg->currtime);

}

int main(int argc, char **argv)
{
    int    listenfd, connfd, portnum;
    struct sockaddr_in servaddr;
    char   buff[MAXLINE];
    time_t ticks;

    if (argc != 2) {
        printf("usage: server <PortNumber>\n");
        exit(1);
    }

    portnum = atoi(argv[1]);
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    struct ifreq ifr;
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, "ens160", IFNAMSIZ-1);
    ioctl(listenfd, SIOCGIFADDR, &ifr);
    printf("%s\n", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(portnum); /* daytime server */

    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    listen(listenfd, LISTENQ);

    for ( ; ; ) {
        connfd = accept(listenfd, (struct sockaddr *) NULL, NULL);

        ticks = time(NULL);
        snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
        write(connfd, buff, strlen(buff));
        printf("Sending response: %s", buff);

        close(connfd);
    }
}

