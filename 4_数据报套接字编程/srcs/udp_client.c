#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
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

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("usage:%s <IPaddress>\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0)
        errExit("socket()");

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    struct sockaddr_in resp_addr;
    socklen_t len = sizeof(resp_addr);
    char send[BUF_SIZE], recv[BUF_SIZE + 1];

    while (fgets(send, BUF_SIZE, stdin) != NULL)
    {
        int nbytes = strlen(send);
        send[nbytes - 1] = '\0';

        int n = sendto(sock_fd, send, nbytes, 0, (CSA *)&server_addr, sizeof(server_addr));
        if (n < 0)
            errExit("sendto()");

        n = recvfrom(sock_fd, recv, BUF_SIZE, 0, (SA *)&resp_addr, &len);
        if (n < 0)
        {
            fprintf(stderr, "recvfrom failed:%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        recv[n] = '\0';
        printf("client received %s\n", recv);
    }

    exit(EXIT_SUCCESS);
}