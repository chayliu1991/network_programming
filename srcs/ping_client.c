#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

#include "message_objecte.h"

#define KEEP_ALIVE_TIME 10
#define KEEP_ALIVE_INTERVAL 3
#define KEEP_ALIVE_PROBETIMES 3

#define SERV_PORT (50005)
#define MAXLINE (4096)

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("usage: %s <IPaddress>", argv[0]);
        exit(EXIT_SUCCESS);
    }

    int socket_fd;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    socklen_t server_len = sizeof(server_addr);
    int connect_rt = connect(socket_fd, (struct sockaddr *)&server_addr, server_len);
    if (connect_rt < 0)
    {
        fprintf(stderr, "connect() error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    char recv_line[MAXLINE + 1];
    int n;

    fd_set readmask;
    fd_set allreads;

    struct timeval tv;
    int heartbeats = 0;

    tv.tv_sec = KEEP_ALIVE_TIME;
    tv.tv_usec = 0;

    messageObject messageObject;

    FD_ZERO(&allreads);
    FD_SET(0, &allreads);
    FD_SET(socket_fd, &allreads);
    for (;;)
    {
        readmask = allreads;
        int rc = select(socket_fd + 1, &readmask, NULL, NULL, &tv);
        if (rc < 0)
        {
            fprintf(stderr, "select() error:%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (rc == 0)
        {
            if (++heartbeats > KEEP_ALIVE_PROBETIMES)
            {
                fprintf(stderr, "connection dead\n");
                exit(EXIT_FAILURE);
            }
            printf("sending heartbeat #%d\n", heartbeats);
            messageObject.type = htonl(MSG_PING);
            rc = send(socket_fd, (char *)&messageObject, sizeof(messageObject), 0);
            if (rc < 0)
            {
                fprintf(stderr, "send() error:%s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            tv.tv_sec = KEEP_ALIVE_INTERVAL;
            continue;
        }
        if (FD_ISSET(socket_fd, &readmask))
        {
            n = read(socket_fd, recv_line, MAXLINE);
            if (n < 0)
            {
                fprintf(stderr, "read() error:%s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            else if (n == 0)
            {
                fprintf(stderr, "server terminated\n");
                exit(EXIT_FAILURE);
            }
            printf("received heartbeat, make heartbeats to 0 \n");
            heartbeats = 0;
            tv.tv_sec = KEEP_ALIVE_TIME;
        }
    }
}
