# epoll 的性能分析

事件集合：

在每次使用 `poll()` 或 `select()` 之前，都需要准备一个感兴趣的事件集合，系统内核拿到事件集合，进行分析并在内核空间构建相应的数据结构来完成对事件集合的注册。而 `epoll()` ` 则不是这样，`epoll()` 维护了一个全局的事件集合，通过 `epoll()` 句柄，可以操纵这个事件集合，增加、删除或修改这个事件集合里的某个元素。要知道在绝大多数情况下，事件集合的变化没有那么的大，这样操纵系统内核就不需要每次重新扫描事件集合，构建内核空间数据结构。

就绪列表：

每次在使用 `poll()` 或者 `select()` 之后，应用程序都需要扫描整个感兴趣的事件集合，从中找出真正活动的事件，这个列表如果增长到 10K 以上，每次扫描的时间损耗也是惊人的。事实上，很多情况下扫描完一圈，可能发现只有几个真正活动的事件。而 `epoll()` 则不是这样，`epoll()` 返回的直接就是活动的事件列表，应用程序减少了大量的扫描时间。

边缘触发：

`epoll()` 还提供了更高级的能力。

如果某个套接字有100个字节可以读，边缘触发（edge-triggered）和条件触发（level-triggered）都会产生 read ready notification 事件，如果应用程序只读取了50个字节，边缘触发就会陷入等待；而条件触发则会因为还有50个字节没有读取完，不断地产生 read ready notification 事件。

在条件触发下，如果某个套接字缓冲区可以写，会无限次返回write ready notification事件，在这种情况下，如果应用程序没有准备好，不需要发送数据，一定需要解除套接字上的 ready notification 事件，否则CPU就直接跪了。

# 基本数据结构

##　eventpoll

`eventpoll` 是在调用 `epoll_create()` 之后内核侧创建的一个句柄，表示了一个 `epoll` 实例。后续如果我们再调用 `epoll_ctl()` 和 `epoll_wait()`等，都是对这个 `eventpoll` 数据进行操作，这部分数据会被保存在 `epoll_create` 创建的匿名文件 `file` 的 `private_data` 字段中。

```
/*
 * This structure is stored inside the "private_data" member of the file
 * structure and represents the main data structure for the eventpoll
 * interface.
 */
struct eventpoll {
    /* Protect the access to this structure */
    spinlock_t lock;

    /*
     * This mutex is used to ensure that files are not removed
     * while epoll is using them. This is held during the event
     * collection loop, the file cleanup path, the epoll file exit
     * code and the ctl operations.
     */
    struct mutex mtx;

    /* Wait queue used by sys_epoll_wait() */
    //这个队列里存放的是执行epoll_wait从而等待的进程队列
    wait_queue_head_t wq;

    /* Wait queue used by file->poll() */
    //这个队列里存放的是该eventloop作为poll对象的一个实例，加入到等待的队列
    //这是因为eventpoll本身也是一个file, 所以也会有poll操作
    wait_queue_head_t poll_wait;

    /* List of ready file descriptors */
    //这里存放的是事件就绪的fd列表，链表的每个元素是下面的epitem
    struct list_head rdllist;

    /* RB tree root used to store monitored fd structs */
    //这是用来快速查找fd的红黑树
    struct rb_root_cached rbr;

    /*
     * This is a single linked list that chains all the "struct epitem" that
     * happened while transferring ready events to userspace w/out
     * holding ->lock.
     */
    struct epitem *ovflist;

    /* wakeup_source used when ep_scan_ready_list is running */
    struct wakeup_source *ws;

    /* The user that created the eventpoll descriptor */
    struct user_struct *user;

    //这是eventloop对应的匿名文件，充分体现了Linux下一切皆文件的思想
    struct file *file;

