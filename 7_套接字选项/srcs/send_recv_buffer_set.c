#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE (512)

int main(int argc, char *argv[])
{
    if (argc < 5)
    {
        printf("usage: %s <IPAddress> <PortNnumber> <recvbuffer> <sendbuffer>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &server_addr.sin_addr);
    server_addr.sin_port = htons(port);

    int sock_fd = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock_fd >= 0);

    int sendbuf = atoi(argv[3]);
    int send_len = sizeof(sendbuf);
    int recvbuf = atoi(argv[4]);
    int recv_len = sizeof(recvbuf);

    //@ 先设置 TCP 发送缓冲区的大小，然后立即读取之
    getsockopt(sock_fd, SOL_SOCKET, SO_SNDBUF, &sendbuf, (socklen_t *)&send_len);
    printf("the tcp send buffer size before setting is %d \n", sendbuf);
    setsockopt(sock_fd, SOL_SOCKET, SO_SNDBUF, &sendbuf, sizeof(sendbuf));
    getsockopt(sock_fd, SOL_SOCKET, SO_SNDBUF, &sendbuf, (socklen_t *)&send_len);
    printf("the tcp send buffer size after setting is %d \n", sendbuf);

    //@ 先设置 TCP 接收缓冲区的大小，然后立即读取之
    getsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, &recvbuf, (socklen_t *)&recv_len);
    printf("the tcp receive buffer size before setting is %d \n", recvbuf);
    setsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, &recvbuf, sizeof(recvbuf));
    getsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, &recvbuf, (socklen_t *)&recv_len);
    printf("the tcp receive buffer size after setting is %d \n", recvbuf);

    return 0;
}
