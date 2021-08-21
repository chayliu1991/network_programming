# 数据报套接字编程模型

![](./img/udp.png)

# 交换数据报

```
#include <sys/types.h>
#include <sys/socket.h>

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,const struct sockaddr *dest_addr, socklen_t addrlen);

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,struct sockaddr *src_addr, socklen_t *addrlen);
```

- `flags` 是和 I/O 相关的参数，一般设置为 0
- `src_addr` 和 `addrlen` 用来获取或指定与之通信的对等 socket 的地址，如果不关心可以设置为 `NULL` 和 0
- `dest_addr` 和 `addrlen` 指定数据报发送到的 socket

对于 `recvfrom()` 不管 `len` 的参数值是多少， `recvfrom()` 只会从一个数据报 socket 中读取一条消息，如果消息的长度超过了 `length`，那么消息将会静默被截断为 `len` 字节。

对于 `sendto()` 可以发送长度为 0 的数据报，但不是所有的 UNIX 实现都是如此。

# 服务端-客户端

## 服务端

```
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define SERV_PORT 43211
#define MAXLINE 4096

static int s_count = 0;

static void recvfrom_int(int signo)
{
    printf("\n received %d datagrams \n", s_count);
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    int sockfd;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    socklen_t clilen;
    char message[MAXLINE];
    s_count = 0;

    signal(SIGINT, recvfrom_int);

    struct sockaddr_in client_addr;
    clilen = sizeof(client_addr);
    for (;;)
    {
        printf("server waiting...\n");
        int n = recvfrom(sockfd, message, MAXLINE, 0, (struct sockaddr *)&client_addr, &clilen);
        message[n] = 0;
        printf("received %d bytes:%s\n", n, message);

        char send_line[MAXLINE];
        snprintf(send_line, sizeof(message), "Hi,%s", message);

        sendto(sockfd, send_line, strlen(send_line), 0, (struct sockaddr *)&client_addr, clilen);

        s_count++;
    }
}
```

## 客户端

```
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#define SERV_PORT 43211
#define MAXLINE 4096

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("usage:%s <Ipaddress>\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    int sockfd;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    socklen_t serverlen = sizeof(server_addr);
    struct sockaddr *resp_addr;
    resp_addr = malloc(serverlen);

    char send_line[MAXLINE], resp_line[MAXLINE + 1];
    socklen_t len;
    int n;

    while (fgets(send_line, MAXLINE, stdin) != NULL)
    {
        int i = strlen(send_line);
        if (send_line[i - 1] != '\n')
            send_line[i - 1] = 0;

        printf("now sending %s\n", send_line);

        size_t rt = sendto(sockfd, send_line, strlen(send_line), 0, (struct sockaddr *)&server_addr, serverlen);
        if (rt < 0)
        {
            fprintf(stderr, "sendto failed:%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        printf("send bytes:%zu\n", rt);

        len = 0;
        n = recvfrom(sockfd, resp_line, MAXLINE, 0, resp_addr, &len);
        if (n < 0)
        {
            fprintf(stderr, "recvfrom failed:%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        resp_line[n] = 0;
        fputs("response:", stdout);
        fputs(resp_line, stdout);
        fputs("\n", stdout);
    }

    exit(EXIT_SUCCESS);
}
```

如果不开启服务端，TCP 客户端的 `connect()` 会直接返回 “Connection refused” 报错信息。而在 UDP 程序里，程序会一直阻塞在 `recvfrom()`。