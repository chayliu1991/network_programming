#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
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

void sigchld_handler(int sig)
{
    pid_t pid;
    int stat;
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
    {
        printf("child %ld terminated\n", (long)pid);
    };
    return;
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

int main(int argc, char *argv[])
{
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_handler = sigchld_handler;
    act.sa_flags = 0;
    sigaction(SIGCHLD, &act, NULL);

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
        errExit("socket()");

    struct sockaddr_in server_addr, client_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET,
    server_addr.sin_port = htons(SERVE_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_fd, (SA *)&server_addr, sizeof(server_addr)) < 0)
        errExit("bind()");

    if (listen(listen_fd, 1024) < 0)
        errExit("listen()");

    socklen_t client_len = sizeof(client_addr);
    int conn_fd;
    for (;;)
    {
        conn_fd = accept(listen_fd, (SA *)&client_addr, &client_len);
        if (conn_fd < 0)
        {
            if (errno == EINTR)
                continue;
            else
                errExit("accept()");
        }

        printf("a new connect\n");

        switch (fork())
        {
        case -1:
            errExit("fork()");
        case 0:
        {
            close(listen_fd); //@ 子进程中关闭监听套接字
            echo(conn_fd);
            _exit(EXIT_SUCCESS);
        }
        default: //@ 父进程关闭连接套接字
        {
            close(conn_fd);
        }
        }
    }
}