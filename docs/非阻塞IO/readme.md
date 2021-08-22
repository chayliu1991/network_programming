非阻塞 I/O 可以使用在 `read()`、`write()`、`accept()`、`connect()` 等多种不同的场景，在非阻塞 I/O 下，使用轮询的方式引起 CPU 占用率高，所以一般将非阻塞 I/O 和 I/O 多路复用技术 `select()`、`poll()` 等搭配使用，在非阻塞 I/O 事件发生时，再调用对应事件的处理函数。这种方式，极大地提高了程序的健壮性和稳定性，是 Linux 下高性能网络编程的首选。

# 阻塞 VS 非阻塞

当应用程序调用阻塞 I/O 完成某个操作时，应用程序会被挂起，等待内核完成操作，感觉上应用程序像是被 “阻塞” 了一样。实际上内核所做的事情是将 CPU 时间切换给其他有需要的进程，网络应用程序在这种情况下就会得不到 CPU 时间做该做的事情。

非阻塞 I/O 则不然，当应用程序调用非阻塞 I/O 完成某个操作时，内核立即返回，不会把 CPU 时间切换给其他进程，应用程序在返回后，可以得到足够的 CPU 时间继续完成其他事情。

按照使用场景，非阻塞I/O可以被用到读操作、写操作、接收连接操作和发起连接操作上。

# 非阻塞 I/O

## 读操作

如果套接字对应的接收缓冲区没有数据可读，在非阻塞情况下 `read()` 调用会立即返回，一般返回 `EWOULDBLOCK` 或 `EAGAIN` 出错信息。在这种情况下，出错信息是需要小心处理，比如后面再次调用 `read()` 操作，而不是直接作为错误直接返回。

## 写操作

在阻塞 I/O 情况下，`write()` 函数返回的字节数，和输入的参数总是一样的。在非阻塞 I/O 的情况下，如果套接字的发送缓冲区已达到了极限，不能容纳更多的字节，那么操作系统内核会尽最大可能从应用程序拷贝数据到发送缓冲区中，并立即从 `write()` 等函数调用中返回。可想而知，在拷贝动作发生的瞬间，有可能一个字符也没拷贝，有可能所有请求字符都被拷贝完成，那么这个时候就需要返回一个数值，告诉应用程序到底有多少数据被成功拷贝到了发送缓冲区中，应用程序需要再次调用 `write()` 函数，以输出未完成拷贝的字节。

`write()` 等函数是可以同时作用到阻塞 I/O 和非阻塞 I/O 上的，为了复用一个函数，处理非阻塞和阻塞 I/O 多种情况，设计出了写入返回值，并用这个返回值表示实际写入的数据大小。

也就是说，非阻塞 I/O 和阻塞 I/O 处理的方式是不一样的：

- 非阻塞 I/O 需要这样：拷贝→返回→再拷贝→再返回
- 阻塞 I/O 需要这样：拷贝→直到所有数据拷贝至发送缓冲区完成→返回

不过在实战中，可以不用区别阻塞和非阻塞 I/O，使用循环的方式来写入数据就好了。只不过在阻塞 I/O 的情况下，循环只执行一次就结束了。

```
ssize_t writen(int fd,const void* data,size_t b)
{
	size_t nleft;
	ssize_t nwritten;
	const char* ptr;
	
	ptr = data;
	nleft = n;
	
	while(nleft > 0){
		if((nwritten = write(fd,ptr,nleft)) <= 0){
			//@ 这里EINTR是非阻塞non-blocking情况下，通知我们再次调用write() 
			if(nwritten < 0 && errno == EINTR)
				nwritten = 0;
			else
				return -1; //@ 出错退出
		}
		
		nleft -= nwritten;
		ptr += nwritten;		
	}
	return n;
}
```

## 读写小结

![](./img/read_write.png)

- `read()` 总是在接收缓冲区有数据时就立即返回，不是等到应用程序给定的数据充满才返回。当接收缓冲区为空时，阻塞模式会等待，非阻塞模式立即返回 -1，并有 `EWOULDBLOCK` 或 `EAGAIN` 错误
- 和 `read()` 不同，阻塞模式下，`write()` 只有在发送缓冲区足以容纳应用程序的输出字节时才返回；而非阻塞模式下，则是能写入多少就写入多少，并返回实际写入的字节数
- 阻塞模式下的 `write()` 有个特例, 就是对方主动关闭了套接字，这个时候 `write()` 调用会立即返回，并通过返回值告诉应用程序实际写入的字节数，如果再次对这样的套接字进行 `write()` 操作，就会返回失败。失败是通过返回值 -1来通知到应用程序的

