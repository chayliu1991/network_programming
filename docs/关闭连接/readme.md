# close() 函数

```
#include <unistd.h>

int close(int fd);
```

- `close()` 成功则返回 0，若出错则返回 -1
- `close()` 会对套接字引用计数减一，一旦发现套接字引用计数到 0，就会对套接字进行彻底释放，并且会关闭TCP两个方向的数据流：
  - 在输入方向，系统内核会将该套接字设置为不可读，任何读操作都会返回异常，如果对端没有检测到套接字已关闭，还继续发送报文，就会收到一个 `RST` 报文
  - 在输出方向，系统内核尝试将发送缓冲区的数据发送给对端，并最后向对端发送一个 `FIN` 报文，接下来如果再对该套接字进行写操作会返回异常

# shutdown() 函数

```
#include <sys/socket.h>

int shutdown(int sockfd, int how);
```

`how` 参数的选项：

- `SHUT_RD(0)`：关闭连接的“读”这个方向，对该套接字进行读操作直接返回 `EOF`。从数据角度来看，套接字上接收缓冲区已有的数据将被丢弃，如果再有新的数据流到达，会对数据进行 `ACK`，然后悄悄地丢弃。也就是说，对端还是会接收到 `ACK`，在这种情况下根本不知道数据已经被丢弃了
- `SHUT_WR(1)`：关闭连接的“写”这个方向，这就是常被称为”半关闭“的连接。套接字上发送缓冲区已有的数据将被立即发送出去，并发送一个FIN报文给对端。应用程序如果对该套接字进行写操作会报错
- `SHUT_RDWR(2)`：相当于 `SHUT_RD` 和 `SHUT_WR` 操作各一次，关闭套接字的读和写两个方向

## close() 和 shutdown() 的区别

- `close()` 会关闭连接，并释放所有连接对应的资源，而 `shutdown()` 并不会释放掉套接字和所有的资源
- `close()` 存在引用计数的概念，并不一定导致该套接字不可用；`shutdown()` 则不管引用计数，直接使得该套接字不可用，如果有别的进程企图使用该套接字，将会受到影响
- `close()` 的引用计数导致不一定会发出 `FIN` 结束报文，而 `shutdown()` 则总是会发出 `FIN` 结束报文，这在我们打算关闭连接通知对端的时候，是非常重要的

在大多数情况下，我们会优选 `shutdown()` 来完成对连接一个方向的关闭，待对端处理完之后，再完成另外一个方向的关闭。

# 测试

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
#include <signal.h>

#define SERV_PORT (50005)
#define MAX_LINE (4096)

static int s_count = 0;

static void sig_int(int sig)
{
    printf("\n received %d datagrams \n", s_count);
    exit(0);
}

static void sig_pipe(int sig)
{
    printf("\n received %d datagrams \n", s_count);
    exit(0);
}

int main(int argc, char **argv)
{
    int listenfd;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERV_PORT);

    int rt1 = bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (rt1 < 0)
    {
        fprintf(stderr, "bind() error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int rt2 = listen(listenfd, 1024);
    if (rt2 < 0)
    {
        fprintf(stderr, "listen() error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, sig_int);
    signal(SIGPIPE, sig_pipe);

    int connfd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    if ((connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_len)) < 0)
    {
        fprintf(stderr, "accept() error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    char message[MAX_LINE];
    s_count = 0;

    for (;;)
    {
        int n = read(connfd, message, MAX_LINE);
        if (n < 0)
        {
            fprintf(stderr, "read() error:%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        else if (n == 0)
        {
            printf("peer close\n");
            exit(EXIT_FAILURE);
        }
        message[n] = 0;
        printf("received %d bytes: %s\n", n, message);
        s_count++;

        char send_line[MAX_LINE];
        sprintf(send_line, "Hi, %s", message);

        sleep(5);

        int write_nc = send(connfd, send_line, strlen(send_line), 0);
        printf("send bytes: %zu \n", write_nc);
        if (write_nc < 0)
        {
            fprintf(stderr, "send() error:%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
}

```

## 客户端

```

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

#define SERV_PORT (50005)
#define MAX_LINE (4096)

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("usage: %s <IPaddress>", argv[0]);
        exit(EXIT_SUCCESS);
    }
    int socket_fd;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    socklen_t server_len = sizeof(server_addr);
    int connect_rt = connect(socket_fd, (struct sockaddr *)&server_addr, server_len);
    if (connect_rt < 0)
    {
        fprintf(stderr, "connect() error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    char send_line[MAX_LINE], recv_line[MAX_LINE + 1];
    int n;

    fd_set readmask;
    fd_set allreads;

    FD_ZERO(&allreads);
    FD_SET(0, &allreads);
    FD_SET(socket_fd, &allreads);
    for (;;)
    {
        readmask = allreads;
        int rc = select(socket_fd + 1, &readmask, NULL, NULL, NULL);
        if (rc <= 0)
        {
            fprintf(stderr, "select() error:%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(socket_fd, &readmask))
        {
            n = read(socket_fd, recv_line, MAX_LINE);
            if (n < 0)
            {
                fprintf(stderr, "read() error:%s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            else if (n == 0)
            {
                printf("peer close\n");
                exit(EXIT_SUCCESS);
            }
            recv_line[n] = 0;
            fputs(recv_line, stdout);
            fputs("\n", stdout);
        }
        if (FD_ISSET(0, &readmask))
        {
            if (fgets(send_line, MAX_LINE, stdin) != NULL)
            {
                if (strncmp(send_line, "shutdown", 8) == 0)
                {
                    FD_CLR(0, &allreads);
                    if (shutdown(socket_fd, 1))
                    {
                        fprintf(stderr, "shutdown() error:%s\n", strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                }
                else if (strncmp(send_line, "close", 5) == 0)
                {
                    FD_CLR(0, &allreads);
                    if (close(socket_fd))
                    {
                        fprintf(stderr, "close() error:%s\n", strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                    sleep(6);
                    exit(0);
                }
                else
                {
                    int i = strlen(send_line);
                    if (send_line[i - 1] == '\n')
                    {
                        send_line[i - 1] = 0;
                    }

                    printf("now sending %s\n", send_line);
                    size_t rt = write(socket_fd, send_line, strlen(send_line));
                    if (rt < 0)
                    {
                        fprintf(stderr, "write() error:%s\n", strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                    printf("send bytes: %zu \n", rt);
                }
            }
        }
    }
}
```







