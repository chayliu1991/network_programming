#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
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
    int clients[FD_SETSIZE];
    for (int i = 0; i < FD_SETSIZE; i++)
        clients[i] = -1;

    int listen_fd = tcp_nonblocking_server_listen(SERVE_PORT);
    fd_set set, allset;
    FD_ZERO(&allset);
    FD_SET(listen_fd, &allset);

    int conn_fd, max_client_index;
    int max_fd = listen_fd;
    struct sockaddr_in client_addr;
    char buf[BUF_SIZE];
    for (;;)
    {
        set = allset; //@ 每次循环都要重置想要获取的事件
        int nready = select(max_fd + 1, &set, NULL, NULL, NULL);
        if (nready < 0)
            errExit("select()");

        //@ 监听套接字上有可读事件
        if (FD_ISSET(listen_fd, &set))
        {
            socklen_t client_len = sizeof(client_addr);
            conn_fd = accept(listen_fd, (SA *)&client_addr, &client_len);
            if (conn_fd < 0)
                errExit("accept()");

            int i;
            for (i = 0; i < FD_SETSIZE; i++)
            {
                if (clients[i] == -1)
                {
                    clients[i] = conn_fd;
                    break;
                }
            }
            if (i == FD_SETSIZE)
            {
                printf("too many clients\n");
                exit(EXIT_FAILURE);
            }

            FD_SET(conn_fd, &allset);
            if (conn_fd > max_fd)
                max_fd = conn_fd;
            if (i > max_client_index)
                max_client_index = i;

            //@ 没有更对的就绪事件可供处理
            if (--nready <= 0)
                continue;
        }

        //@ 遍历所有的客户端看看是否存在就绪事件
        for (int i = 0; i <= max_client_index; i++)
        {
            int fd = clients[i];
            if (fd < 0)
                continue;
            if (FD_ISSET(fd, &set))
            {
                int nbytes = read(fd, buf, BUF_SIZE - 1);
                if (nbytes == 0)
                {
                    printf("peer client closed\n");
                    FD_CLR(fd, &set);
                    close(fd);
                    continue;
                }
                else if (nbytes < 0)
                {
                    printf("read error\n");
                    FD_CLR(fd, &set);
                    close(fd);
                    continue;
                }
                else
                {
                    buf[nbytes] = '\0';
                    printf("server received :%d : %s\n", nbytes, buf);
                    if (write(fd, buf, nbytes) < 0)
                        errExit("write()");
                }

                //@ 没有更对的就绪事件可供处理
                if (--nready <= 0)
                    break;
            }
        }
    }
}