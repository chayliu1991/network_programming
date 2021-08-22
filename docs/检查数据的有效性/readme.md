# 对端的异常状况

通过 `read()` 等调用时，可以通过对 `EOF` 的判断，随时防范对方程序崩溃：

```
int nBytes = recv(connfd, buffer, sizeof(buffer), 0);
if (nBytes == -1) {
    errExit("error read message\n");
} else if (nBytes == 0) {
    errExit("client closed \n");
}
```

当调用 `read()` 函数返回 0 字节时，实际上就是操作系统内核返回 `EOF` 的一种反映。如果是服务器端同时处理多个客户端连接，一般这里会调用 `shutdown()` 关闭连接的这一端。

不是每种情况都可以通过读操作来感知异常，比如，服务器完全崩溃，或者网络中断的情况下，此时，如果是阻塞套接字，会一直阻塞在 `read()` 等调用上，没有办法感知套接字的异常。

几种办法来解决这个问题：

- 设置超时时间

给套接字的 `read()` 操作设置超时，如果超过了一段时间就认为连接已经不存在。

```
struct timeval tv;
tv.tv_sec = 5;
tv.tv_usec = 0;

/// 设置了套接字的读操作超时
setsockopt(connfd,SOL_SOCKET,SO_RCVTIMEO,(const char*)&tv,sizeof(tv));

while(1)
{
	int nbytes = recv(connfd,buffer,sizeof(buffer),0);
	if(nbytes == -1)
	{
		if(errno == EAGAIN || errno == EWOULDBLOCK){
			printf("read timeout\n");
			onClientTimeout(connfd);
		}else{
			errExit("error read message");
		}
	}else if(nbytes == 0){
		errExit("client close");
	}
	....
}
```

- 检测连接是否正常

添加对连接是否正常的检测。如果连接不正常，需要从当前 `read()` 阻塞中返回并处理。

- 利用多路复用技术自带的超时能力，来完成对套接字的 IO 检查，如果超过了预设的时间，就进入异常处理：

```
struct timeval tv;
tv.tv_sec = 5;
tv.tv_usec = 0;

FD_ZERO(&allreads);
FD_SET(sock_fd,&allreads);

for(;;)
{
	readmask = allreads;
	int rc = select(sock_fd+1,&allreads,NULL,NULL,&tv);
	if(rc < 0){
		errExit("select failed\n");
	}
	
	if(rc == 0){
		printf("read timeout \n");
		onClientTimeout(sock_fd);
	}
	...
}
```

# 缓冲区处理

一个设计良好的网络程序，应该可以在随机输入的情况下表现稳定。除此之外，编写的网络程序能不能在黑客的刻意攻击之下表现稳定，也是一个重要考量因素。

下面几个实例程序说明可能出现的漏洞。

## 第一个例子

```
char Response[] = "COMMAND OK";
char buf[128];

while(1)
{
	int nbytes = recv(connfd,buffer,sizeof(buffer),0);
	if(nbytes == -1){
		errExit("error read message\n");
	}else if(nbytes == 0){
		errExit("client close\n");
	}
	
	buffer[nbytes] = '\0';
	if(strcmp(buffer,"quit") == 0){
		printf("client quit");
		send(connfd,Response,sizeof(Response),0);
	}
	
	printf("received %d bytes:%s",nbytes,buffer);
}
```

这段代码从连接套接字中获取字节流，并且判断了出错和 `EOF` 情况，如果对端发送来的字符是 "quit" 就回应 "COMAAND OK" 的字符流，乍看上去一切正常。

但仔细看一下，这段代码很有可能会产生下面的结果：

```
char buffer[128];
buffer[128] = '\0';
```

通过 `recv()` 读取的字符数为128时，就会出现这样的情况。因为 `buffer` 的大小只有 128 字节，最后的赋值环节，产生了缓冲区溢出的问题。

所以应该修改为：

```
int nbytes = recv(connfd,buffer,sizeof(buffer)-1,0);
```

`Response` 字符串中的 `'\0'` 是被发送出去的，而我们在接收字符时，则假设没有`'\0'`字符的存在：

```
send(socket, Response, strlen(Response), 0);
```

