#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
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

void str_client(FILE *fd, int sockfd)
{
    int n;
    int recv[BUF_SIZE], send[BUF_SIZE];
    while (fgets(send, BUF_SIZE, fd) != NULL)
    {
        int nbytes = strlen(send);
        send[nbytes - 1] = '\0';
        if (writen(sockfd, send, nbytes) != nbytes)
            errExit("writen()");

        if ((n = read(sockfd, recv, BUF_SIZE - 1)) > 0)
        {
            recv[n] = '\0';
            printf("client received:%s\n", recv);
        }
        else if (n == 0)
        {
            printf("peer server closed\n");
            break;
        }
        else if (n < 0)
        {
            if (errno == 0)
                continue;
            else
                errExit("read()");
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

    int sock_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (sock_fd < 0)
        errExit("socket()");

    char *local_path = argv[1];
    struct sockaddr_un server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sun_family = AF_LOCAL;
    strcpy(server_addr.sun_path, local_path);

    if (connect(sock_fd, (SA *)&server_addr, sizeof(server_addr)) < 0)
        errExit("connect()");

    str_client(stdin, sock_fd);

    exit(EXIT_SUCCESS);
}
