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
