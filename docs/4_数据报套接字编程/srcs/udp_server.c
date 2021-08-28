#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SERV_PORT 60006
#define BUF_SIZE 4096
#define SA struct sockaddr
#define CSA const struct sockaddr

static int s_count = 0;

void errExit(const char *errmsg)
{
    printf("%s:%s\n", errmsg, strerror(errno));
    exit(EXIT_FAILURE);
}

static void sig_int(int signo)
{
    printf("\n received %d datagrams \n", s_count);
    exit(EXIT_SUCCESS);
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
    struct sigaction action;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    action.sa_handler = sig_int;
    sigaction(SIGINT, &action, NULL);

    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0)
        errExit("socket()");

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        errExit("bind()");

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buf[BUF_SIZE + 1];
    for (;;)
    {
        printf("server waiting...\n");
        int nbytes = recvfrom(socket_fd, buf, BUF_SIZE, 0, (SA *)&client_addr, &client_len);
        buf[nbytes] = '\0';
        printf("server received %d bytes : %s\n", nbytes, buf);
        to_upper(buf);

        if (sendto(socket_fd, buf, nbytes, 0, (CSA *)&client_addr, client_len) != nbytes)
            errExit("sendto()");

        s_count++;
    }
}