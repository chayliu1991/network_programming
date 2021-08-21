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

#define SERV_PORT (50005)
#define MAXLINE (4096)

static int s_count;

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("usage: %s <IPaddress>", argv[0]);
        exit(EXIT_SUCCESS);
    }

    int sleepingTime = atoi(argv[1]);

    int listenfd;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERV_PORT);

    int rt1 = bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (rt1 < 0)
    {
        fprintf(stderr, "bind() error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int rt2 = listen(listenfd, 2014);
    if (rt2 < 0)
    {
        fprintf(stderr, "listen() error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int connfd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    if ((connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_len)) < 0)
    {
        fprintf(stderr, "accept() error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    messageObject message;
    s_count = 0;

    for (;;)
    {
        int n = read(connfd, (char *)&message, sizeof(messageObject));
        if (n < 0)
        {
            fprintf(stderr, "read() error:%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        else if (n == 0)
        {
            fprintf(stderr, "client closed\n");
            exit(EXIT_FAILURE);
        }

        printf("received %d bytes\n", n);
        s_count++;

        switch (ntohl(message.type))
        {
        case MSG_TYPE1:
            printf("process  MSG_TYPE1 \n");
            break;

        case MSG_TYPE2:
            printf("process  MSG_TYPE2 \n");
            break;

        case MSG_PING:
        {
            messageObject pong_message;
            pong_message.type = MSG_PONG;
            sleep(sleepingTime);
            ssize_t rc = send(connfd, (char *)&pong_message, sizeof(pong_message), 0);
            if (rc < 0)
            {
                fprintf(stderr, "send failure\n");
                exit(EXIT_FAILURE);
            }
            break;
        }

        default:
        {
            fprintf(stderr, "unknown message type(%d)\n", ntohl(message.type));
            exit(EXIT_FAILURE);
        }
        }
    }
}