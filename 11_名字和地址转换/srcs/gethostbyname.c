
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/socket.h>

extern int h_errno;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("usage: %s www.google.com\n", *argv);
        return -1;
    }

    int i = 1;
    struct hostent *hptr;
    while (argv[i] != NULL)
    {
        hptr = gethostbyname(argv[i]);
        if (hptr == NULL)
        {
            printf("gethostbyname error for host: %s: %s\n", argv[i], hstrerror(h_errno));
            continue;
        }
        //@ 输出主机的规范名
        printf("\tofficial: %s\n", hptr->h_name);

        //@ 输出主机的别名
        char **pptr;
        char str[INET_ADDRSTRLEN];
        for (pptr = hptr->h_aliases; *pptr != NULL; pptr++)
            printf("\ttalias: %s\n", *pptr);

        //@ 输出ip地址
        switch (hptr->h_addrtype)
        {
        case AF_INET:
            pptr = hptr->h_addr_list;
            for (; *pptr != NULL; pptr++)
            {
                printf("\taddress: %s\n",
                       inet_ntop(hptr->h_addrtype, hptr->h_addr, str, sizeof(str)));
            }
            break;
        default:
            printf("unknown address type\n");
            break;
        }

        printf("---------------------------------------------------\n");
        i++;
    }
    return 0;
}