    /* used to optimize loop detection check */
    int visited;
    struct list_head visited_list_link;

#ifdef CONFIG_NET_RX_BUSY_POLL
    /* used to track busy poll napi_id */
    unsigned int napi_id;
#endif
};
```

## epitem

调用 `epoll_ctl()` 增加一个 `fd` 时，内核就会为我们创建出一个 `epitem` 实例，并且把这个实例作为红黑树的一个子节点，增加到 `eventpoll` 结构体中的红黑树中，对应的字段是 `rbr`。这之后，查找每一个 `fd` 上是否有事件发生都是通过红黑树上的 `epitem` 来操作。

```
/*
 * Each file descriptor added to the eventpoll interface will
 * have an entry of this type linked to the "rbr" RB tree.
 * Avoid increasing the size of this struct, there can be many thousands
 * of these on a server and we do not want this to take another cache line.
 */
struct epitem {
    union {
        /* RB tree node links this structure to the eventpoll RB tree */
        struct rb_node rbn;
        /* Used to free the struct epitem */
        struct rcu_head rcu;
    };

    /* List header used to link this structure to the eventpoll ready list */
    //将这个epitem连接到eventpoll 里面的rdllist的list指针
    struct list_head rdllink;

    /*
     * Works together "struct eventpoll"->ovflist in keeping the
     * single linked chain of items.
     */
    struct epitem *next;

    /* The file descriptor information this item refers to */
    //epoll监听的fd
    struct epoll_filefd ffd;

    /* Number of active wait queue attached to poll operations */
    //一个文件可以被多个epoll实例所监听，这里就记录了当前文件被监听的次数
    int nwait;

    /* List containing poll wait queues */
    struct list_head pwqlist;

    /* The "container" of this item */
    //当前epollitem所属的eventpoll
    struct eventpoll *ep;

    /* List header used to link this item to the "struct file" items list */
    struct list_head fllink;

    /* wakeup_source used when EPOLLWAKEUP is set */
    struct wakeup_source __rcu *ws;

    /* The structure that describe the interested events and the source fd */
    struct epoll_event event;
};
```

## eppoll_entry

每次当一个 `fd` 关联到一个 `epoll` 实例，就会有一个 `eppoll_entry` 产生。

```
/* Wait structure used by the poll hooks */
struct eppoll_entry {
    /* List header used to link this structure to the "struct epitem" */
    struct list_head llink;

    /* The "base" pointer is set to the container "struct epitem" */
    struct epitem *base;

    /*
     * Wait queue item that will be linked to the target file wait
     * queue head.
     */
    wait_queue_entry_t wait;

    /* The wait queue head that linked the "wait" wait queue item */
    wait_queue_head_t *whead;
};s
```

# 操作

## epoll_create

在使用 `epoll` 的时候，首先会调用 `epoll_create()` 来创建一个 `epoll` 实例。

`epoll_create()` 会对传入的 `flags` 参数做简单的验证：

```
/* Check the EPOLL_* constant for consistency.  */
BUILD_BUG_ON(EPOLL_CLOEXEC != O_CLOEXEC);

if (flags & ~EPOLL_CLOEXEC)
    return -EINVAL;
```

接下来，内核申请分配 `eventpoll` 需要的内存空间：

```
/* Create the internal data structure ("struct eventpoll").
*/
error = ep_alloc(&ep);
if (error < 0)
  return error;
```

在接下来，`epoll_create()` 为 `epoll` 实例分配了匿名文件和文件描述字，其中 `fd` 是文件描述字，`file` 是一个匿名文件。这里充分体现了 UNIX 下一切都是文件的思想。注意，`eventpoll` 的实例会保存一份匿名文件的引用，通过调用 `fd_install` 函数将匿名文件和文件描述字完成了绑定。

这里还有一个特别需要注意的地方，在调用 `anon_inode_get_file` 的时候，`epoll_create` 将 `eventpoll` 作为匿名文件 `file` 的 `private_data` 保存了起来，这样，在之后通过 `epoll` 实例的文件描述字来查找时，就可以快速地定位到 `eventpoll` 对象了。

最后，这个文件描述字作为 `epoll` 的文件句柄，被返回给 `epoll_create` 的调用者。

```
/*
 * Creates all the items needed to setup an eventpoll file. That is,
 * a file structure and a free file descriptor.
 */
