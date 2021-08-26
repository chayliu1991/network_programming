#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

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
    int socket_fd;
    socklen_t clilen;
    int res = 0;
    struct sockaddr_un cliaddr, servaddr;

    socket_fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (socket_fd < 0)
    {
        fprintf(stderr, "socket() error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    bzero(&cliaddr, sizeof(cliaddr));
    cliaddr.sun_family = AF_LOCAL;
    strcpy(cliaddr.sun_path, tmpnam(NULL));
    res = bind(socket_fd, (struct sockaddr *)&cliaddr, sizeof(cliaddr));
    if (res < 0)
    {
        fprintf(stderr, "bind() error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path, local_path);

    char send_line[MAX_LINE];
    bzero(send_line, sizeof(send_line));
    char resp_line[MAX_LINE];

    while (fgets(send_line, MAX_LINE, stdin) != NULL)
    {
        int i = strlen(send_line);
        if (send_line[i - 1] != '\n')
            send_line[i - 1] = 0;

        size_t nbytes = strlen(send_line);
        printf("now sending %s\n", send_line);
        if (sendto(socket_fd, send_line, nbytes, 0, &servaddr, sizeof(servaddr)) != nbytes)
        {
            fprintf(stderr, "accept() error:%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        int n = recvfrom(socket_fd, resp_line, MAX_LINE, 0, NULL, NULL);
        resp_line[n] = 0;

        fputs(resp_line, stdout);
        fputs("\n", stdout);
    }

    close(socket_fd);
    exit(EXIT_SUCCESS);
}
