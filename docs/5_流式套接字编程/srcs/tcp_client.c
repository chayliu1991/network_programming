#include <errno.h>
#include <netinet/in.h>
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

int tcp_client(char *address, int port)
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
        errExit("socket()");

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, address, &server_addr.sin_addr);

    if (connect(socket_fd, (SA *)&server_addr, sizeof(server_addr)) < 0)
        errExit("connect()");

    return socket_fd;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("usage:%s <Ipaddress>\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    int socket_fd = tcp_client(argv[1], SERV_PORT);
    char buf[BUF_SIZE];
    while (fgets(buf, BUF_SIZE, stdin) != NULL)
    {
        int len = strlen(buf);
        buf[len - 1] = '\0';
        int n = send(socket_fd, buf, len, 0);
        if (n < 0)
            errExit("send()");

        sleep(1);
        n = read(socket_fd, buf, BUF_SIZE - 1);
        if (n < 0)
            errExit("read()");
        else if (n == 0)
        {
            printf("peer connection quit\n");
            exit(EXIT_FAILURE);
        }
        else
            printf("client received : %s \n", buf);
    }
    exit(EXIT_SUCCESS);
}