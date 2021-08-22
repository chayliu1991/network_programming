`epoll()` 通过改进的接口设计，避免了用户态-内核态频繁的数据拷贝，大大提高了系统性能。在使用 `epoll()` 的时候，我们一定要理解条件触发和边缘触发两种模式。

# epoll() 的用法

使用 `epoll()` 进行网络程序的编写，需要三个步骤，分别是 `epoll_create()`，`epoll_ctl()` 和 `epoll_wait()`。

## epoll_create()

```
#include <sys/epoll.h>

int epoll_create(int size);
int epoll_create1(int flags);
```

- `epoll_create()` 方法创建了一个 epoll 实例，这个 epoll 实例被用来调用 `epoll_ctl()` 和 `epoll_wait()` ，如果这个 epoll 实例不再需要，比如服务器正常关机，需要调用 `close()` 方法释放 epoll 实例，这样系统内核可以回收 epoll 实例所分配使用的内核资源
- `size` 在一开始的 `epoll_create()` 实现中，是用来告知内核期望监控的文件描述字大小，然后内核使用这部分的信息来初始化内核数据结构，在新的实现中，这个参数不再被需要，因为内核可以动态分配需要的内核数据结构。我们只需要注意，每次将 `size` 设置成一个大于 0 的整数就可以了
- `epoll_create1()` 的用法和 `epoll_create()` 基本一致，如果 `epoll_create1()` 的输入 `flags` 为 0，则和 `epoll_create()` 一样，内核自动忽略。可以增加如 `EPOLL_CLOEXEC` 的额外选项

## epoll_ctl()

```
#include <sys/epoll.h>

int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
```

- `epoll_ctl()` 往这个 epoll 实例增加或删除监控的事件
- `epfd` 是刚刚调用 `epoll_create()` 创建的 epoll 实例描述字，可以简单理解成是 epoll 句柄
- `op` 表示操作选项，可以选择为：
  - `EPOLL_CTL_ADD`： 向 epoll 实例注册文件描述符对应的事件
  - `EPOLL_CTL_DEL`：向 epoll 实例删除文件描述符对应的事件
  - `EPOLL_CTL_MOD`： 修改文件描述符对应的事件
- `fd` 是注册的事件的文件描述符，比如一个监听套接字
- `event` 表示的是注册的事件类型，并且可以在这个结构体里设置用户需要的数据，其中最为常见的是使用联合结构里的 `fd` 字段，表示事件所对应的文件描述符：

```
typedef union epoll_data {
    void        *ptr;
    int          fd;
    uint32_t     u32;
    uint64_t     u64;
} epoll_data_t;

struct epoll_event {
    uint32_t     events;      /* Epoll events */
    epoll_data_t data;        /* User data variable */
};
```

- 事件类型：
  - `EPOLLIN`：表示对应的文件描述字可以读
  - `EPOLLOUT`：表示对应的文件描述字可以写
  - `EPOLLRDHUP`：表示套接字的一端已经关闭，或者半关闭
  - `EPOLLHUP`：表示对应的文件描述字被挂起
  - `EPOLLET`：设置为 edge-triggered，默认为 level-triggered
- 成功返回 0，若返回 -1 表示出错

## epoll_wait()

```
#include <sys/epoll.h>

int epoll_wait(int epfd, struct epoll_event *events,int maxevents, int timeout);
```

- `epoll_wait()` 函数类似之前的 `poll()` 和 `select()` 函数，调用者进程被挂起，在等待内核 I/O 事件的分发
-  `epfd` 是 epoll 实例描述字，也就是 epoll 句柄
- `events` 返回给用户空间需要处理的 I/O 事件，这是一个数组，数组的大小由 `epoll_wait()` 的返回值决定，这个数组的每个元素都是一个需要待处理的 I/O 事件，其中 events 表示具体的事件类型，事件类型取值和 `epoll_ctl()` 可设置的值一样，这个 `epoll_event` 结构体里的 `data` 值就是在 `epoll_ctl()` 那里设置的 `data`，也就是用户空间和内核空间调用时需要的数据
- `maxevents` 是一个大于 0 的整数，表示 `epoll_wait()` 可以返回的最大事件值
- `timeout` 是 `epoll_wait()` 阻塞调用的超时值，如果这个值设置为 -1，表示不超时；如果设置为 0 则立即返回，即使没有任何 I/O 事件发生

