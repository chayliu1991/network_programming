# 概述

unix 域协议并不是一个实际的协议族，而是在单个主机上执行客户/服务器通信的一种方法。unix 域提供两类套接字：

- 字节流套接字（类似TCP）

- 数据报套接字（类似UDP）

使用 unix 域协议有如下的优势：

- unix 域套接字往往比通信两端位于同一个主机的 TCP 套接字快出一倍
- unix 域套接字可用于在同一个主机上的不同进程之间传递描述符
- unix 域套接字较新的实现把客户的凭证（用户ID和组ID）提供给服务器，从而能够提供额外的安全检查措施

unix 域中用于标识客户和服务器的协议地址是普通文件系统中的路径名。

# 本地套接字 bind()

当用来绑定 UNIX domain socket 时，`bind()` 会在文件系统中创建一个条目。这个文件会被标记为一个 socket，当在这个路径名上应用 `stat()` 时，它会在 `stat`  结构的 `st_mode` 字段中的文件类型部分返回值 `S_IFSOCK`。当使用 `ls -l` 列出时，UNIX domain socket 在第一列将会显示 `s`，而 `ls -F` 则会在 socket 路径名后面加上一个等号 `=`。

绑定一个 UNIX domain socket 方面还需要注意以下几点：

- 无法将一个 socket 绑定到一个既有路径名上，`bind()` 会失败并返回 `EADDRINUSE`  错误
- 通常会将一个 socket 绑定到一个绝对路径名上，这样这个 socket 就会位于文件系统中的一个固定地址，也可以使用一个相对路径名，但是并不常见，因为它要求想要 `connect()` 这个 socket 的应用程序知道执行 `bind()` 的应用程序的当前工作目录
- 一个 socket 只能绑定到一个路径名上，相应地，一个路径名只能被一个 socket 绑定
- 无法使用 `open()` 打开一个 socket
- 当不再需要一个 socket 时可以使用 `unlink()` 或者 `remove()` 删除其路径名条目

# 本地数据报套接字通信

对于本地数据报套接字来讲，数据报的传输是在内核中发生的，并且也是可靠的。所有的消息都会按序传递并且不会发生重复。

在 Linux 上数据报的限制是通过 `SO_SNDBUF` socket 选项和各个 `/proc` 文件来控制的。

# 创建互联的套接字对

有时候让当个进程创建一对 socket 并将它们连接起来是比较有用的。这可以通过使用两个 `socket()` 调用和一个 `bind()` 调用以及对 `listen()`、`connect()`、`accept()` 的调用或对 `connect()` 的调用来完成，`socketpair()` 则为这个操作提供了一个快捷方式：

```
#include <sys/types.h>
#include <sys/socket.h>

int socketpair(int domain, int type, int protocol, int sv[2]);
```

- `socketpair()` 只能用在 UNIX domain 中，即 `type` 必须是 `AF_UNIX`
- `type` 可以被指定为 `SOCK_DGRAM` 或者 `SOCK_STREAM`
- `protocol` 必须是 0
- `sv` 返回引用了这两个相互连接的 socket 的文件描述符

一般来说，socket 对的使用方式与管道的使用方式类似，例如：在调用完 `socketpair()` 之后，进程会使用 `fork()` 创建一个子进程，子进程会继承父进程的文件描述符的副本，因此父子进程可以使用一对 socket 进行 IPC。

使用 `socketpair()` 创建一对 socket  与手工创建一对相互连接的 socket 这两种做法之间的一个差别在于前一对 socket 不会绑定到任意地址上，因为这对 socket 对其他进程是不可见的。

# Linux 抽象 socket 名空间

所谓的抽象路径名空间是 Linux 特有的，它允许将一个本地套接字绑定到一个名字上但不会在文件系统中创建该名字，其具备的优势：

- 无需担心与文件系统中既有名字产生冲突
- 没有必要在使用完 socket 之后删除 socket 路径名，当 socket 被关闭后会自动删除这个抽象名
- 无需为 socket 创建一个文件系统路径名，这对于 chroot 环境以及在不具备文件系统写权限时是比较有用的

要创建一个抽象绑定就需要将 `sun_path` 字段的第一个字节指定为 `NULL` 即 `\0`。

```
struct sockaddr_un addr;

memset(&addr,0,sizeof(struct sockaddr_un));
addr.sun_family = AF_UNIX;

strncpy(&addr.sun_path[1],"xyz",sizeof(addr.sun_path)-2);

sockfd = socket(AF_UNIX,SOCK_STREAM,0);
if(sockfd == -1)
	errExit("socket()");
	
if(bind(sockfd,(struct sockaddr*)&addr,sizeof(struct sockaddr_un)) == -1)
	errExit("bind()");
```

