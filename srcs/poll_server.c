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
#include <sys/poll.h>
#include <signal.h>


#define INIT_SIZE (128)
#define MAX_LINE  (4096)
#define SERV_PORT (50005)


int tcp_server_listen(int port) {
    int listenfd;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    int on = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    int rt1 = bind(listenfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (rt1 < 0) {
        fprintf(stderr, "bind() failed:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int rt2 = listen(listenfd, 1024);
    if (rt2 < 0) {
        fprintf(stderr, "listen() failed:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    signal(SIGPIPE, SIG_IGN);

    return listenfd;
}

int main(int argc,char* agrv[])
{
    int listen_fd,connected_fd;
    int ready_number;
    ssize_t n;
    char buf[MAX_LINE];
    struct sockaddr_in client_addr;

    listen_fd = tcp_server_listen(SERV_PORT);

    //@ 初始化 pollfd 数组，这个数组的第一个元素是 listen_fd，其余的用来记录将要连接的 connect_fd
    struct pollfd event_set[INIT_SIZE];
    event_set[0].fd = listen_fd;
    //@ 监听套接字上的连接建立完成事件
    event_set[0].events = POLLRDNORM;

    //@ 用-1表示这个数组位置还没有被占用
    int i;
    for(i = 1;i < INIT_SIZE;i++){
        event_set[i].fd = -1;
    }    

    for(;;){
        if((ready_number = poll(event_set,INIT_SIZE,-1)) < 0){
            fprintf(stderr, "poll() failed:%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if(event_set[0].revents & POLLRDNORM){
            socklen_t client_len = sizeof(client_addr);
            connected_fd = accept(listen_fd,(struct sockaddr*)&client_addr,&client_len);

            for(i = 1;i < INIT_SIZE;i++){
                if(event_set[i].fd < 0){
                    event_set[i].fd = connected_fd;
                    event_set[i].events = POLLRDNORM;
                    break;
                }
            }

            if(i == INIT_SIZE){
                fprintf(stderr, "can not hold so many clients\n");
                exit(EXIT_FAILURE);
            }

            if(--ready_number <= 0)
                continue;
        }

        for(i = 1;i < INIT_SIZE;i++){
            int socket_fd;
            if((socket_fd = event_set[i].fd) < 0)
                continue;
            
            if(event_set[i].revents & (POLLRDNORM | POLLERR)){
                if((n = read(socket_fd,buf,MAX_LINE)) > 0){
                    if(write(socket_fd,buf,n) < 0){
                        fprintf(stderr, "write() failed:%s\n", strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                }else if(n == 0 || errno == ECONNRESET){
                    close(socket_fd);
                    event_set[i].fd = -1;
                }else{
                    fprintf(stderr, "read() failed:%s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }

                if(--ready_number <= 0)
                    break;
            }
        }
    }
}