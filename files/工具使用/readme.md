# ping

`ping` 在网络上用来完成对网络连通性的探测。

`ping` 是基于一种叫做 ICMP 的协议开发的，ICMP 又是一种基于IP协议的控制协议，翻译为网际控制协议。

# ifconfig

`ifconfig` 命令被用于配置和显示 Linux 内核中网络接口的网络参数。用 `ifconfig` 命令配置的网卡信息，在网卡重启后机器重启后，配置就不存在。要想将上述的配置信息永远的存的电脑里，那就要修改网卡的配置文件了。

常用命令选项：

- `up`：启动指定的网络设备
- `down`：关闭指定的网络设备
- `mtu <字节>`：设置网络设备的最大传输单元
- `netmask <子网掩码>`：设置网络设备的子网掩码
- `broadcast <广播地址>`：设置网络设备的广播地址

**显示网络设备信息**

```
ifonfig

enp0s3    Link encap:Ethernet  HWaddr 02:54:ad:ea:60:2e
          inet addr:10.0.2.15  Bcast:10.0.2.255  Mask:255.255.255.0
          inet6 addr: fe80::54:adff:feea:602e/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:7951 errors:0 dropped:0 overruns:0 frame:0
          TX packets:4123 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000
          RX bytes:5081047 (5.0 MB)  TX bytes:385600 (385.6 KB)
```

释义：

```
Link encap:Ethernet  HWaddr 02:54:ad:ea:60:2e
```

表明这是一个以太网设备，MAC 地址为 `02:54:ad:ea:60:2e`。

```
inet 10.0.2.15  netmask 255.255.255.0  broadcast 10.0.2.255
inet6 fe80::e096:3a76:6df1:bd6d  prefixlen 64  scopeid 0x20<link>
```

显示的是网卡的 IPv4 和 IPv6 地址，其中 IPv4 还显示了该网络的子网掩码以及广播地址。

```
UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
```

`UP` 表示网卡处在开启状态，`RUNNING` 表示网卡的网线被接上，`MULTICAST` 表示支持组播，`MTU:1500` 表示最大传输单元1500字节。

Linux 在一台主机上可以有多个网卡设备，很可能有这么一种情况，多个网卡可以路由到目的地。一个简单的例子是在同时有无线网卡和有线网卡的情况下，网络连接是从哪一个网卡设备上出去的？Metric 就是用来确定多块网卡的优先级的，数值越小，优先级越高，1为最高级。

```
RX packets:7951 errors:0 dropped:0 overruns:0 frame:0
TX packets:4123 errors:0 dropped:0 overruns:0 carrier:0
collisions:0 txqueuelen:1000
RX bytes:5081047 (5.0 MB)  TX bytes:385600 (385.6 KB)
```

`  RX packets [xx]/bytes [xx]` 表示接收数据包数量/字节数，` TX packets [xx]/bytes [xx]` 表示发送数据包数量/字节数。

 **启动关闭指定网卡**

```
ifconfig eth0 up    # 启动网卡eth0
ifconfig eth0 down  # 关闭网卡eth0
```

**启用和关闭 ARP 协议（地址解析协议)**

```
ifconfig eth0 arp    #开启网卡eth0 的arp协议
ifconfig eth0 -arp   #关闭网卡eth0 的arp协议
```

**配置IP地址、子网掩码、广播地址**

```
# 如果不加任何其他参数，则系统会依照该 IP 所在的 class 范围，自动的计算出 netmask 以及 network, broadcast 等 IP 参数；
ifconfig eth0 192.168.2.10


ifconfig eth0 192.168.2.10 netmask 255.255.255.0
ifconfig eth0 192.168.2.10 netmask 255.255.255.0 broadcast 192.168.2.255
```

**设置最大传输单元**

```
ifconfig eth0 mtu 1500    #设置能通过的最大数据包大小为 1500 bytes
```

**放弃 `ifconfig` 的全部修改，以 `ifcfg-eth\*` 的配置文件重置网络设置**

```
/etc/init.d/network restart
```

# route

`route` 主要用于显示并设置 Linux 内核中的网络路由表，`route` 命令设置的路由主要是静态路由。

直接在命令行下执行 `route` 命令来添加路由，不会永久保存，当网卡重启或者机器重启之后，该路由就失效了；可以在 `/etc/rc.local` 中添加 `route` 命令来保证该路由设置永久有效。

要实现两个不同的子网之间的通信，需要一台连接两个网络的路由器，或者同时位于两个网络的网关来实现。在一台服务器里，连接内网的网卡是不能进行设置。

常用选项：

- `-n`：不使用通信协议或主机名，直接显示数字形式的IP地址和端口号
- `-net`：到一个网络的路由表
- `-host`：到一个主机的路由表

常用参数：

- `add`：增加指定的路由记录
- `del`：删除指定的路由记录
- `gw`：设置默认网关

**显示当前路由列表**

```
route -n # 类似于 netstat -r

Kernel IP routing table
Destination     Gateway         Genmask         Flags Metric Ref    Use Iface
0.0.0.0         10.0.2.2        0.0.0.0         UG    102    0        0 eth0
10.0.2.0        0.0.0.0         255.255.255.0   U     102    0        0 eth0
192.168.33.0    0.0.0.0         255.255.255.0   U     103    0        0 eth1
```