fd = get_unused_fd_flags(O_RDWR | (flags & O_CLOEXEC));
if (fd < 0) {
    error = fd;
    goto out_free_ep;
}
file = anon_inode_getfile("[eventpoll]", &eventpoll_fops, ep, O_RDWR | (flags & O_CLOEXEC));
if (IS_ERR(file)) {
    error = PTR_ERR(file);
    goto out_free_fd;
}
ep->file = file;
fd_install(fd, file);
return fd;
```

## epoll_ctl

### 查找epoll实例

`epoll_ctl()` 函数通过 `epoll` 实例句柄来获得对应的匿名文件，这一点很好理解，UNIX下一切都是文件，`epoll` 的实例也是一个匿名文件。

```
//获得epoll实例对应的匿名文件
f = fdget(epfd);
if (!f.file)
    goto error_return;
```

接下来，获得添加的套接字对应的文件，这里 `tf` 表示的是 target file，即待处理的目标文件。

```
/* Get the "struct file *" for the target file */
//获得真正的文件，如监听套接字、读写套接字
tf = fdget(fd);
if (!tf.file)
    goto error_fput;
```

再接下来，进行了一系列的数据验证，以保证用户传入的参数是合法的，比如 `epfd` 真的是一个 `epoll` 实例句柄，而不是一个普通文件描述符。

```
/* The target file descriptor must support poll */
//如果不支持poll，那么该文件描述字是无效的
error = -EPERM;
if (!tf.file->f_op->poll)
    goto error_tgt_fput;
...
```

如果获得了一个真正的 `epoll` 实例句柄，就可以通过 `private_data` 获取之前创建的 `eventpoll` 实例了。

```
/*
 * At this point it is safe to assume that the "private_data" contains
 * our own data structure.
 */
ep = f.file->private_data;
```

### 红黑树查找

`epoll_ctl()` 通过目标文件和对应描述字，在红黑树中查找是否存在该套接字，这也是 `epoll` 为什么高效的地方。红黑树（RB-tree）是一种常见的数据结构，这里 `eventpoll` 通过红黑树跟踪了当前监听的所有文件描述字，而这棵树的根就保存在 `eventpoll` 数据结构中。

```
/* RB tree root used to store monitored fd structs */
struct rb_root_cached rbr;
```

对于每个被监听的文件描述字，都有一个对应的 `epitem` 与之对应，`epitem` 作为红黑树中的节点就保存在红黑树中。

```
/*
 * Try to lookup the file inside our RB tree, Since we grabbed "mtx"
 * above, we can be sure to be able to use the item looked up by
 * ep_find() till we release the mutex.
 */
epi = ep_find(ep, tf.file, fd);
```

红黑树是一棵二叉树，作为二叉树上的节点，`epitem` 必须提供比较能力，以便可以按大小顺序构建出一棵有序的二叉树。其排序能力是依靠 `epoll_filefd` 结构体来完成的，`epoll_filefd` 可以简单理解为需要监听的文件描述字，它对应到二叉树上的节点。

可以看到这个还是比较好理解的，按照文件的地址大小排序。如果两个相同，就按照文件文件描述字来排序。

```
struct epoll_filefd {
	struct file *file; // pointer to the target file struct corresponding to the fd
	int fd; // target file descriptor number
} __packed;

/* Compare RB tree keys */
static inline int ep_cmp_ffd(struct epoll_filefd *p1,
                            struct epoll_filefd *p2)
{
	return (p1->file > p2->file ? +1:
		   (p1->file < p2->file ? -1 : p1->fd - p2->fd));
}
```

在进行完红黑树查找之后，如果发现是一个 ADD 操作，并且在树中没有找到对应的二叉树节点，就会调用 `ep_insert` 进行二叉树节点的增加。

```
case EPOLL_CTL_ADD:
    if (!epi) {
        epds.events |= POLLERR | POLLHUP;
        error = ep_insert(ep, &epds, tf.file, fd, full_check);
    } else
        error = -EEXIST;
    if (full_check)
        clear_tfile_check_list();
    break;
```



























