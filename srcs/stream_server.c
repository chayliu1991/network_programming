#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <unistd.h>

#define SERV_PORT (50005)

static int s_count;

static void sig_int(int signo) {
  printf("\nreceived %d datagrams\n", s_count);
  exit(0);
}

size_t readn(int fd, void *buffer, size_t length) {
  size_t count;
  ssize_t nread;
  char *ptr;

  ptr = buffer;
  count = length;
  while (count > 0) {
    nread = read(fd, ptr, count);

    if (nread < 0) {
      if (errno == EINTR)
        continue;
      else
        return (-1);
    } else if (nread == 0)
      break;

    count -= nread;
    ptr += nread;
  }
  return (length - count);
}

size_t read_message(int fd, char *buffer, size_t length) {
  u_int32_t msg_length;
  u_int32_t msg_type;
  int rc;

  rc = readn(fd, (char *)&msg_length, sizeof(u_int32_t));
  if (rc != sizeof(u_int32_t)) return rc < 0 ? -1 : 0;
  msg_length = ntohl(msg_length);

  rc = readn(fd, (char *)&msg_type, sizeof(msg_type));
  if (rc != sizeof(u_int32_t)) return rc < 0 ? -1 : 0;

  if (msg_length > length) {
    return -1;
  }

  rc = readn(fd, buffer, msg_length);
  if (rc != msg_length) return rc < 0 ? -1 : 0;
  return rc;
}

int main(int argc, char **argv) {
  int listenfd;
  listenfd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in server_addr;
  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(SERV_PORT);

  int on = 1;
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

  int rt1 =
      bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
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

  int connfd;
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);

  if ((connfd = accept(listenfd, (struct sockaddr *)&client_addr,
                       &client_len)) < 0) {
    fprintf(stderr, "accept() error:%s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  char buf[128];
  s_count = 0;

  while (1) {
    int n = read_message(connfd, buf, sizeof(buf));
    if (n < 0) {
      fprintf(stderr, "read_message() error\n");
      exit(EXIT_FAILURE);
    } else if (n == 0) {
      fprintf(stderr, "client closed\n");
      exit(EXIT_FAILURE);
    }
    buf[n] = 0;
    printf("received %d bytes: %s\n", n, buf);
    s_count++;
  }

  exit(0);
}