- `Destination`，`Genmask`：分别是 IP 与 子网掩码，它们组合成为一个完整的网域
- `Gateway`：该网域是通过哪个网关连接出去的。如果显示 `0.0.0.0` 表示该路由是直接由本机传送，也就是可以通过局域网的 MAC 直接发送；如果有显示 IP 的话，表示该路由需要经过路由器 (网关) 的帮忙才能够传送出去
- `Flags` 为路由标志，标记当前网络节点的状态：
  - `U`：`Up` 表示此路由当前为启动状态，`!` 表示此路由当前为关闭状态
  - `H`：`Host`，表示此网关为一主机
  - `G`：`Gateway`，表示此网关为一路由器
  - `R`：`Reinstate Route`，使用动态路由重新初始化的路
  - `D`：`Dynamically`，此路由是动态性地写入
  - `M`：`Modified`，此路由是由路由守护程序或导向器动态修改

**删除和添加设置默认网关**

```
route del default gw 192.168.0.1
route add default gw 192.168.0.2
```

# netstat 

`netstat` 程序可以显示系统中 Internet 和 UNIX 域套接字的状态。

常用的命令选项：

- `-t`：列出TCP协议的端口
- `-u`：列出UDP协议的端口
- `-n`：不使用域名与服务名，而使用 IP 地址和端口号
- `-l`：仅列出在监听状态的网络服务
- `-a`：列出所有的网络连接
- `-p`：显示正在使用 socket 的程序识别码和程序名称
- `-r`：显示路由表
- `-c`：连续重新显示套接字信息，每秒刷新显示一次
- `--inet`：显示 Internet 域套接字的信息
- `--unix`：显示 UNIX 域套接字的信息

对于每个 Internet 域套接字，可以看到如下信息：

```
Proto Recv-Q Send-Q Local Address           Foreign Address         State       User       Inode       PID/Program name
```

- `Proto`：套接字使用的协议类型：tcp, udp, raw
- `Recv-Q`：表示套接字接收缓冲区中还没有被本地应用读取的字节数
- `Send-Q`：表示套接字发送缓冲区中排队等待发送的字节数
- `Local Address`：表示套接字绑定到的地址，以 `主机名:端口号` 的形式表示，`*` 表示这是一个通配 IP 地址
- `Foreign Address`：对端套接字所绑定的地址，`：`  表示没有对端地址
- `State`：表示当前套接字所处的状态：
  - `ESTABLISHED`
  - `SYN_SENT`
  - `SYN_RECV`
  - `FIN_WAIT1`
  - `FIN_WAIT2`
  - `TIME_WAIT`
  - `CLOSE_WAIT`
  - `LAST_ACK`
  - `CLOSING`
  - `UNKNOWN`
- `User`：socket 的属主用户名
- `PID/Program name`：拥有 socket 的进程 ID 和进程名称

对于本地域套接字，可以看到如下信息：

```
Proto RefCnt Flags       Type       State         I-Node   PID/Program name    Path
```

- `Proto`：套接字使用的协议类型：unix
- `RefCnt` ：引用次数
- `Flags`：`SO_ACCEPTON`(`ACC`)，`SO_WAITDATA`(`W`)，`SO_NOSPACE`(`N`)，`SO_ACCEPTON` 表示等待连接
- `Type`：socket 类型：
  - `SOCK_DGRAM`
  - `SOCK_STREAM`
  - `SOCK_RAW`
  - `SOCK_RDM`
  - `SOCK_SEQPACKET`
  - `SOCK_PACKET`
  - `UNKNOWN`
- `State` ：表示当前套接字所处的状态：
  - `LISTENING`
  - `CONNECTING`
  - `CONNECTED`
  - `DISCONNECTING`
  - `empty`
  - `UNKNOWN`
- `PID/Program name`：拥有 socket 的进程 ID 和进程名称
- `Path` ：绑定的 socket 文件

**找出程序运行的端口**

```
netstat -ap | grep ssh
```

**找出运行在指定端口的进程**

```
netstat -antp | grep "22"
```

**显示所有的网卡列表**

```
netstat -i
netstat -ie # 显示详细信息，等同于 ifconfig
```

**显示所有路由表信息**

```
netstat -r
```

# ss

显示处于活动状态的 socket 信息。`ss` 命令可以用来获取 socket 统计信息，它可以显示和 `netstat` 类似的内容。但 `ss` 的优势在于它能够显示更多更详细的有关 TCP 和连接状态的信息，而且比 `netstat` 更快速更高效。

常用选项：

- `-t`：列出 TCP 协议的 socket
- `-u`：列出 UDP 协议的 socket
- `-l`：仅列出在监听状态的 socket
- `-a`：列出所有的 socket
- `-p`：显示正在使用 socket 的进程信息

# tcpdump

`tcpdump` 命令用于倾倒网络传输数据。执行 `tcpdump` 指令可列出经过指定网络界面的数据包文件头。

