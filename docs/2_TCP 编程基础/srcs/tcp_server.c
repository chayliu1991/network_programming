#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SERVE_PORT (8088)
#define BUF_SIZE (4096)

void errExit(const char *caller)
{
    fprintf(stderr, "%s  error: %s\n", caller, strerror(errno));
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
        errExit("socket()");

    struct sockaddr_in server_addr, client_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET,
    server_addr.sin_port = htons(SERVE_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        errExit("bind()");

    if (listen(listen_fd, 1024) < 0)
        errExit("listen()");

    socklen_t client_len = sizeof(client_addr);
    int conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
    if (conn_fd < 0)
        errExit("listen()");

    char read_buf[BUF_SIZE] = {0};
    while (1)
    {
        bzero(read_buf, sizeof(read_buf));
        int nread = read(conn_fd, read_buf, BUF_SIZE - 1);
        if (nread < 0)
            errExit("read()");
        else if (nread == 0)
        {
            printf("client close\n");
            close(listen_fd);
            exit(EXIT_SUCCESS);
        }
        read_buf[BUF_SIZE - 1] = '\0';
        printf("Server Received:%s\n", read_buf);
        if (write(conn_fd, read_buf, strlen(read_buf)) != strlen(read_buf))
            errExit("write()");
    }
}