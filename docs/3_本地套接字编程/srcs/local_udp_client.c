#include <arpa/inet.h>
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
    printf("%s:%s", errmsg, strerror(errno));
    printf("  {%s}-{%s}-{%d}\n", __FILE__, __func__, __LINE__);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("usage:%s <Serverpath> <Localpath>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sock_fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock_fd < 0)
        errExit("socket()");

    struct sockaddr_un server_addr, client_addr;

    //@ 填充服务器本地信息结构体
    const char *remote_path = argv[1];
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sun_family = AF_LOCAL;
    strcpy(server_addr.sun_path, remote_path);
    socklen_t server_len = sizeof(server_addr);

    //@ 客户端需要绑定自己的信息,否则服务器无法给客户端发送数据
    const char *local_path = argv[2];
    if (remove(local_path) == -1 && errno != ENOENT) //@ 移除所有的既有文件
        errExit("remove()");
    unlink(local_path);

    bzero(&client_addr, sizeof(client_addr));
    client_addr.sun_family = AF_LOCAL;
    strcpy(client_addr.sun_path, local_path);
    if (bind(sock_fd, (SA *)&client_addr, sizeof(client_addr)) < 0)
        errExit("bind()");

    char buf[BUF_SIZE] = {0};
    while (fgets(buf, BUF_SIZE, stdin) != NULL)
    {
        int nbytes = strlen(buf);
        buf[nbytes - 1] = '\0';

        if (sendto(sock_fd, buf, nbytes, 0, (SA *)&server_addr, server_len) != nbytes)
            errExit("sendto()");

        if (strncmp(buf, "quit", strlen("quit")) == 0)
        {
            printf("client quit\n");
            break;
        }

        int n = recvfrom(sock_fd, buf, BUF_SIZE - 1, 0, (SA *)&server_addr, &server_len);
        if (n == 0)
        {
            printf("peer server closed\n");
            break;
        }
        else if (n < 0 && errno == EINTR)
            continue;
        else if (n < 0)
            errExit("recvfrom()");
        buf[n] = '\0';

        printf("from %s", server_addr.sun_path);
        printf(" client received : %s\n", buf);
    }

    close(sock_fd);
    exit(EXIT_SUCCESS);
}
