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

static void recvfrom_int(int signo) {
  printf("\nreceived %d datagrams\n", s_count);
  exit(0);
}

int main(int argc, char **argv) {
  int socket_fd;
  socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

  struct sockaddr_in server_addr;
  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(SERV_PORT);

  bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

  socklen_t client_len;
  char message[MAX_LINE];
  message[0] = 0;
  s_count = 0;

  signal(SIGINT, recvfrom_int);

  struct sockaddr_in client_addr;
  client_len = sizeof(client_addr);

  int n = recvfrom(socket_fd, message, MAX_LINE, 0,
                   (struct sockaddr *)&client_addr, &client_len);
  if (n < 0) {
    fprintf(stderr, "recvfrom() error:%s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  message[n] = 0;
  printf("received %d bytes: %s\n", n, message);

  if (connect(socket_fd, (struct sockaddr *)&client_addr, client_len)) {
    fprintf(stderr, "connect() error:%s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  while (strncmp(message, "goodbye", 7) != 0) {
    char send_line[MAX_LINE];
    sprintf(send_line, "Hi, %s", message);

    size_t rt = send(socket_fd, send_line, strlen(send_line), 0);
    if (rt < 0) {
      fprintf(stderr, "send() error:%s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
    printf("send bytes: %zu \n", rt);

    size_t rc = recv(socket_fd, message, MAX_LINE, 0);
    if (rc < 0) {
      fprintf(stderr, "recv() error:%s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }

    s_count++;
  }

  exit(0);
}