# 程序示例

```
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


char rot13_char(char c) 
{
    if ((c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'M'))
        return c + 13;
    else if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z'))
        return c - 13;
    else
        return c;
}

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

int main(int argc, char **argv)
{
    int listen_fd, socket_fd;
    int n, i;
    int efd;
    struct epoll_event event;
    struct epoll_event *events;

    listen_fd = tcp_nonblocking_server_listen(SERV_PORT);

    efd = epoll_create1(0);
    if (efd == -1) {
        fprintf(stderr, "epoll_create1() failed:%s\n", strerror(errno));
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
                while (1) {
                    char buf[512];
                    if ((n = read(socket_fd, buf, sizeof(buf))) < 0) {
                        if (errno != EAGAIN) {
                            close(socket_fd);
                            fprintf(stderr, "read error\n");
                            exit(EXIT_FAILURE);
                        }
                        break;
                    } else if (n == 0) {
                        close(socket_fd);
                        break;
                    } else {
                        for (i = 0; i < n; ++i) {
                            buf[i] = rot13_char(buf[i]);
                        }
                        if (write(socket_fd, buf, n) < 0) {
                            fprintf(stderr, "write() failed:%s\n", strerror(errno));
                            exit(EXIT_FAILURE);
                        }
                    }
                }
            }
        }
    }

    free(events);
    close(listen_fd);
}
```

测试：

```
telnet localhost 50005
Trying 127.0.0.1...
Connected to localhost.
Escape character is '^]'.
helloworld
uryybjbeyq
```

# edge-triggered VS level-triggered

对于 edge-triggered 和 level-triggered， 官方的说法是一个是边缘触发，一个是条件触发。

条件触发的意思是只要满足事件的条件，比如有数据需要读，就一直不断地把这个事件传递给用户；而边缘触发的意思是只有第一次满足条件的时候才触发，之后就不会再传递同样的事件了。

一般我们认为，边缘触发的效率比条件触发的效率要高，这一点也是 epoll 的杀手锏之一。

## edge-triggered

设置为 edge-triggered，即边缘触发。开启这个服务器程序，用 telnet 连接上，输入一些字符，我们看到，服务器端只从 epoll_wait 中苏醒过一次，就是第一次有数据可读的时候。

```
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
#define MAXEVENTS 128


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
```

服务端：

```
./epoll_server_edge_trigger 
epoll_wait wakeup
epoll_wait wakeup
get event on socket fd == 5 
```

客户端：

```
telnet localhost 50005
Trying 127.0.0.1...
Connected to localhost.
Escape character is '^]'.
asfafas
```

## level-triggered

设置为 level-triggered，即条件触发。然后按照同样的步骤来一次，观察服务器端，这一次我们可以看到，服务器端不断地从 `epoll_wait()` 中苏醒，告诉我们有数据需要读取。

```
/// epoll_server_level_trigger.c

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
        fprintf(stderr, "epoll_create1() failed:%s\n", strerror(errno));
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
                    event.events = EPOLLIN;  //level-triggered
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
```

服务端：

```
./epoll_server_level_trigger
epoll_wait wakeup
epoll_wait wakeup
get event on socket fd == 5
epoll_wait wakeup
get event on socket fd == 5
epoll_wait wakeup
get event on socket fd == 5
epoll_wait wakeup
get event on socket fd == 5
```

 客户端：

```
telnet localhost 50005
Trying 127.0.0.1...
Connected to localhost.
Escape character is '^]'.
asfafas
```

























