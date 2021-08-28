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

void to_upper(char *str)
{
    char *pos = str;
    while (*pos != '\0')
    {
        *pos = toupper(*pos);
        pos++;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("usage:%s <Localpath>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *local_path = argv[1];
    if (remove(local_path) == -1 && errno != ENOENT) //@ 移除所有的既有文件
        errExit("remove()");
    unlink(local_path);

    int sock_fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock_fd < 0)
        errExit("socket()");

    struct sockaddr_un server_addr, client_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sun_family = AF_LOCAL;
    strcpy(server_addr.sun_path, local_path);
    if (bind(sock_fd, (SA *)&server_addr, sizeof(server_addr)) < 0)
        errExit("bind()");

    char buf[BUF_SIZE] = {0};
    socklen_t client_len = sizeof(client_addr);
    for (;;)
    {
        bzero(buf, sizeof(buf));
        int n = recvfrom(sock_fd, buf, BUF_SIZE - 1, 0, (SA *)&client_addr, &client_len);
        if (n < 0 && errno == EINTR)
            continue;
        else if (n < 0)
            errExit("recvfrom()");

        buf[n] = '\0';
        printf("%s", client_addr.sun_path);
        if (strncmp(buf, "quit", strlen("quit")) == 0)
        {
            printf(" quit\n");
            continue;
        }

        printf(" server received : %s\n", buf);
        to_upper(buf);
        size_t nbytes = strlen(buf);
        if (sendto(sock_fd, buf, nbytes, 0, (CSA *)&client_addr, client_len) != nbytes)
            errExit("sendto()");
    }
}
