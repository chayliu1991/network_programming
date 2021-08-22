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
#include <sys/time.h>
#include <sys/select.h>

#define SERV_PORT (50005)
#define MAX_LINE (4096)
#define KEEP_ALIVE_TIME (10)
#define KEEP_ALIVE_INTERVAL (3)
#define KEEP_ALIVE_PROBETIMES (3)


int main(int argc,char* argv[])
{
    if (argc != 2)
    {
        printf("usage: %s <IPaddress>", argv[0]);
        exit(EXIT_SUCCESS);
    }

    int socket_fd;
    socket_fd = socket(AF_INET,SOCK_STREAM,0);

    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET,argv[1],&server_addr.sin_addr);

    socklen_t server_len = sizeof(server_addr);
    int connect_rt = connect(socket_fd,(struct sockaddr*)&server_addr,server_len);
    if(connect_rt < 0){
        fprintf(stderr, "connect() error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct  linger ling;
    ling.l_onoff = 1;
    ling.l_linger = 0;
    setsockopt(socket_fd,SOL_SOCKET,SO_LINGER,&ling,sizeof(ling));
    close(socket_fd);
    exit(0);
}
