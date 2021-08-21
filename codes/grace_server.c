
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

#define SERV_PORT (50005)
#define MAX_LINE (4096)

static int s_count = 0;

static void sig_int(int sig)
{
    printf("\n received %d datagrams \n", s_count);
    exit(0);
}

static void sig_pipe(int sig)
{
    printf("\n received %d datagrams \n", s_count);
    exit(0);
}

int main(int argc, char **argv)
{
    int listenfd;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERV_PORT);

    int rt1 = bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (rt1 < 0)
    {
        fprintf(stderr, "bind() error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int rt2 = listen(listenfd, 1024);
    if (rt2 < 0)
    {
        fprintf(stderr, "listen() error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, sig_int);
    signal(SIGPIPE, sig_pipe);

    int connfd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    if ((connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_len)) < 0)
    {
        fprintf(stderr, "accept() error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    char message[MAX_LINE];
    s_count = 0;

    for (;;)
    {
        int n = read(connfd, message, MAX_LINE);
        if (n < 0)
        {
            fprintf(stderr, "read() error:%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        else if (n == 0)
        {
            printf("peer close\n");
            exit(EXIT_FAILURE);
        }
        message[n] = 0;
        printf("received %d bytes: %s\n", n, message);
        s_count++;

        char send_line[MAX_LINE];
        sprintf(send_line, "Hi, %s", message);

        sleep(5);

        int write_nc = send(connfd, send_line, strlen(send_line), 0);
        printf("send bytes: %zu \n", write_nc);
        if (write_nc < 0)
        {
            fprintf(stderr, "send() error:%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
}
