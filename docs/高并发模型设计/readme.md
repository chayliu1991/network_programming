# C10K问题

C10K 问题是这样的：如何在一台物理机上同时服务10000个用户？这里C表示并发，10K等于10000。

C10K 问题本质上是一个操作系统问题，要在一台主机上同时支持1万个连接，意味着什么呢? 需要考虑哪些方面？

## 文件句柄

每个客户连接都代表一个文件描述符，一旦文件描述符不够用了，新的连接就会被放弃，产生如下的错误：

```
Socket/File:Can't open so many files
```

在 Linux下，单个进程打开的文件句柄数是有限制的，没有经过修改的值一般都是1024：

```
$ulimit -n
1024
```

这意味着最多可以服务的连接数上限只能是1024。不过，可以对这个值进行修改，比如用 root 权限修改 `/etc/sysctl.conf` 文件，使得系统可用支持10000个描述符上限：

```
fs.file-max = 10000
net.ipv4.ip_conntrack_max = 10000
net.ipv4.netfilter.ip_conntrack_max = 10000
```

## 系统内存

每个 TCP 连接占用的资源可不止一个连接套接字这么简单，。每个 TCP 连接都需要占用一定的发送缓冲区和接收缓冲区。

发送缓冲区和接收缓冲区的值：

```
$cat   /proc/sys/net/ipv4/tcp_wmem
4096	16384	4194304
$ cat   /proc/sys/net/ipv4/tcp_rmem
4096	87380	6291456
```

这三个值分别表示了最小分配值、默认分配值和最大分配值。按照默认分配值计算，一万个连接需要的内存消耗为：

```
发送缓冲区： 16384*10000 = 160M bytes
接收缓冲区： 87380*10000 = 880M bytes
```

为了方便，我们假设每个连接需要 128K 的缓冲区，那么1万个链接就需要大约 1.2G 的应用层缓冲。这样，我们可以得出大致的结论，支持1万个并发连接，内存并不是一个巨大的瓶颈。

## 网络带宽

假设1万个连接，每个连接每秒传输大约 1KB 的数据，那么带宽需要 `10000 x 1KB/s x8 = 80Mbps`。这在今天的动辄万兆网卡的时代简直小菜一碟。

# C10K问题解决之道

通过对操作系统层面的资源分析，可以得出一个结论，在系统资源层面，C10K问题是可以解决的。

在网络编程中，涉及到频繁的用户态-内核态数据拷贝，设计不够好的程序可能在低并发的情况下工作良好，一旦到了高并发情形，其性能可能呈现出指数级别的损失。如果没有考虑好C10K问题，一个基于select的经典程序可能在一台服务器上可以很好处理1000的并发用户，但是在性能2倍的服务器上，却往往并不能很好地处理2000的并发用户。

要想解决C10K问题，就需要从两个层面上来统筹考虑：

- 应用程序如何和操作系统配合，感知 I/O 事件发生，并调度处理在上万个套接字上的 I/O 操作？阻塞 I/O、非阻塞 I/O 讨论的就是这方面的问题
- 应用程序如何分配进程、线程资源来服务上万个连接

##　阻塞I/O + 进程

每个连接通过 `fork()` 派生一个子进程进行处理，因为一个独立的子进程负责处理了该连接所有的 I/O，所以即便是阻塞 I/O，多个连接之间也不会互相影响。这个方法虽然简单，但是效率不高，扩展性差，资源占用率高。

```
do{
   accept connections
   fork for conneced connection fd
   process_run(fd)
}
```

## 阻塞I/O + 线程

进程模型占用的资源太大，幸运的是，还有一种轻量级的资源模型，这就是线程。

```
do{
   accept connections
   pthread_create for conneced connection fd
   thread_run(fd)
}while(true)
```

因为线程的创建是比较消耗资源的，况且不是每个连接在每个时刻都需要服务，因此，我们可以预先通过创建一个线程池，并在多个连接中复用线程池来获得某种效率上的提升。

```
create thread pool
do{
   accept connections
   get connection fd
   push_queue(fd)
}while(true)
```

## 非阻塞I/O + readiness notification + 单线程

应用程序其实可以采取轮询的方式来对保存的套接字集合进行挨个询问，从而找出需要进行 I/O 处理的套接字。

```
for fd in fdset{
   if(is_readable(fd) == true){
     handle_read(fd)
   }else if(is_writeable(fd)==true){
     handle_write(fd)
   }
}
```

如果这个 `fdset` 有一万个之多，每次循环判断都会消耗大量的 CPU 时间，而且极有可能在一个循环之内，没有任何一个套接字准备好可读，或者可写。既然这样，CPU 的消耗太大，那么干脆让操作系统来告诉我们哪个套接字可以读，哪个套接字可以写。在这个结果发生之前，我们把CPU的控制权交出去，让操作系统来把宝贵的CPU时间调度给那些需要的进程，这就是 `select()`、`poll()` 这样的 I/O 分发技术。

```
do {
    poller.dispatch()
    for fd in registered_fdset{
         if(is_readable(fd) == true){
           handle_read(fd)
         }else if(is_writeable(fd)==true){
           handle_write(fd)
     }
}while(ture)
```

这样的方法需要每次 `dispatch` 之后，对所有注册的套接字进行逐个排查，效率并不是最高的。如果 `dispatch` 调用返回之后只提供有  I/O 事件或者 I/O 变化的套接字，这样排查的效率不就高很多了么？这就是 `epoll()` 的设计。

```
do {
    poller.dispatch()
    for fd_event in active_event_set{
         if(is_readable_event(fd_event) == true){
           handle_read(fd_event)
         }else if(is_writeable_event(fd_event)==true){
           handle_write(fd_event)
     }
}while(ture)
```

## 非阻塞I/O + readiness notification +多线程

如果我们把线程引入进来，可以利用现代 CPU 多核的能力，让每个核都可以作为一个 I/O 分发器进行 I/O 事件的分发。

这就是所谓的主从 reactor 模式。基于 epoll/poll/select 的 I/O 事件分发器可以叫做 reactor，也可以叫做事件驱动，或者事件轮询（eventloop）。

## 异步I/O+ 多线程

异步非阻塞 I/O 模型是一种更为高效的方式，当调用结束之后，请求立即返回，由操作系统后台完成对应的操作，当最终操作完成，就会产生一个信号，或者执行一个回调函数来完成 I/O 处理。

这就涉及到了Linux下的aio机制。