## accept()

当 `accept()` 和 I/O 多路复用 `select()`、`poll()` 等一起配合使用时，如果在监听套接字上触发事件，说明有连接建立完成，此时调用accept肯定可以返回已连接套接字。

构建一个客户端程序，设置 `SO_LINGER` 套接字选项，把 `l_onoff` 标志设置为1，把 `l_linger` 时间设置为0。这样，连接被关闭时，TCP 套接字上将会发送一个 RST：

```
struct linger ling;
ling.l_onoff = 1; 
ling.l_linger = 0;
setsockopt(socket_fd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling));
close(socket_fd);
```

服务器端使用 `select()`  I/O多路复用，不过，监听套接字仍然是 `blocking` 的。如果监听套接字上有事件发生，休眠 5 秒：

```
if (FD_ISSET(listen_fd, &readset)) {
    printf("listening socket readable\n");
    sleep(5);
    struct sockaddr_storage ss;
    socklen_t slen = sizeof(ss);
    int fd = accept(listen_fd, (struct sockaddr *) &ss, &slen);
    }
```

在监听套接字上有可读事件发生时，并没有马上调用 `accept()`。由于客户端发生了 RST 分节，该连接被接收端内核从自己的已完成队列中删除了，此时再调用 `accept()`，由于没有已完成连接（假设没有其他已完成连接），`accept()` 一直阻塞，更为严重的是，该线程再也没有机会对其他 I/O 事件进行分发，相当于该服务器无法对新连接和其他 I/O 进行服务。

如果我们将监听套接字设为非阻塞，上述的情形就不会再发生。只不过对于 `accept()` 的返回值，需要正确地处理各种看似异常的错误，例如忽略 `EWOULDBLOCK`、`EAGAIN` 等。

这个例子给我们的启发是，一定要将监听套接字设置为非阻塞的。

## connect()

在非阻塞 TCP 套接字上调用 `connect()` 函数，会立即返回一个 `EINPROGRESS` 错误。TCP三次握手会正常进行，应用程序可以继续做其他初始化的事情。当该连接建立成功或者失败时，通过 I/O 多路复用 `select()`、`poll()` 等可以进行连接的状态检测。

# 非阻塞 I/O + select 多路复用

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
#include <sys/select.h>
#include <signal.h>
#include <fcntl.h>


#define SERV_PORT (50005)
#define MAX_LINE 1024
#define FD_INIT_SIZE 128

char rot13_char(char c) 
{
    if ((c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'M'))
        return c + 13;
    else if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z'))
        return c - 13;
    else
        return c;
}

//@ 数据缓冲区
struct Buffer {
    int connect_fd;  //@ 连接字
    char buffer[MAX_LINE];  //@ 实际缓冲
    size_t writeIndex;      //@ 缓冲写入位置
    size_t readIndex;       //@ 缓冲读取位置
    int readable;           //@ 是否可以读
};

//@ 分配一个 Buffer 对象并初始化
struct Buffer *alloc_Buffer() 
{
    struct Buffer *buffer = malloc(sizeof(struct Buffer));
    if (!buffer)
        return NULL;
    buffer->connect_fd = 0;
    buffer->writeIndex = buffer->readIndex = buffer->readable = 0;
    return buffer;
}

//@ 释放Buffer对象
void free_Buffer(struct Buffer *buffer) 
{
    free(buffer);
}

