/// epoll_server_edge_trigger.c

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define SA struct sockaddr
#define MAXEVENTS (128)
#define SERV_PORT (60006)

void errExit(const char *caller)
{
    fprintf(stderr, "%s  error: %s\n", caller, strerror(errno));
    exit(EXIT_FAILURE);
}

void make_nonblocking(int fd)
{
    fcntl(fd, F_SETFL, O_NONBLOCK);
}

int tcp_nonblocking_server_listen(int port)
{
    signal(SIGPIPE, SIG_IGN);

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
        errExit("socket()");

    make_nonblocking(listen_fd);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    int on = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    if (bind(listen_fd, (SA *)&server_addr, sizeof(server_addr)) < 0)
        errExit("bind()");

    if (listen(listen_fd, 1024) < 0)
        errExit("listen()");

    return listen_fd;
}

int main(int argc, char **argv)
{
    int listen_fd = tcp_nonblocking_server_listen(SERV_PORT);
    int efd = epoll_create1(0);
    if (efd == -1)
        errExit("epoll_create1()");

    struct epoll_event event;
    event.data.fd = listen_fd;
    event.events = EPOLLIN | EPOLLET; //@ edge trigger
    //@ 添加 listen_fd 到 epollfd
    if (epoll_ctl(efd, EPOLL_CTL_ADD, listen_fd, &event) == -1)
        errExit("epoll_ctl() - ADD");

    /* Buffer where events are returned */
    struct epoll_event *events;
    events = calloc(MAXEVENTS, sizeof(event));
    while (1)
    {
        int nready = epoll_wait(efd, events, MAXEVENTS, -1);
        if (nready < 0)
            errExit("epoll_wait()");
        printf("epoll_wait wakeup\n");

        int i;
        for (i = 0; i < nready; i++)
        {
            if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN)))
            {
                printf("epoll error\n");
                close(events[i].data.fd);
                continue;
            }
            else if (listen_fd == events[i].data.fd)
            {
                struct sockaddr_in client_addr;
                socklen_t clien_len = sizeof(client_addr);
                int fd = accept(listen_fd, (SA *)&client_addr, &clien_len);
                if (fd < 0)
                    errExit("accept()");
                else
                {
                    make_nonblocking(fd);
                    event.data.fd = fd;
                    event.events = EPOLLIN | EPOLLET; //edge-triggered
                    if (epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event) == -1)
                        errExit("epoll_ctl() - ADD");
                }
                continue;
            }
            else
            {
                int fd = events[i].data.fd;
                printf("get event on socket fd == %d \n", fd);
            }
        }
    }

    free(events);
    close(listen_fd);
    exit(EXIT_SUCCESS);
}
