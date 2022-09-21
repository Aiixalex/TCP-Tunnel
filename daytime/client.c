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
#include <fcntl.h>

#define MAXLINE     4096    /* max text line length */
// #define DAYTIME_PORT 3333

struct message {
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
    if (argc < 3) {
        printf("usage: client <ServerIp> <ServerPort> or client <TunnelIp> <TunnelPort> <ServerIp> <ServerPort>\n");
        exit(EXIT_FAILURE);
    }

    int    sockfd, s, n;
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
    struct sockaddr_in hostaddr;
    char hostname[MAXLINE];
    char hostip[MAXLINE];
    hostaddr = *(struct sockaddr_in*)result->ai_addr;
    host = gethostbyaddr( (const void *) &hostaddr.sin_addr, sizeof(struct in_addr), AF_INET);
    strcpy(hostname, host->h_name);
    strcpy(hostip, inet_ntoa(hostaddr.sin_addr));

    // portnum = atoi(argv[2]);

    // bzero(&servaddr, sizeof(servaddr));
    // servaddr.sin_family = AF_INET;
    // servaddr.sin_port = htons(portnum);  /* daytime server */
    // if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
    //     printf("inet_pton error for %s\n", argv[1]);
    //     exit(1);
    // }

    if ( (sockfd = socket(result->ai_family, result->ai_socktype, 0)) < 0) {
        printf("socket error\n");
        exit(EXIT_FAILURE);
    }
    if (connect(sockfd, result->ai_addr, result->ai_addrlen) < 0) {
        printf("connect error\n");
        exit(EXIT_FAILURE);
    }

    if (argc == 3) { // connect server directly
        struct message msg;
        while ( (n = read(sockfd, &msg, sizeof(msg))) > 0 && strlen(msg.payload) == msg.msglen && strlen(msg.currtime) == msg.timelen) {
            // recvline[n] = 0;        /* null terminate */
            // printf("%ld %d %ld %d\n", strlen(msg.payload), msg.msglen, strlen(msg.currtime), msg.timelen);
            if (fprintf(stdout, "Server Name: %s\nIP Address: %s\nTime: %s\nWho: %s", 
                        host->h_name, hostip, msg.currtime, msg.payload) == EOF) {
                printf("fprintf server name error\n");
                exit(EXIT_FAILURE);
            }
        }
        if (n < 0) {
            printf("read error\n");
            exit(EXIT_FAILURE);
        }
        // for ( ; ; ) {
        //     printf("111\n");
            
        //     if ( (n = read(sockfd, &msg, sizeof(msg))) > 0 && strlen(msg.payload) == msg.msglen && strlen(msg.currtime) == msg.timelen) {
        //         // recvline[n] = 0;        /* null terminate */
        //         // printf("%ld %d %ld %d\n", strlen(msg.payload), msg.msglen, strlen(msg.currtime), msg.timelen);
        //         if (fprintf(stdout, "Server Name: %s\nIP Address: %s\nTime: %s\nWho: %s", 
        //                     host->h_name, hostip, msg.currtime, msg.payload) == EOF) {
        //             printf("fprintf server name error\n");
        //             exit(EXIT_FAILURE);
        //         }
        //         close(sockfd);
        //         break;
        //     }
        //     if (n < 0) {
        //         printf("read error\n");
        //         exit(EXIT_FAILURE);
        //     }
        //     close(sockfd);
        // }
    }
    else if (argc == 5) { // connect server through tunnel
        struct addrinfo servhints, *servresult;
        // struct sockaddr_in servaddr;

        memset(&servhints, 0, sizeof(servhints));
        servhints.ai_family = AF_INET;
        servhints.ai_socktype = SOCK_STREAM; /* Datagram socket */
        getaddrinfo(argv[3], argv[4], &servhints, &servresult);

        struct hostent* server;
        struct sockaddr_in servaddr;
        char serverip[MAXLINE];
        servaddr = *(struct sockaddr_in*)servresult->ai_addr;
        server = gethostbyaddr( (const void *) &servaddr.sin_addr, sizeof(struct in_addr), AF_INET);
        strcpy(serverip, inet_ntoa(servaddr.sin_addr));

        struct msg_tunnel msg_to_tunnel;
        strcpy(msg_to_tunnel.serveraddr, serverip);
        strcpy(msg_to_tunnel.serverport, argv[4]);
        write(sockfd, &msg_to_tunnel, sizeof(msg_to_tunnel));

        struct message recv_msg;
        // int flags = fcntl(sockfd, F_GETFL);
        // fcntl(sockfd, F_SETFL, flags & ~O_NONBLOCK);
        for ( ; ; ) {
            if ( (n = read(sockfd, &recv_msg, sizeof(recv_msg))) > 0) {
                if (fprintf(stdout, "Server Name: %s\nIP Address: %s\nTime: %s\n\nVia Tunnel: %s\nIP Address: %s\nPort Number: %s\n", 
                            server->h_name, serverip, recv_msg.currtime, hostname, hostip, argv[2]) == EOF) {
                    printf("fprintf server name error\n");
                    exit(EXIT_FAILURE);
                }
            }

            if (n < 0) {
                printf("read error\n");
                exit(EXIT_FAILURE);
            }
        }
        // if ( (n = read(sockfd, &msg, sizeof(msg))) > 0) {
        //     // recvline[n] = 0;        /* null terminate */
        //     if (fprintf(stdout, "Server Name: %s\nIP Address: %s\nTime: %s\n\nVia Tunnel: %s\nIP Address: %s\nPort Number: %s\n", 
        //                 server->h_name, serverip, msg.currtime, host->h_name, hostip, argv[2]) == EOF) {
        //         printf("fprintf server name error\n");
        //         exit(EXIT_FAILURE);
        //     }
        // }
        // if (n < 0) {
        //     printf("read error\n");
        //     exit(EXIT_FAILURE);
        // }
    } else {
        printf("usage: client <ServerIp> <ServerPort> or client <TunnelIp> <TunnelPort> <ServerIp> <ServerPort>\n");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
