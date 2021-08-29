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

#define SERV_PORT (60006)
#define BUF_SIZE (4096)

static int s_count = 0;

#define SA struct sockaddr
#define CSA const struct sockaddr

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

    int sleeping_time = atoi(argv[1]);

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
        errExit("socket()");

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERV_PORT);

    if (bind(listen_fd, (SA *)&server_addr, sizeof(server_addr)) < 0)
        errExit("bind()");

    if (listen(listen_fd, 1024) < 0)
        errExit("listen()");

    int conn_fd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    if ((conn_fd = accept(listen_fd, (SA *)&client_addr, &client_len)) < 0)
        errExit("accept()");

    messageObject message;
    for (;;)
    {
        int n = read(conn_fd, (char *)&message, sizeof(messageObject));
        if (n < 0)
        {
            if (errno != EINTR)
                continue;

            errExit("read()");
        }
        else if (n == 0)
        {
            printf("peer client quit\n");
            exit(EXIT_SUCCESS);
        }

        printf("server received %d bytes\n", n);
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
            printf("process  MSG_PING \n");
            messageObject pong_message;
            pong_message.type = MSG_PONG;
            sleep(sleeping_time);
            ssize_t n = send(conn_fd, (char *)&pong_message, sizeof(pong_message), 0);
            if (n < 0)
                errExit("send()");
            break;
        }
        default:
        {
            printf("unknown message type(%d)\n", ntohl(message.type));
            exit(EXIT_FAILURE);
        }
        }
    }
}