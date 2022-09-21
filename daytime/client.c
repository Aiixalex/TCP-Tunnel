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
    char name[MAXLINE];
    char currtime[MAXLINE];
    char payload[MAXLINE];
};

struct msg_tunnel{
    char addr[MAXLINE];
    char name[MAXLINE];
    char currtime[MAXLINE];
    char payload[MAXLINE];
    char serveraddr[MAXLINE];
    char servername[MAXLINE];
    char serverport[MAXLINE];
};

int main(int argc, char **argv)
{
    int    sockfd, n, s;
    struct addrinfo hints, *result;
    // struct sockaddr_in servaddr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
    s = getaddrinfo(argv[1], argv[2], &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    // // Get the server ipaddr and service
    // char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
    // if (getnameinfo(result->ai_addr, result->ai_addrlen, hbuf, sizeof(hbuf), sbuf,
    //     sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV) == 0) {
    //     // printf("host=%s, serv=%s\n", hbuf, sbuf);
    // }

    // Get server hostname
    struct hostent* host;
    struct sockaddr_in servaddr;
    char serverip[MAXLINE];
    servaddr = *(struct sockaddr_in*)result->ai_addr;
    host = gethostbyaddr( (const void *) &servaddr.sin_addr, sizeof(struct in_addr), AF_INET);
    strcpy(serverip, inet_ntoa(servaddr.sin_addr));


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

    if (argc == 3) {
        struct message msg;
        if ( (n = read(sockfd, &msg, sizeof(msg))) > 0) {
            // recvline[n] = 0;        /* null terminate */
            if (fprintf(stdout, "Server Name: %s\n", host->h_name) == EOF) {
                printf("fprintf server name error\n");
                exit(EXIT_FAILURE);
            }
            if (fprintf(stdout, "IP Address: %s\n", serverip) == EOF) {
                printf("fprintf ipaddr error\n");
                exit(EXIT_FAILURE);
            }
            if (fprintf(stdout, "Time: %s\n", msg.currtime) == EOF) {
                printf("fprintf currtime error\n");
                exit(EXIT_FAILURE);
            }
            if (fprintf(stdout, "Who: %s", msg.payload) == EOF) {
                printf("fprintf who command error\n");
                exit(EXIT_FAILURE);
            }
        }
        if (n < 0) {
            printf("read error\n");
            exit(EXIT_FAILURE);
        }

        // // Get the hostname of the client
        // struct message ret_msg;
        // char hostbuffer[MAXLINE];
        // if (gethostname(hostbuffer, sizeof(hostbuffer))) {
        //     printf("gethostname error\n");
        //     exit(EXIT_FAILURE);
        // }

        // strcpy(ret_msg.name, hostbuffer);
        // write(sockfd, &ret_msg, sizeof(ret_msg));
    }
    else if (argc == 5) {
        struct msg_tunnel msg;
        if ( (n = read(sockfd, &msg, sizeof(msg))) > 0) {
            // recvline[n] = 0;        /* null terminate */
            if (fprintf(stdout, "Server Name: %s\n", msg.servername) == EOF) {
                printf("fprintf server name error\n");
                exit(EXIT_FAILURE);
            }
            if (fprintf(stdout, "IP Address: %s\n", serverip) == EOF) {
                printf("fprintf ipaddr error\n");
                exit(EXIT_FAILURE);
            }
            if (fprintf(stdout, "Time: %s\n", msg.currtime) == EOF) {
                printf("fprintf currtime error\n");
                exit(EXIT_FAILURE);
            }
            if (fprintf(stdout, "Who: %s", msg.payload) == EOF) {
                printf("fprintf who command error\n");
                exit(EXIT_FAILURE);
            }
        }
        if (n < 0) {
            printf("read error\n");
            exit(EXIT_FAILURE);
        }
    } else {
        printf("usage: client <IPaddress> <PortNumber> or client <TunnelIp> <TunnelPort> <ServerIp> <ServerPort>\n");
        exit(1);
    }

    exit(EXIT_SUCCESS);
}
