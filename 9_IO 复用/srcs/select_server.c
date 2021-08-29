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

int main(int argc, char **argv)
{
    int i, maxi, max_fd, conn_fd, sock_fd;
    int nready, client[FD_SETSIZE];
    ssize_t n;
    fd_set rset, all_set;
    char buf[BUF_SIZE];
    socklen_t client_len;
    struct sockaddr_in client_addr, server_addr;

    int listen_fd = tcp_nonblocking_server_listen(SERVE_PORT);
    max_fd = listen_fd;
    maxi = -1;
    for (i = 0; i < FD_SETSIZE; i++)
        client[i] = -1;
    FD_ZERO(&all_set);
    FD_SET(listen_fd, &all_set);

    for (;;)
    {
        rset = all_set;
        nready = select(max_fd + 1, &rset, NULL, NULL, NULL);
        if (nready < 0)
            errExit("select()");

        if (FD_ISSET(listen_fd, &rset))
        {
            client_len = sizeof(client_addr);
            conn_fd = accept(listen_fd, (SA *)&client_addr, &client_len);
            if (conn_fd < 0)
                errExit("accept()");

            for (i = 0; i < FD_SETSIZE; i++)
            {
                if (client[i] < 0)
                {
                    client[i] = conn_fd;
                    break;
                }
            }

            if (i == FD_SETSIZE)
            {
                printf("too many clients\n");
                exit(EXIT_FAILURE);
            }
            FD_SET(conn_fd, &all_set);
            if (conn_fd > max_fd)
                max_fd = conn_fd;
            if (i > maxi)
                maxi = i;
            if (--nready <= 0)
                continue;
        }

        for (i = 0; i <= maxi; i++)
        {
            if ((sock_fd = client[i]) < 0)
                continue;
            if (FD_ISSET(sock_fd, &rset))
            {
                if ((n = read(sock_fd, buf, BUF_SIZE - 1)) == 0)
                {
                    printf("peer client closed\n");
                    FD_CLR(sock_fd, &all_set);
                    close(sock_fd);
                    client[i] = -1;
                }
                else
                {
                    buf[n] = '\0';
                    printf("server received :%d : %s\n", strlen(buf), buf);
                    if (write(sock_fd, buf, n) < 0)
                        errExit("write()");
                }

                if (--nready <= 0)
                    break;
            }
        }
    }
}