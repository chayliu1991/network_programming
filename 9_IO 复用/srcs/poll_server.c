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

int main(int argc, char *argv[])
{
    int listen_fd = tcp_nonblocking_server_listen(SERVE_PORT);
    struct pollfd event_set[INIT_SIZE];
    //@ 初始化 pollfd 数组，这个数组的第一个元素是 listen_fd，其余的用来记录将要连接的 connect_fd
    event_set[0].fd = listen_fd;
    event_set[0].events = POLLRDNORM; //@ 监听套接字上的连接建立完成事件

    //@ 用 -1 表示这个数组位置还没有被占用
    for (int i = 1; i < INIT_SIZE; i++)
        event_set[i].fd = -1;

    char buf[BUF_SIZE];
    struct sockaddr_in client_addr;
    for (;;)
    {
        int nready = poll(event_set, INIT_SIZE, -1);
        if (nready < 0)
            errExit("poll()");

        if (event_set[0].events & POLLRDNORM)
        {
            socklen_t client_len = sizeof(client_addr);
            int conn_fd = accept(listen_fd, (SA *)&client_addr, &client_len);
            if (conn_fd < 0)
                errExit("accept()");

            int i;
            for (i = 1; i < INIT_SIZE; i++)
            {
                if (event_set[i].fd == -1)
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

            if (--nready <= 0)
                continue;
        }

        for (int i = 1; i < INIT_SIZE; i++)
        {
            int fd = event_set[i].fd;
            if (event_set[i].events & (POLLRDNORM | POLLERR))
            {
                int nbytes = read(fd, buf, BUF_SIZE - 1);
                if (nbytes > 0)
                {
                    buf[nbytes] = '\0';
                    printf("server received : %d : %s\n", nbytes, buf);
                    if (write(fd, buf, nbytes) != nbytes)
                        errExit("write()");
                }
                else if (nbytes == 0 || errno == ECONNRESET)
                {
                    printf("peer client closed\n");
                    event_set[i].fd = -1;
                    close(fd);
                }
                else
                    errExit("read()");
            }

            if (--nready <= 0)
                break;
        }
    }
}