//@ 这里从fd套接字读取数据，数据先读取到本地buf数组中，再逐个拷贝到buffer对象缓冲中
int onSocketRead(int fd, struct Buffer *buffer) 
{
    char buf[1024];
    int i;
    ssize_t result;
    while (1) {
        result = recv(fd, buf, sizeof(buf), 0);
        if (result <= 0)
            break;

        //@ 按char对每个字节进行拷贝，每个字节都会先调用rot13_char来完成编码，之后拷贝到buffer对象的缓冲中，
        //@ 其中writeIndex标志了缓冲中写的位置
        for (i = 0; i < result; ++i) {
            if (buffer->writeIndex < sizeof(buffer->buffer))
                buffer->buffer[buffer->writeIndex++] = rot13_char(buf[i]);
            //@ 如果读取了回车符，则认为client端发送结束，此时可以把编码后的数据回送给客户端
            if (buf[i] == '\n') {
                buffer->readable = 1;  //@ 缓冲区可以读
            }
        }
    }

    if (result == 0) {
        return 1;
    } else if (result < 0) {
        if (errno == EAGAIN)
            return 0;
        return -1;
    }

    return 0;
}


//@ 从buffer对象的readIndex开始读，一直读到writeIndex的位置，这段区间是有效数据
int onSocketWrite(int fd, struct Buffer *buffer) 
{
    while (buffer->readIndex < buffer->writeIndex) {
        ssize_t result = send(fd, buffer->buffer + buffer->readIndex, buffer->writeIndex - buffer->readIndex, 0);
        if (result < 0) {
            if (errno == EAGAIN)
                return 0;
            return -1;
        }

        buffer->readIndex += result;
    }

    //@ readindex已经追上writeIndex，说明有效发送区间已经全部读完，将readIndex和writeIndex设置为0，复用这段缓冲
    if (buffer->readIndex == buffer->writeIndex)
        buffer->readIndex = buffer->writeIndex = 0;

    //@ 缓冲数据已经全部读完，不需要再读
    buffer->readable = 0;

    return 0;
}

void make_nonblocking(int fd) 
{
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
    int listen_fd;
    int i, maxfd;

    struct Buffer *buffer[FD_INIT_SIZE];
    for (i = 0; i < FD_INIT_SIZE; ++i) {
        buffer[i] = alloc_Buffer();
    }

    listen_fd = tcp_nonblocking_server_listen(SERV_PORT);

    fd_set readset, writeset, exset;
    FD_ZERO(&readset);
    FD_ZERO(&writeset);
    FD_ZERO(&exset);

    while (1) {
        maxfd = listen_fd;

        FD_ZERO(&readset);
        FD_ZERO(&writeset);
        FD_ZERO(&exset);

        //@ listener加入readset
        FD_SET(listen_fd, &readset);

        for (i = 0; i < FD_INIT_SIZE; ++i) {
            if (buffer[i]->connect_fd > 0) {
                if (buffer[i]->connect_fd > maxfd)
                    maxfd = buffer[i]->connect_fd;
                FD_SET(buffer[i]->connect_fd, &readset);
                if (buffer[i]->readable) {
                    FD_SET(buffer[i]->connect_fd, &writeset);
                }
            }
        }

        if (select(maxfd + 1, &readset, &writeset, &exset, NULL) < 0) {
            fprintf(stderr, "select() failed:%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(listen_fd, &readset)) {
            printf("listening socket readable\n");
            sleep(5);
            struct sockaddr_storage ss;
            socklen_t slen = sizeof(ss);
            int fd = accept(listen_fd, (struct sockaddr *) &ss, &slen);
            if (fd < 0) {
                fprintf(stderr, "accept() failed:%s\n", strerror(errno));
                exit(EXIT_FAILURE);
            } else if (fd > FD_INIT_SIZE) {
                close(fd);
                fprintf(stderr, "too many connections\n");
                exit(EXIT_FAILURE);
            } else {
                make_nonblocking(fd);
                if (buffer[fd]->connect_fd == 0) {
                    buffer[fd]->connect_fd = fd;
                } else {
                    fprintf(stderr, "too many connections\n");
                    exit(EXIT_FAILURE);
                }
            }
        }

        for (i = 0; i < maxfd + 1; ++i) {
            int r = 0;
            if (i == listen_fd)
                continue;

            if (FD_ISSET(i, &readset)) {
                r = onSocketRead(i, buffer[i]);
            }
            if (r == 0 && FD_ISSET(i, &writeset)) {
                r = onSocketWrite(i, buffer[i]);
            }
            if (r) {
                buffer[i]->connect_fd = 0;
                close(i);
            }
        }
    }
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
fnsnfs
safasf
```