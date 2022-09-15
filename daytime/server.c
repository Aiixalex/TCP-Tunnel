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
#define IPADDR_INTERFACE "ens160"
// #define DAYTIME_PORT 3333

struct message{
    int addrlen, timelen, msglen;
    char addr[MAXLINE];
    char currtime[MAXLINE];
    char payload[MAXLINE];
};

void generate_message(char* hostname, struct message* msg)
{
    strcpy(msg->addr, hostname);
    msg->addrlen = strlen(msg->addr);

    time_t ticks = time(NULL);
    snprintf(msg->currtime, sizeof(msg->currtime), "%.24s", ctime(&ticks));
    // printf("%s", msg->currtime);
    msg->timelen = strlen(msg->currtime);

    FILE* fp = popen("who", "r");
    if (fp) {
        if (fgets(msg->payload, sizeof(msg->payload), fp)) {
            // printf("%s", msg->payload);
        }
        pclose(fp);
    }
    msg->msglen = strlen(msg->payload) + msg->addrlen + msg->timelen;
}

int main(int argc, char **argv)
{
    int    listenfd, connfd, portnum;
    struct sockaddr_in servaddr;

    if (argc != 2) {
        printf("usage: server <PortNumber>\n");
        exit(1);
    }

    portnum = atoi(argv[1]);
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    // Get the hostname of the server
    char hostbuffer[MAXLINE];
    if (gethostname(hostbuffer, sizeof(hostbuffer))) {
        printf("gethostname error\n");
        exit(EXIT_FAILURE);
    }

    // // Get the IP address of the interface
    // struct ifreq ifr;
    // ifr.ifr_addr.sa_family = AF_INET;
    // strncpy(ifr.ifr_name, IPADDR_INTERFACE, IFNAMSIZ-1);
    // ioctl(listenfd, SIOCGIFADDR, &ifr);
    // char* ipaddr = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
    // // printf("%s\n", ipaddr);

    struct message msg;
    generate_message(hostbuffer, &msg);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(portnum); /* daytime server */

    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    listen(listenfd, LISTENQ);

    struct message recv_msg;
    struct sockaddr_in accept_addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    char clientip[MAXLINE];
    for ( ; ; ) {
        connfd = accept(listenfd, (struct sockaddr *) NULL, NULL);

        // ticks = time(NULL);
        // snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
        write(connfd, &msg, sizeof(msg));

        getpeername(connfd, (struct sockaddr *)&accept_addr, &addr_size);
        strcpy(clientip, inet_ntoa(accept_addr.sin_addr)); 
        if (read(connfd, &recv_msg, sizeof(recv_msg)) > 0)
        {
            printf("Server Name: %s\n", recv_msg.addr);
            printf("Ip Address: %s\n", clientip);
        }

        close(connfd);
    }
}

