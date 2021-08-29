#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "message_objecte.h"

#define SA struct sockaddr
#define CSA const struct sockaddr

#define KEEP_ALIVE_TIME 3
#define KEEP_ALIVE_INTERVAL 3
#define KEEP_ALIVE_PROBETIMES 3

#define SERV_PORT (60006)
#define BUF_SIZE (4096)

void errExit(const char *errmsg)
{
    printf("%s:%s\n", errmsg, strerror(errno));
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("usage: %s <IPaddress>", argv[0]);
        exit(EXIT_SUCCESS);
    }

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
        errExit("socket()");

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        errExit("connect()");

    fd_set readmask;
    fd_set allreads;
    struct timeval tv;
    tv.tv_sec = KEEP_ALIVE_TIME;
    tv.tv_usec = 0;

    FD_ZERO(&allreads);
    FD_SET(0, &allreads);
    FD_SET(socket_fd, &allreads);

    char recv_line[BUF_SIZE + 1];
    int n;
    int heartbeats = 0;
    messageObject messageObject;
    for (;;)
    {
        readmask = allreads;
        int n = select(socket_fd + 1, &readmask, NULL, NULL, &tv);
        if (n < 0)
            errExit("select()");
        if (n == 0)
        {
            if (++heartbeats > KEEP_ALIVE_PROBETIMES)
            {
                printf("connection dead\n");
                exit(EXIT_FAILURE);
            }

            printf("sending heartbeat #%d\n", heartbeats);
            messageObject.type = htonl(MSG_PING);
            n = send(socket_fd, (char *)&messageObject, sizeof(messageObject), 0);
            if (n < 0)
                errExit("send()");
            tv.tv_sec = KEEP_ALIVE_INTERVAL; //@ 重置
            continue;
        }
        if (FD_ISSET(socket_fd, &readmask))
        {
            n = read(socket_fd, recv_line, BUF_SIZE);
            if (n < 0)
                errExit("read()");
            else if (n == 0)
            {
                printf("peer server quit\n");
                exit(EXIT_FAILURE);
            }

            printf("received heartbeat, make heartbeats to 0 \n");
            heartbeats = 0;
            tv.tv_sec = KEEP_ALIVE_TIME;
        }
    }
}