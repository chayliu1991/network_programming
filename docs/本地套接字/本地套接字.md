# 本地套接字概述

本地套接字是一种特殊类型的套接字，和TCP/UDP套接字不同。TCP/UDP即使在本地地址通信，也要走系统网络协议栈，而本地套接字，严格意义上说提供了一种单主机跨进程间调用的手段，减少了协议栈实现的复杂度，效率比TCP/UDP套接字都要高许多。

# 本地字节流套接字

## 服务端

```
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#define MAX_LINE (4096)
#define BUF_SIZE (4096)

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("usage:%s <LocalPath>\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    char *local_path = argv[1];
    if (remove(local_path) == -1 && errno != ENOENT) //@ 移除所有的既有文件
    {
        fprintf(stderr, "remove() error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int listenfd, connfd;
    socklen_t clilen;
    int res = 0;
    struct sockaddr_un cliaddr, servaddr;

    listenfd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (listenfd < 0)
    {
        fprintf(stderr, "socket() error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    unlink(local_path);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path, local_path);
    res = bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (res < 0)
    {
        fprintf(stderr, "bind() error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    res = listen(listenfd, 1024);
    if (res < 0)
    {
        fprintf(stderr, "listen() error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    clilen = sizeof(cliaddr);
    connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
    if (connfd < 0)
    {
        fprintf(stderr, "accept() error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    char buf[BUF_SIZE];

    while (1)
    {
        bzero(buf, sizeof(buf));

        if (read(connfd, buf, BUF_SIZE) == 0)
        {
            printf("client quit\n");
            break;
        }

        printf("Received : %s\n", buf);

        char send_line[MAX_LINE];
        snprintf(send_line, sizeof(send_line), "Hi,%s", buf);

        int nbytes = sizeof(send_line);
        if (write(connfd, send_line, nbytes) != nbytes)
        {
            fprintf(stderr, "accept() error:%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    close(listenfd);
    close(connfd);
    exit(EXIT_SUCCESS);
}
```

## 客户端

```
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#define MAX_LINE (4096)

int main(int argc, char **argv)
{
    if (argc < 2 || strcmp(argv[1], "--help") == 0)
    {
        fprintf(stderr, "usage: %s <LocalPath> \n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sockfd;
    struct sockaddr_un servaddr;
    int res;

    sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        fprintf(stderr, "socket() error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path, argv[1]);

    res = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (res < 0)
    {
        fprintf(stderr, "connect() error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    char send_line[MAX_LINE];
    bzero(send_line, sizeof(send_line));
    char resp_line[MAX_LINE];

    while (fgets(send_line, MAX_LINE, stdin) != NULL)
    {
        int nbytes = sizeof(send_line);
        if (write(sockfd, send_line, nbytes) != nbytes)
        {
            fprintf(stderr, "write() error:%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (read(sockfd, resp_line, MAX_LINE) == 0)
        {
            fprintf(stderr, "read() error:%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        fputs(resp_line, stdout);
    }

    exit(EXIT_SUCCESS);
}
```

# 本地数据报套接字

## 服务端

```
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#define MAX_LINE (4096)
#define BUF_SIZE (4096)

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("usage:%s <LocalPath>\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    char *local_path = argv[1];
    if (remove(local_path) == -1 && errno != ENOENT) //@ 移除所有的既有文件
    {
        fprintf(stderr, "remove() error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int socket_fd;
    socklen_t clilen;
    int res = 0;
    struct sockaddr_un cliaddr, servaddr;

    socket_fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (socket_fd < 0)
    {
        fprintf(stderr, "socket() error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    unlink(local_path);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path, local_path);
    res = bind(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (res < 0)
    {
        fprintf(stderr, "bind() error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    char buf[BUF_SIZE];
    clilen = sizeof(cliaddr);

    while (1)
    {
        bzero(buf, sizeof(buf));
        if (recvfrom(socket_fd, buf, BUF_SIZE, 0, &cliaddr, &clilen) == 0)
        {
            printf("client quit\n");
            break;
        }

        printf("Received : %s\n", buf);

        char send_line[MAX_LINE];
        bzero(send_line, sizeof(send_line));
        snprintf(send_line, sizeof(send_line), "Hi,%s", buf);

        int nbytes = sizeof(send_line);
        if (sendto(socket_fd, send_line, nbytes, 0, &cliaddr, &clilen) != nbytes)
        {
            fprintf(stderr, "accept() error:%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    close(socket_fd);
    exit(EXIT_SUCCESS);
}
```

## 客户端

```
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#define MAX_LINE (4096)
#define BUF_SIZE (4096)

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("usage:%s <LocalPath>\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    char *local_path = argv[1];
    int socket_fd;
    socklen_t clilen;
    int res = 0;
    struct sockaddr_un cliaddr, servaddr;

    socket_fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (socket_fd < 0)
    {
        fprintf(stderr, "socket() error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    bzero(&cliaddr, sizeof(cliaddr));
    cliaddr.sun_family = AF_LOCAL;
    strcpy(cliaddr.sun_path, tmpnam(NULL));
    res = bind(socket_fd, (struct sockaddr *)&cliaddr, sizeof(cliaddr));
    if (res < 0)
    {
        fprintf(stderr, "bind() error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path, local_path);

    char send_line[MAX_LINE];
    bzero(send_line, sizeof(send_line));
    char resp_line[MAX_LINE];

    while (fgets(send_line, MAX_LINE, stdin) != NULL)
    {
        int i = strlen(send_line);
        if (send_line[i - 1] != '\n')
            send_line[i - 1] = 0;

        size_t nbytes = strlen(send_line);
        printf("now sending %s\n", send_line);
        if (sendto(socket_fd, send_line, nbytes, 0, &servaddr, sizeof(servaddr)) != nbytes)
        {
            fprintf(stderr, "accept() error:%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        int n = recvfrom(socket_fd, resp_line, MAX_LINE, 0, NULL, NULL);
        resp_line[n] = 0;

        fputs(resp_line, stdout);
        fputs("\n", stdout);
    }

    close(socket_fd);
    exit(EXIT_SUCCESS);
}
```



