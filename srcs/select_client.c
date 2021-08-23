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


#define MAX_LINE (1024)
#define SERV_PORT (50005)


int tcp_client(char *address, int port) {
  int socket_fd;
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in server_addr;
  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  inet_pton(AF_INET, address, &server_addr.sin_addr);

  socklen_t server_len = sizeof(server_addr);
  int connect_rt =
      connect(socket_fd, (struct sockaddr *)&server_addr, server_len);
  if (connect_rt < 0) {
    fprintf(stderr, "connect() failed:%s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  return socket_fd;
}


int main(int argc,char* argv[])
{
    if(argc != 2){
        printf("usage: %s <IPaddress>", argv[0]);
        exit(EXIT_SUCCESS);
    }

    int socket_fd = tcp_client(argv[1],SERV_PORT);
    char recv_line[MAX_LINE],send_line[MAX_LINE];
    int n;

    fd_set read_mask;
    fd_set allreads;
    FD_ZERO(&allreads);
    FD_SET(0,&allreads);
    FD_SET(socket_fd,&allreads);

    for(;;){
        read_mask = allreads;
        int rc = select(socket_fd+1,&read_mask,NULL,NULL,NULL);
        if(rc < 0){
            fprintf(stderr, "select() failed:%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if(FD_ISSET(socket_fd,&read_mask)){
            n = read(socket_fd,recv_line,MAX_LINE);
            if(n < 0){
                fprintf(stderr, "read() failed:%s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }else if(n == 0){
                fprintf(stderr, "server terminate\n");
                exit(EXIT_FAILURE);
            }

            recv_line[n] = 0;
            fputs(recv_line,stdout);
            fputs("\n",stdout);
        }

        if(FD_ISSET(STDIN_FILENO,&read_mask)){
            if(fgets(send_line,MAX_LINE,stdin) != NULL){
                int i = strlen(send_line);
                if(send_line[i-1] == '\n'){
                    send_line[i-1] = 0;
                }          
            }

            printf("now sending %s\n",send_line);
            ssize_t rt = write(socket_fd,send_line,strlen(send_line));
            if(rt < 0){
                fprintf(stderr, "write() failed:%s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            printf("send bytes:%zu \n",rt);
        }
    }
}
