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

#define SERV_PORT 60006
#define BUF_SIZE 4096
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
        fprintf(stderr, "usage: %s <IPaddress> \n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0)
        errExit("socket()");

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    if (connect(socket_fd, (SA *)&server_addr, sizeof(server_addr)))
        errExit("connect()");

    char send_buf[BUF_SIZE], recv_buf[BUF_SIZE + 1];
    while (fgets(send, BUF_SIZE, stdin) != NULL)
    {
        int nbytes = strlen(send_buf);
        send_buf[nbytes - 1] = '\0';

        int n = send(socket_fd, send_buf, nbytes, 0);
        if (n < 0)
            errExit("send()");

        n = recv(socket_fd, recv_buf, BUF_SIZE, 0);
        if (n < 0)
            errExit("recv()");

        recv_buf[n] = '\0';
        printf("client received : %s \n", recv_buf);
    }

    exit(EXIT_SUCCESS);
}
