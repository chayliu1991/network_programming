#include <netdb.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("usage: %s <hostname> \n", *argv);
        return -1;
    }

    struct addrinfo hints, *res;
    struct sockaddr_in *addr;
    char str[1024];
    if (getaddrinfo(argv[1], "domain", NULL, &res) < 0)
    {
        printf("getaddrinfo() error\n");
        return 0;
    }

    for (; res->ai_next; res = res->ai_next)
    {
        printf("ai_falgs:%d\n", res->ai_flags);
        printf("ai_family:%d\n", res->ai_family);
        printf("ai_socktype:%d\n", res->ai_socktype);
        printf("ai_addrlen:%d\n", res->ai_addrlen);
        printf("ai_canonname:%s\n", res->ai_canonname);

        addr = (struct sockaddr_in *)res->ai_addr;
        printf("sin_family:%d\n", addr->sin_family);
        printf("sin_port:%d\n", ntohs(addr->sin_port));
        inet_ntop(addr->sin_family, &addr->sin_addr, str, sizeof(str));
        printf("sin_addr:%s\n", str);
    }

    freeaddrinfo(res);

    return 0;
}