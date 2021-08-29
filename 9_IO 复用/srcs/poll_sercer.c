#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define SA struct sockaddr
#define INIT_SIZE (128)
#define SERVE_PORT (60006)
#define BUF_SIZE (4096)

void errExit(const char *caller)
{
    fprintf(stderr, "%s  error: %s\n", caller, strerror(errno));
    exit(EXIT_FAILURE);
}

void make_nonblocking(int fd)
{
    fcntl(fd, F_SETFL, O_NONBLOCK);
}

int tcp_nonblocking_server_listen(int port)
{
    signal(SIGPIPE, SIG_IGN);

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
        errExit("socket()");

    make_nonblocking(listen_fd);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    int on = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    if (bind(listen_fd, (SA *)&server_addr, sizeof(server_addr)) < 0)
        errExit("bind()");

    if (listen(listen_fd, 1024) < 0)
        errExit("listen()");

    return listen_fd;
}

int main(int argc, char *agrv[])
{
    int listen_fd = tcp_nonblocking_server_listen(SERVE_PORT);

    //@ 初始化 pollfd 数组，这个数组的第一个元素是 listen_fd，其余的用来记录将要连接的 connect_fd
    struct pollfd event_set[INIT_SIZE];
    event_set[0].fd = listen_fd;
    //@ 监听套接字上的连接建立完成事件
    event_set[0].events = POLLRDNORM;

    //@ 用-1表示这个数组位置还没有被占用
    int i;
    for (i = 1; i < INIT_SIZE; i++)
        event_set[i].fd = -1;

    int ready;
    ssize_t n;
    char buf[BUF_SIZE];
    struct sockaddr_in client_addr;
    int conn_fd;
    for (;;)
    {
        if ((ready = poll(event_set, INIT_SIZE, -1)) < 0)
            errExit("poll()");

        if (event_set[0].revents & POLLRDNORM)
        {
            socklen_t client_len = sizeof(client_addr);
            conn_fd = accept(listen_fd, (SA *)&client_addr, &client_len);
            if (conn_fd < 0)
                errExit("accept()");

            for (i = 1; i < INIT_SIZE; i++)
            {
                if (event_set[i].fd < 0)
                {
                    event_set[i].fd = conn_fd;
                    event_set[i].events = POLLRDNORM;
                    break;
                }
            }

            if (i == INIT_SIZE)
            {
                printf("can not hold so many clients\n");
                exit(EXIT_FAILURE);
            }

            if (--ready <= 0)
                continue;
        }

        for (i = 1; i < INIT_SIZE; i++)
        {
            int socket_fd;
            if ((socket_fd = event_set[i].fd) < 0)
                continue;

            if (event_set[i].revents & (POLLRDNORM | POLLERR))
            {
                if ((n = read(socket_fd, buf, BUF_SIZE - 1)) > 0)
                {
                    buf[n] = '\0';
                    printf("server received %d butes : %s\n", strlen(buf), buf);
                    if (write(socket_fd, buf, n) < 0)
                    {
                        fprintf(stderr, "write() failed:%s\n", strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                }
                else if (n == 0 || errno == ECONNRESET)
                {
                    printf("peer reset\n");
                    close(socket_fd);
                    event_set[i].fd = -1;
                }
                else
                {
                    fprintf(stderr, "read() failed:%s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }

                if (--ready <= 0)
                    break;
            }
        }
    }
}