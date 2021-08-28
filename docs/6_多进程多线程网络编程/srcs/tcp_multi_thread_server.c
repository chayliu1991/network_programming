#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define SA struct sockaddr
#define SERVE_PORT (60006)
#define BUF_SIZE (4096)

void errExit(const char *caller)
{
    fprintf(stderr, "%s  error: %s\n", caller, strerror(errno));
    exit(EXIT_FAILURE);
}

ssize_t writen(int fd, void *vptr, size_t n)
{
    size_t nleft, nwritten;
    const char *ptr;

    ptr = vptr;
    nleft = n;

    while (nleft > 0)
    {
        if ((nwritten = write(fd, ptr, nleft)) < 0)
        {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0; //@ call write again
            else
                return -1;
        }

        nleft -= nwritten;
        ptr += nwritten;
    }
    return n;
}

void echo(int fd)
{
    char buf[BUF_SIZE];
    ssize_t n = 0;

    while (n = read(fd, buf, BUF_SIZE - 1))
    {
        if (n < 0 && n == EINTR)
            continue;
        else if (n < 0)
        {
            printf("read error occured\n");
            close(fd);
            break;
        }
        else if (n == 0)
        {
            printf("peer client closed\n");
            close(fd);
            break;
        }
        else
        {
            buf[n] = '\0';
            printf("receiver received : %s\n", buf);
            if (writen(fd, buf, n) != n)
            {
                printf("write error occured\n");
                close(fd);
                break;
            }
        }
    }
}

void thread_run(void *arg)
{
    pthread_detach(pthread_self());
    int fd = *((int *)arg);
    echo(fd);
}

int tcp_server_listen(int port)
{
    signal(SIGPIPE, SIG_IGN);

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
        errExit("socket()");

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        errExit("bind()");

    if (listen(listen_fd, 1024) < 0)
        errExit("listen()");

    return listen_fd;
}

int main(int c, char **v)
{
    int listener_fd = tcp_server_listen(SERVE_PORT);
    pthread_t tid;
    while (1)
    {
        struct sockaddr_in ss;
        socklen_t slen = sizeof(ss);
        int conn_fd = accept(listener_fd, (SA *)&ss, &slen);
        if (conn_fd < 0)
            errExit("accept()");
        else
            pthread_create(&tid, NULL, &thread_run, (void *)&conn_fd);
    }

    return 0;
}