`tcpdump` 在开启抓包的时候，会自动创建一个类型为 `AF_PACKET` 的网络套接口，并向系统内核注册。当网卡接收到一个网络报文之后，它会遍历系统中所有已经被注册的网络协议，包括其中已经注册了的 `AF_PACKET` 网络协议。系统内核接下来就会将网卡收到的报文发送给该协议的回调函数进行一次处理，回调函数可以把接收到的报文完完整整地复制一份，假装是自己接收到的报文，然后交给 `tcpdump` 程序，进行各种条件的过滤和判断，再对报文进行解析输出。

常用参数：

- `-a`：尝试将网络和广播地址转换成名称
- `-c<数据包数目>`：收到指定的数据包数目后，就停止进行倾倒操作
- `-d` ：把编译过的数据包编码转换成可阅读的格式，并倾倒到标准输出
- `-dd` ：把编译过的数据包编码转换成C语言的格式，并倾倒到标准输出
- `-ddd` ：把编译过的数据包编码转换成十进制数字的格式，并倾倒到标准输出
- `-e` ：在每列倾倒资料上显示连接层级的文件头
- `-f` ：用数字显示网际网络地址
- `-F<表达文件>` ：指定内含表达方式的文件
- `-i<网络接口>` ：使用指定的网络接口送出数据包
- `-l` ：使用标准输出列的缓冲区
- `-n` ：不把主机的网络地址转换成名字
- `-N` ：不列出域名
- `-O` ：不将数据包编码最佳化
- `-p` ：不让网络界面进入混杂模式
- `-q` ：快速输出，仅列出少数的传输协议信息
- `-r<数据包文件>` ：从指定的文件读取数据包数据
- `-s<数据包大小>` ：设置每个数据包的大小
- `-S` ：用绝对而非相对数值列出TCP关联数
- `-t` ：在每列倾倒资料上不显示时间戳记
- `-tt`： 在每列倾倒资料上显示未经格式化的时间戳记
- `-T<数据包类型>` ：强制将表达方式所指定的数据包转译成设置的数据包类型
- `-v` ：详细显示指令执行过程
- `-vv` ：更详细显示指令执行过程
- `-x` ：用十六进制字码列出数据包资料
- `-w<数据包文件>`： 把数据包数据写入指定的文件

## 监视指定网络接口的数据包

```
tcpdump -i eth1
```

如果不指定网卡，默认 `tcpdump` 只会监视第一个网络接口，一般是 `eth0`。

指定 ip,例如截获所有 `210.27.48.1` 的主机收到的和发出的所有的数据包：

```
tcpdump host 210.27.48.1 
```

打印 `helios` 与 `hot` 或者与 `ace` 之间通信的数据包：

```
tcpdump host helios and \(hot or ace \)
```

截获主机 `210.27.48.1` 和主机 `210.27.48.2` 或 `210.27.48.3` 的通信：

```
tcpdump host 210.27.48.1 and \(210.27.48.2 or 210.27.48.3\)
```

截获主机 `hostname` 发送的所有数据：

```
tcpdump -i eth0 src host hostname
```

监视所有送到主机 `hostname` 的数据包：

```
tcpdump -i eth0 dst host hostname
```

## 监视指定主机和端口的数据包

如果想要获取主机 `210.27.48.1` 接收或发出的 `telnet` 包，使用如下命令：

```
tcpdump tcp port 23 and host 210.27.48.1
```

对本机的 `udp 123` 端口进行监视 `123` 为 `ntp` 的服务端口：

```
tcpdump udp port 123 
```

## 监视指定协议的数据包

打印 TCP 会话中的的开始和结束数据包，并且数据包的源或目的不是本地网络上的主机：

```
tcpdump 'tcp[tcpflags] & (tcp-syn|tcp-fin) != 0 and not src and dst net localnet'
```

打印所有源或目的端口是80， 网络层协议为 IPv4，并且含有数据，而不是`SYN`，`FIN` 以及 `ACK-only` 等不含数据的数据包：

```
tcpdump 'tcp port 80 and (((ip[2:2] - ((ip[0]&0xf)<<2)) - ((tcp[12]&0xf0)>>2)) != 0)'
```

## tcpdump 与 Wireshark

Wireshark 是 Windows 下非常简单易用的抓包工具。可以在 Linux 里抓包，然后在Windows 里分析包：

```
tcpdump tcp -i eth1 -t -s 0 -c 100 and dst port ! 22 and src net 192.168.1.0/24 -w ./target.cap
```

- `tcp`，`ip `，`icmp `，`arp `，`rarp `，`udp` 这些选项等都要放到第一个参数的位置，用来过滤数据报的类型
- `-i eth1`：只抓经过接口 `eth1` 的包 
- `-t` ： 不显示时间戳
- `-s 0`： 抓取数据包时默认抓取长度为 68 字节。加上 `-s 0` 后可以抓到完整的数据包
- `-c 100`： 只抓取 100 个数据包
- `dst port ! 22`： 不抓取目标端口是 22 的数据包
- `src net 192.168.1.0/24`：数据包的源网络地址为 `192.168.1.0/24`
- `-w ./target.cap`：保存成cap文件，方便用 Wireshark分析



