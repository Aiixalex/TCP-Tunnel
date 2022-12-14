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
#include <netdb.h>

#define MAXLINE     4096    /* max text line length */
#define LISTENQ     1024    /* 2nd argument to listen() */
// #define IPADDR_INTERFACE "ens160"
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
    snprintf(msg->currtime, sizeof(msg->currtime), "%.24s", ctime(&ticks));

    FILE* fp = popen("who", "r");
    if (fp) {
        char line[MAXLINE];
        while (fgets(line, sizeof(line), fp)) {
            strcat(msg->payload, line);
        }
        pclose(fp);
    }
    msg->timelen = strlen(msg->currtime);
    msg->msglen = strlen(msg->payload);
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("usage: server <PortNumber>\n");
        exit(1);
    }

    int    listenfd, connfd, portnum;
    struct sockaddr_in servaddr;
    struct hostent* host;

    portnum = atoi(argv[1]);
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(portnum);

    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    struct message msg;
    generate_message(&msg);

    listen(listenfd, LISTENQ);

    struct sockaddr_in clientaddr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    char clientip[MAXLINE];

    for ( ; ; ) {
        connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &addr_size);

        // ticks = time(NULL);
        // snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
        write(connfd, &msg, sizeof(msg));

        // getpeername(connfd, (struct sockaddr *)&clientaddr, &addr_size);
        strcpy(clientip, inet_ntoa(clientaddr.sin_addr));

        host = gethostbyaddr( (const void*) &clientaddr.sin_addr, sizeof(struct in_addr), AF_INET);
        // if (read(connfd, &recv_msg, sizeof(recv_msg)) > 0)
        // {
        //     printf("Server Name: %s\n", recv_msg.name);
        //     printf("Ip Address: %s\n", clientip);
        // }

        printf("Client Name: %s\nIp Address: %s\n", host->h_name, clientip);

        close(connfd);
    }
}
