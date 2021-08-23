#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define MAX_LINE 4096
#define SERV_PORT (50005)

static int s_count;

static void sig_int(int signo) {
  printf("\nreceived %d datagrams\n", s_count);
  exit(0);
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
    fprintf(stderr, "bind() error:%s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  int rt2 = listen(listenfd, 1024);
  if (rt2 < 0) {
    fprintf(stderr, "listen() error:%s\n", strerror(errno));
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

  char message[MAX_LINE];
  s_count = 0;

  for (;;) {
    int n = read(connfd, message, MAX_LINE);
    if (n < 0) {
      fprintf(stderr, "read() error:%s\n", strerror(errno));
      exit(EXIT_FAILURE);
    } else if (n == 0) {
      fprintf(stderr, "client closed\n");
      exit(EXIT_FAILURE);
    }

    message[n] = 0;
    printf("received %d bytes: %s\n", n, message);
    s_count++;
  }
}