## 第二个例子

对变长报文解析的两种手段，一个是使用特殊的边界符号，例如HTTP使用的回车换行符；另一个是将报文信息的长度编码进入消息。在实战中，我们也需要对这部分报文长度保持警惕。

```
size_t read_message(int fd,char* buffer,size_t length)
{
	u_int32_t msg_length;
	u_int32_t msg_type;
	int rc;
		
	rc = readn(fd,(char*)&msg_length,sizeof(u_int32_t));
	if(rc != sizeof(u_int32_t))
		return rc < 0 ? -1 : 0;
	
	msg_length = ntohl(msg_length);
	
	rc = readn(fd,(char*)&msg_type,sizeof(msg_type));
	if(rc != sizeof(u_int32_t))
		return rc < 0 ? -1 : 0;
	
	/// 对比 msg_length 和缓冲区的大小，防止缓冲区溢出，还可以防止 msg_length 错误的设置过大，而实际发送很少的字节数导致 read() 阻塞
	if(msg_length > length)
		return -1;
	
	rc = readn(fd,buffer,msg_length);
	if(rc != msg_length)
		return rc < 0 ? -1 : 0;
	return rc;
}
```

## 第三个例子

假设报文的分界符是换行符 `\n`，一个简单的想法是每次读取一个字符，判断这个字符是不是换行符。

```
size_t read_line(int fd,char* buffer,size_t length)
{
	char* begin = buffer;
	char c;
	
	while(length > 0 && recv(fd,&c,1,0) == 1){
		*buffer ++ = c;
		length--;
		
		if(c == '\n'){
			*buffer = '\0';
			return buffer - begin;
		}
	}
	
	return -1;
}
```

这个函数的最大问题是工作效率太低，要知道每次调用 `recv()` 函数都是一次系统调用，需要从用户空间切换到内核空间，上下文切换的开销对于高性能来说最好是能省则省。

```
size_t read_line(int fd,char* buffer,size_t length)
{
	char* begin = buffer;
	static char* buffer_poiner;
	int nleft = 0;
	static char read_buffer[512];
	char c;
	
	while(length-- > 0){
		if(nleft <= 0){
			int nread = recv(fd,read_buffer,sizeof(read_buffer),0);
			if(nread < 0){
				if(errno == EINTR){
					length++;
					continue;
				}
				return -1;
			}
			
			if(nread == 0)
				return 0;
			
			buffer_poiner = read_buffer;
			nleft = nread;
		}		
		
		c = *buffer_poiner;
		*buffer++ = c;
		nleft--;
		if(c == '\n'){
			*buffer = '\0';
			return buffer - begin;
		}
	}
	
	return -1;
}
```

这个函数一次性读取最多 512 字节到临时缓冲区，之后将临时缓冲区的字符一个一个拷贝到应用程序最终的缓冲区中，这样的做法明显效率会高很多。

这个程序运行起来可能很久都没有问题，但是，它还是有一个微小的瑕疵，这个瑕疵很可能会造成线上故障。假设输入的字符是 `012345678\n`。

当读到最后一个 `\n` 字符时，`length` 为1，如果读到了换行符，就会增加一个字符串截止符，这显然越过了应用程序缓冲区的大小。这里最关键的是需要先对`length` 进行处理，再去判断 `length` 的大小是否可以容纳下字符：

```
size_t read_line(int fd,char* buffer,size_t length)
{
	char* begin = buffer;
	static char* buffer_poiner;
	int nleft = 0;
	static char read_buffer[512];
	char c;
	
	/// 先对 length 处理
	while(--length > 0){
		if(nleft <= 0){
			int nread = recv(fd,read_buffer,sizeof(read_buffer),0);
			if(nread < 0){
				if(errno == EINTR){
					length++;
					continue;
				}
				return -1;
			}
			
			if(nread == 0)
				return 0;
			
			buffer_poiner = read_buffer;
			nleft = nread;
		}		
		
		c = *buffer_poiner;
		*buffer++ = c;
		nleft--;
		if(c == '\n'){
			*buffer = '\0';
			return buffer - begin;
		}
	}
	
	return -1;
}
```











