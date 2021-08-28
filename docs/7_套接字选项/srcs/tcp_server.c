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

#define SERVE_PORT (60006)
#define BUF_SIZE (4096)

static int s_count = 0;
void errExit(const char *caller)
{
    fprintf(stderr, "%s  error: %s\n", caller, strerror(errno));
    exit(EXIT_FAILURE);
}

static void sig_int(int signo)
{
    printf("\nreceived %d datagrams\n", s_count);
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
    signal(SIGPIPE, SIG_IGN);

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
        errExit("socket()");

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVE_PORT);

    int on = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        errExit("bind()");

    if (listen(listen_fd, 1024) < 0)
        errExit("listen()");

    int conn_fd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    if ((conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len)) < 0)
        errExit("accept()");

    char message[BUF_SIZE];
    for (;;)
    {
        int n = read(conn_fd, message, BUF_SIZE - 1);
        if (n < 0)
            errExit("read()");
        else if (n == 0)
        {
            printf("client closed\n");
            exit(EXIT_FAILURE);
        }

        message[n] = '\0';
        printf("received %d bytes: %s\n", n, message);
        s_count++;
    }
}