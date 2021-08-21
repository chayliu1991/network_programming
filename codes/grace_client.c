
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

#define SERV_PORT (50005)
#define MAX_LINE (4096)

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("usage: %s <IPaddress>", argv[0]);
        exit(EXIT_SUCCESS);
    }
    int socket_fd;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    socklen_t server_len = sizeof(server_addr);
    int connect_rt = connect(socket_fd, (struct sockaddr *)&server_addr, server_len);
    if (connect_rt < 0)
    {
        fprintf(stderr, "connect() error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    char send_line[MAX_LINE], recv_line[MAX_LINE + 1];
    int n;

    fd_set readmask;
    fd_set allreads;

    FD_ZERO(&allreads);
    FD_SET(0, &allreads);
    FD_SET(socket_fd, &allreads);
    for (;;)
    {
        readmask = allreads;
        int rc = select(socket_fd + 1, &readmask, NULL, NULL, NULL);
        if (rc <= 0)
        {
            fprintf(stderr, "select() error:%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(socket_fd, &readmask))
        {
            n = read(socket_fd, recv_line, MAX_LINE);
            if (n < 0)
            {
                fprintf(stderr, "read() error:%s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            else if (n == 0)
            {
                printf("peer close\n");
                exit(EXIT_SUCCESS);
            }
            recv_line[n] = 0;
            fputs(recv_line, stdout);
            fputs("\n", stdout);
        }
        if (FD_ISSET(0, &readmask))
        {
            if (fgets(send_line, MAX_LINE, stdin) != NULL)
            {
                if (strncmp(send_line, "shutdown", 8) == 0)
                {
                    FD_CLR(0, &allreads);
                    if (shutdown(socket_fd, 1))
                    {
                        fprintf(stderr, "shutdown() error:%s\n", strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                }
                else if (strncmp(send_line, "close", 5) == 0)
                {
                    FD_CLR(0, &allreads);
                    if (close(socket_fd))
                    {
                        fprintf(stderr, "close() error:%s\n", strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                    sleep(6);
                    exit(0);
                }
                else
                {
                    int i = strlen(send_line);
                    if (send_line[i - 1] == '\n')
                    {
                        send_line[i - 1] = 0;
                    }

                    printf("now sending %s\n", send_line);
                    size_t rt = write(socket_fd, send_line, strlen(send_line));
                    if (rt < 0)
                    {
                        fprintf(stderr, "write() error:%s\n", strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                    printf("send bytes: %zu \n", rt);
                }
            }
        }
    }
}
