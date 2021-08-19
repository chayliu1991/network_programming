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

int main(int argc, char **argv)
{
    if (argc < 2 || strcmp(argv[1], "--help") == 0)
    {
        fprintf(stderr, "usage: %s <LocalPath> \n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sockfd;
    struct sockaddr_un servaddr;
    int res;

    sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        fprintf(stderr, "socket() error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path, argv[1]);

    res = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (res < 0)
    {
        fprintf(stderr, "connect() error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    char send_line[MAX_LINE];
    bzero(send_line, sizeof(send_line));
    char resp_line[MAX_LINE];

    while (fgets(send_line, MAX_LINE, stdin) != NULL)
    {
        int nbytes = sizeof(send_line);
        if (write(sockfd, send_line, nbytes) != nbytes)
        {
            fprintf(stderr, "write() error:%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (read(sockfd, resp_line, MAX_LINE) == 0)
        {
            fprintf(stderr, "read() error:%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        fputs(resp_line, stdout);
    }

    exit(EXIT_SUCCESS);
}