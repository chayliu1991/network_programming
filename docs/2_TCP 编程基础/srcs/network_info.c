#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>

extern int h_errno;

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("usage: %s <Hostname>\n", *argv);
        return -1;
    }

    char *name = argv[1];
    struct hostent *hptr;

    hptr = gethostbyname(name);
    if (hptr == NULL)
    {
        printf("gethostbyname error for host: %s: %s\n", name, hstrerror(h_errno));
        return -1;
    }

    //@ 输出主机的规范名
    printf("host-name: %s\n", hptr->h_name);

    //@ 输出主机的别名
    char **pptr;
    for (pptr = hptr->h_aliases; *pptr != NULL; pptr++)
    {
        printf("host-alases: %s\n", *pptr);
    }

    //@ 输出ip地址
    char str[INET_ADDRSTRLEN];
    switch (hptr->h_addrtype)
    {
    case AF_INET:
        pptr = hptr->h_addr_list;
        for (; *pptr != NULL; pptr++)
        {
            printf("host-address: %s\n", inet_ntop(hptr->h_addrtype, hptr->h_addr, str, sizeof(str)));
        }
        break;
    default:
        printf("unknown address type\n");
        break;
    }

    return 0;
}
