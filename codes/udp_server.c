#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define SERV_PORT 43211
#define MAXLINE 4096

static int s_count = 0;

static void recvfrom_int(int signo)
{
    printf("\n received %d datagrams \n", s_count);
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    int sockfd;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    socklen_t clilen;
    char message[MAXLINE];
    s_count = 0;

    signal(SIGINT, recvfrom_int);

    struct sockaddr_in client_addr;
    clilen = sizeof(client_addr);
    for (;;)
    {
        printf("server waiting...\n");
        int n = recvfrom(sockfd, message, MAXLINE, 0, (struct sockaddr *)&client_addr, &clilen);
        message[n] = 0;
        printf("received %d bytes:%s\n", n, message);

        char send_line[MAXLINE];
        snprintf(send_line, sizeof(message), "Hi,%s", message);

        sendto(sockfd, send_line, strlen(send_line), 0, (struct sockaddr *)&client_addr, clilen);

        s_count++;
    }
}