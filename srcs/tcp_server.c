#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE (1024)
#define PORT_NUM (50005)

size_t readn(int fd, void *buffer, size_t size)
{
  char *buf = buffer;
  int length = size;

  while (length > 0)
  {
    int result = read(fd, buf, length);

    if (result < 0)
    {
      if (errno == EINTR)
        continue;
      else
        return -1;
    }
    else if (result == 0)
    {
      break;
    }

    length -= result;
    buf += result;
  }
  return (size - length);
}

void read_data(int sockfd)
{
  ssize_t n;
  char buf[BUF_SIZE];

  int time = 0;
  for (;;)
  {
    // fprintf(stdout, "block in read\n");
    if ((n = readn(sockfd, buf, BUF_SIZE)) == 0)
      return;

    time++;
    fprintf(stdout, "1K read for %d time\n", time);
    usleep(1000);
  }
}

int main(int argc, char **argv)
{
  int listenfd, connfd;
  socklen_t clilen;
  struct sockaddr_in cliaddr, servaddr;

  listenfd = socket(AF_INET, SOCK_STREAM, 0);

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(PORT_NUM);

  bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
  listen(listenfd, 1024);

  for (;;)
  {
    clilen = sizeof(cliaddr);
    connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
    read_data(connfd);
    close(connfd);
  }
}
