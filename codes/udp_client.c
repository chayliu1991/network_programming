#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#define SERV_PORT 43211
#define MAXLINE 4096

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("usage:%s <Ipaddress>\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    int sockfd;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    socklen_t serverlen = sizeof(server_addr);
    struct sockaddr *resp_addr;
    resp_addr = malloc(serverlen);

    char send_line[MAXLINE], resp_line[MAXLINE + 1];
    socklen_t len;
    int n;

    while (fgets(send_line, MAXLINE, stdin) != NULL)
    {
        int i = strlen(send_line);
        if (send_line[i - 1] != '\n')
            send_line[i - 1] = 0;

        printf("now sending %s\n", send_line);

        size_t rt = sendto(sockfd, send_line, strlen(send_line), 0, (struct sockaddr *)&server_addr, serverlen);
        if (rt < 0)
        {
            fprintf(stderr, "sendto failed:%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        printf("send bytes:%zu\n", rt);

        len = 0;
        n = recvfrom(sockfd, resp_line, MAXLINE, 0, resp_addr, &len);
        if (n < 0)
        {
            fprintf(stderr, "recvfrom failed:%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        resp_line[n] = 0;
        fputs("response:", stdout);
        fputs(resp_line, stdout);
        fputs("\n", stdout);
    }

    exit(EXIT_SUCCESS);
}