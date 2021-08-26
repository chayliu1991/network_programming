#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("usage: %s <IpAddress>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct in_addr addr;
    if (inet_pton(AF_INET, argv[1], &addr) <= 0)
    {
        fprintf(stderr, "inet_pton() error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("network :%d\n", addr);

    char buf[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &addr, buf, INET_ADDRSTRLEN) <= 0)
    {
        fprintf(stderr, "inet_ntop() error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("presentation:%s\n", buf);

    exit(EXIT_SUCCESS);
}