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

#define MESSAGE_SIZE 102400
#define SERV_PORT 12345

int tcp_client(char *address, int port) {
  int socket_fd;
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in server_addr;
  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  inet_pton(AF_INET, address, &server_addr.sin_addr);

  socklen_t server_len = sizeof(server_addr);
  int connect_rt =
      connect(socket_fd, (struct sockaddr *)&server_addr, server_len);
  if (connect_rt < 0) {
    fprintf(stderr, "connect() failed:%s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  return socket_fd;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    error(1, 0, "usage: reliable_client02 <IPaddress>");
  }

  int socket_fd = tcp_client(argv[1], SERV_PORT);

  signal(SIGPIPE, SIG_IGN);

  char *msg = "network programming";
  ssize_t n_written;

  int count = 10000000;
  while (count > 0) {
    n_written = send(socket_fd, msg, strlen(msg), 0);
    fprintf(stdout, "send into buffer %ld \n", n_written);
    if (n_written <= 0) {
      fprintf(stderr, "send() failed:%s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
    count--;
  }
  return 0;
}
