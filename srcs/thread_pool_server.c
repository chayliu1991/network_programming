#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define SERV_PORT (50005)
#define MAX_LINE (4096)
#define THREAD_NUMBER (4)
#define BLOCK_QUEUE_SIZE (100)

typedef struct
{
    pthread_t thread_tid; /* thread ID */
    long thread_count;    /* # connections handled */
} Thread;
Thread *thread_array;

//@ 定义一个队列
typedef struct
{
    int numer;             //@ 队列中的描述字最大个数
    int *fd;               //@ 这是一个数组指针
    int front;             //@ 当前队列的头位置
    int rear;              //@ 当前队列的尾位置
    pthread_mutex_t mutex; //@ 锁
    pthread_cond_t cond;   //@ 条件变量
} blockQueue;

//@ 初始化队列
void block_queue_init(blockQueue *block_queue, int numer)
{
    block_queue->numer = numer;
    block_queue->fd = calloc(numer, sizeof(int));
    block_queue->front = block_queue->rear = 0;
    pthread_mutex_init(&block_queue->mutex, NULL);
    pthread_cond_init(&block_queue->cond, NULL);
}

//@ 往队列里放置一个描述字fd
void block_queue_push(blockQueue *block_queue, int fd)
{
    pthread_mutex_lock(&block_queue->mutex);
    block_queue->fd[block_queue->rear] = fd;

    //@ 如果已经到最后，重置尾的位置
    if (++block_queue->rear == block_queue->numer)
    {
        block_queue->rear = 0;
    }
    printf("push fd %d to the queue\n", fd);

    //@ 通知其他等待读的线程，有新的连接字等待处理
    pthread_cond_signal(&block_queue->cond);
    pthread_mutex_unlock(&block_queue->mutex);
}

//@ 从队列里读出描述字进行处理
int block_queue_pop(blockQueue *block_queue)
{
    pthread_mutex_lock(&block_queue->mutex);

    //@ 判断队列里没有新的连接字可以处理，就一直条件等待，直到有新的连接字入队列
    while (block_queue->front == block_queue->rear)
        pthread_cond_wait(&block_queue->cond, &block_queue->mutex);

    //@ 取出队列头的连接字
    int fd = block_queue->fd[block_queue->front];

    //@ 如果已经到最后，重置头的位置
    if (++block_queue->front == block_queue->numer)
    {
        block_queue->front = 0;
    }

    printf("pop fd %d from the queue\n", fd);
    pthread_mutex_unlock(&block_queue->mutex);
    return fd;
}

char rot13_char(char c)
{
    if ((c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'M'))
        return c + 13;
    else if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z'))
        return c - 13;
    else
        return c;
}

void loop_echo(int fd)
{
    char outbuf[MAX_LINE + 1];
    size_t outbuf_used = 0;
    ssize_t result;
    while (1)
    {
        char ch;
        result = recv(fd, &ch, 1, 0);

        //@ 断开连接或者出错
        if (result == 0)
        {
            break;
        }
        else if (result == -1)
        {
            fprintf(stderr, "recv() error:%s\n", strerror(errno));
            break;
        }

        if (outbuf_used < sizeof(outbuf))
        {
            outbuf[outbuf_used++] = rot13_char(ch);
        }

        if (ch == '\n')
        {
            send(fd, outbuf, outbuf_used, 0);
            outbuf_used = 0;
            continue;
        }
    }
}

void thread_run(void *arg)
{
    pthread_t tid = pthread_self();
    pthread_detach(tid);

    blockQueue *bq = (blockQueue *)arg;
    while (1)
    {
        int fd = block_queue_pop(bq);
        printf("get fd in thread, fd==%d, tid == %lu\n", fd, tid);
        loop_echo(fd);
    }
}

int tcp_server_listen(int port)
{
    int listenfd;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    int on = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    int rt1 = bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (rt1 < 0)
    {
        fprintf(stderr, "bind() failed:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int rt2 = listen(listenfd, 1024);
    if (rt2 < 0)
    {
        fprintf(stderr, "listen() failed:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    signal(SIGPIPE, SIG_IGN);

    return listenfd;
}

int main(int c, char **v)
{
    int listener_fd = tcp_server_listen(SERV_PORT);

    blockQueue block_queue;
    block_queue_init(&block_queue, BLOCK_QUEUE_SIZE);

    thread_array = calloc(THREAD_NUMBER, sizeof(Thread));
    int i;
    for (i = 0; i < THREAD_NUMBER; i++)
    {
        pthread_create(&(thread_array[i].thread_tid), NULL, &thread_run, (void *)&block_queue);
    }

    while (1)
    {
        struct sockaddr_storage ss;
        socklen_t slen = sizeof(ss);
        int conn_fd = accept(listener_fd, (struct sockaddr *)&ss, &slen);
        if (conn_fd < 0)
        {
            fprintf(stderr, "accept() failed:%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        else
        {
            block_queue_push(&block_queue, conn_fd);
        }
    }

    return 0;
}