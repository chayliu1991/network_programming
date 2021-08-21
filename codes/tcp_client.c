#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#define MESSAGE_SIZE (10240000)
#define PORT_NUM (50005)

void send_data(int sockfd)
{
  char *query;
  query = malloc(MESSAGE_SIZE + 1);
  for (int i = 0; i < MESSAGE_SIZE; i++)
  {
    query[i] = 'a';
  }
  query[MESSAGE_SIZE] = '\0';

  const char *cp;
  cp = query;
  size_t remaining = strlen(query);
  while (remaining)
  {
    int n_written = send(sockfd, cp, remaining, 0);
    fprintf(stdout, "send into buffer %d \n", n_written);
    if (n_written <= 0)
    {
      fprintf(stderr, "send fail:%s\n", strerror(errno));
      return;
    }
    remaining -= n_written;
    cp += n_written;
  }

  return;
}

int main(int argc, char **argv)
{
  if (argc < 2 || strcmp(argv[1], "--help") == 0)
  {
    fprintf(stderr, "usage: %s tcpclient IPaddress \n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int sockfd;
  struct sockaddr_in servaddr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(PORT_NUM);
  inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
  int connect_rt = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
  if (connect_rt < 0)
  {
    fprintf(stderr, "connect to server error: %s \n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  send_data(sockfd);
  printf("main exit\n");
  exit(0);
}