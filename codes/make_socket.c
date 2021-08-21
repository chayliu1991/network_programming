#include <netinet/in.h> /// sockaddr_in
#include <sys/types.h>
#include <sys/socket.h> /// socket()
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int make_socket(uint16_t port)
{
    int sock;
    struct sockaddr_in name;

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket() error");
        exit(EXIT_FAILURE);
    }

    name.sin_family = AF_INET;
    name.sin_port = htons(port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr *)&name, sizeof(name)) < 0)
    {
        perror("bind() error");
        exit(EXIT_FAILURE);
    }

    return sock;
}

int main(int argc, char *crgv)
{
    int sockfd = make_socket(12345);
    exit(EXIT_SUCCESS);
}