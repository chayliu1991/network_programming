#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define BUF_SIZE (4096)
#define SA struct sockaddr
#define CSA const struct sockaddr

void errExit(const char *errmsg)
{
    printf("%s:%s\n", errmsg, strerror(errno));
    exit(EXIT_FAILURE);
}

void sig_chld(int signo)
{
    int stat;
    pid_t pid;
    while (waitpid(-1, &stat, WNOHANG) > 0)
    {
        printf("child %ld quit\n", (long)pid);
    }
    return;
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

void to_upper(char *str)
{
    char *pos = str;
    while (*pos != '\0')
    {
        *pos = toupper(*pos);
        pos++;
    }
}

void change_to_upper(int fd)
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
            printf("received: %s\n", buf);
            to_upper(buf);
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
    if (argc != 2)
    {
        printf("usage:%s <LocalPath>\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    //@ 注册 SIGCHLD 信号处理函数，回收子进程
    struct sigaction action;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    action.sa_handler = sig_chld;
    sigaction(SIGCHLD, &action, NULL);

    char *local_path = argv[1];
    if (remove(local_path) == -1 && errno != ENOENT) //@ 移除所有的既有文件
        errExit("remove()");

    int listen_fd, conn_fd;
    socklen_t client_len;
    struct sockaddr_un client_addr, server_addr;

    listen_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (listen_fd < 0)
        errExit("socket()");

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sun_family = AF_LOCAL;
    strncpy(server_addr.sun_path, local_path, sizeof(server_addr.sun_path) - 1);
    if (bind(listen_fd, (SA *)&server_addr, sizeof(server_addr)) < 0)
        errExit("bind()");

    if (listen(listen_fd, 128) < 0)
        errExit("listen()");

    client_len = sizeof(client_addr);
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

        switch (fork())
        {
        case -1:
            errExit("fork()");
        case 0:
        {
            close(listen_fd);
            change_to_upper(conn_fd);
            _exit(EXIT_SUCCESS);
        }
        default:
        {
            close(conn_fd);
            continue;
        }
        }
    }

    exit(EXIT_SUCCESS);
}