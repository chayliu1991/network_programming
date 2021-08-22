# poll() 函数介绍

`poll()` 是除了 `select()` 之外，另一种普遍使用的 I/O 多路复用技术，和 `select()` 相比，它和内核交互的数据结构有所变化，另外，也突破了文件描述符的个数限制。

```
#include <poll.h>

int poll(struct pollfd *fds, nfds_t nfds, int timeout);
```

- `pollfd` 的结构如下：
```
struct pollfd {
    int    fd;       /* file descriptor */
    short  events;   /* events to look for */
    short  revents;  /* events returned */
 };
```

- 参数 `nfds` 描述的是数组 `fds` 的大小，简单说，就是向 `poll()` 申请的事件检测的个数
- 参数 `timeout`，描述了 `poll()` 的行为
- 和 `select()` 非常不同的地方在于，`poll()` 每次检测之后的结果不会修改原来的传入值，而是将结果保留在 `revents` 字段中，这样就不需要每次检测完都得重置待检测的描述字和感兴趣的事件。我们可以把 `revents` 理解成 "returned events"

`events` 类型的事件可以分为两大类：

- 可读事件，套接字可读事件和 `select()` 的 `readset()` 基本一致，是系统内核通知应用程序有数据可以读，通过 `read()` 函数执行操作不会被阻塞：

```
#define POLLIN     0x0001    /* any readable data available */
#define POLLPRI    0x0002    /* OOB/Urgent readable data */
#define POLLRDNORM 0x0040    /* non-OOB/URG data available */
#define POLLRDBAND 0x0080    /* OOB/Urgent readable data */
```

- 可写事件，套接字可写事件和 `select()` 的 `writeset()` 基本一致，是系统内核通知套接字缓冲区已准备好，通过 `write()` 函数执行写操作不会被阻塞

```
#define POLLOUT    0x0004    /* file descriptor is writeable */
#define POLLWRNORM POLLOUT   /* no write type differentiation */
#define POLLWRBAND 0x0100    /* OOB/Urgent data can be written */
```

以上两大类的事件都可以在 "returned events" 得到复用。还有另一大类事件，没有办法通过 `poll()` 向系统内核递交检测请求，只能通过 "returned events" 来加以检测，这类事件是各种错误事件：

```
#define POLLERR    0x0008    /* 一些错误发送 */
#define POLLHUP    0x0010    /* 描述符挂起*/
#define POLLNVAL   0x0020    /* 请求的事件无效*/
```

























