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

#define SERV_PORT (60006)
#define BUF_SIZE (4096)
#define SA struct sockaddr
#define CSA const struct sockaddr

void errExit(const char *errmsg)
{
    printf("%s:%s\n", errmsg, strerror(errno));
    exit(EXIT_FAILURE);
}

int tcp_server(int port)
{
    signal(SIGPIPE, SIG_IGN);

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
        errExit("socket()");

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if (bind(listen_fd, (SA *)&server_addr, sizeof(server_addr)) < 0)
        errExit("bind()");

    if (listen(listen_fd, 1024) < 0)
        errExit("listen()");

    int conn_fd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    if ((conn_fd = accept(listen_fd, (SA *)&client_addr, &client_len)) < 0)
        errExit("accept()");

    return conn_fd;
}

int main(int argc, char **argv)
{
    int connfd = tcp_server(SERV_PORT);
    char buf[BUF_SIZE];
    for (;;)
    {
        int n = read(connfd, buf, BUF_SIZE - 1);
        if (n < 0)
            errExit("read()");
        else if (n == 0)
        {
            printf("peer client quit\n");
            exit(EXIT_SUCCESS);
        }

        sleep(1);
        buf[n] = '\0';
        printf("sever received : %s\n", buf);
        if (send(connfd, buf, n, 0) != n)
            errExit("send()");
        printf("send bytes: %d \n", n);
    }

    exit(0);
}