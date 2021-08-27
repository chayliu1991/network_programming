#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
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
    if (argc != 2)
    {
        printf("usage: %s <IPaddress>\n", *argv);
        return -1;
    }

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
        errExit("socket()");

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET,
    server_addr.sin_port = htons(SERVE_PORT);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        errExit("connect()");

    char read_buf[BUF_SIZE] = {0};
    while (fgets(read_buf, BUF_SIZE, stdin) != NULL)
    {
        int i = strlen(read_buf);
        if (read_buf[i - 1] != '\n')
            read_buf[i - 1] = 0;

        if (strncmp(read_buf, "quit", strlen("quit")) == 0)
        {
            printf("client will close\n");
            close(sock_fd);
            exit(EXIT_SUCCESS);
        }

        int nwrite = write(sock_fd, read_buf, strlen(read_buf));
        if (nwrite != strlen(read_buf))
            errExit("write()");
        int nread = read(sock_fd, read_buf, BUF_SIZE - 1);
        if (nread < 0)
            errExit("read()");
        else if (nread == 0)
        {
            printf("server closed\n");
            close(sock_fd);
            exit(EXIT_SUCCESS);
        }
        read_buf[BUF_SIZE - 1] = '\0';
        printf("client received:%s\n", read_buf);
    }
}