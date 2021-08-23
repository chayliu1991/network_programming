#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define MESSAGE_SIZE 102400000
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
    printf("usage:%s <Ipaddress>\n", argv[0]);
    exit(EXIT_SUCCESS);
  }

  int socket_fd = tcp_client(argv[1], SERV_PORT);
  char buf[129];
  int len;
  int rc;

  while (fgets(buf, sizeof(buf), stdin) != NULL) {
    len = strlen(buf);
    rc = send(socket_fd, buf, len, 0);
    if (rc < 0) {
      fprintf(stderr, "send() failed:%s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
    sleep(3);
    rc = read(socket_fd, buf, sizeof(buf));
    if (rc < 0) {
      fprintf(stderr, "read() failed:%s\n", strerror(errno));
      exit(EXIT_FAILURE);
    } else if (rc == 0) {
      fprintf(stderr, "peer connection closed\n");
      exit(EXIT_FAILURE);
    } else
      fputs(buf, stdout);
  }
  exit(0);
}