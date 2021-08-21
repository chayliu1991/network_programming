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
#define NDG 2000
#define DGLEN 1400

static int s_count;

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s <LocalPath> \n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int socket_fd;
  socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

  struct sockaddr_in server_addr;
  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SERV_PORT);
  inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

  socklen_t server_len = sizeof(server_addr);

  if (connect(socket_fd, (struct sockaddr *)&server_addr, server_len)) {
    fprintf(stderr, "connect() error:%s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  char send_line[MAX_LINE], recv_line[MAX_LINE + 1];
  int n;

  while (fgets(send_line, MAX_LINE, stdin) != NULL) {
    int i = strlen(send_line);
    if (send_line[i - 1] == '\n') {
      send_line[i - 1] = 0;
    }

    printf("now sending %s\n", send_line);
    size_t rt = send(socket_fd, send_line, strlen(send_line), 0);
    if (rt < 0) {
      fprintf(stderr, "send() error:%s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
    printf("send bytes: %zu \n", rt);

    recv_line[0] = 0;
    n = recv(socket_fd, recv_line, MAX_LINE, 0);
    if (n < 0) {
      fprintf(stderr, "recv() error:%s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
    recv_line[n] = 0;
    fputs(recv_line, stdout);
    fputs("\n", stdout);
  }

  exit(0);
}