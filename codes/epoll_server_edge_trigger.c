/// epoll_server_edge_trigger.c

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
#include <sys/epoll.h>
#include <signal.h>
#include <fcntl.h>


#define MAXEVENTS (128)
#define SERV_PORT (50005)
#define MAXEVENTS (128)


void make_nonblocking(int fd) {
    fcntl(fd, F_SETFL, O_NONBLOCK);
}

int tcp_nonblocking_server_listen(int port) 
{
    int listenfd;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    make_nonblocking(listenfd);

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

int main(int argc, char **argv) {
    int listen_fd, socket_fd;
    int n, i;
    int efd;
    struct epoll_event event;
    struct epoll_event *events;

    listen_fd = tcp_nonblocking_server_listen(SERV_PORT);

    efd = epoll_create1(0);
    if (efd == -1) {
         printf(stderr, "epoll_create1() failed:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    event.data.fd = listen_fd;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(efd, EPOLL_CTL_ADD, listen_fd, &event) == -1) {
        fprintf(stderr, "epoll_ctl() failed:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Buffer where events are returned */
    events = calloc(MAXEVENTS, sizeof(event));

    while (1) {
        n = epoll_wait(efd, events, MAXEVENTS, -1);
        printf("epoll_wait wakeup\n");
        for (i = 0; i < n; i++) {
            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP) ||
                (!(events[i].events & EPOLLIN))) {
                fprintf(stderr, "epoll error\n");
                close(events[i].data.fd);
                continue;
            } else if (listen_fd == events[i].data.fd) {
                struct sockaddr_storage ss;
                socklen_t slen = sizeof(ss);
                int fd = accept(listen_fd, (struct sockaddr *) &ss, &slen);
                if (fd < 0) {
                    fprintf(stderr, "accept() failed:%s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                } else {
                    make_nonblocking(fd);
                    event.data.fd = fd;
                    event.events = EPOLLIN | EPOLLET; //edge-triggered
                    if (epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event) == -1) {
                        fprintf(stderr, "epoll_ctl() failed:%s\n", strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                }
                continue;
            } else {
                socket_fd = events[i].data.fd;
                printf("get event on socket fd == %d \n", socket_fd);
            }
        }
    }

    free(events);
    close(listen_fd);

}
