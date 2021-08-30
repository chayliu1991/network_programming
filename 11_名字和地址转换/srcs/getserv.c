#include <netdb.h>
#include <stdio.h>

static void print_server_info(const struct servent *serv)
{
    if (serv == NULL)
        printf("unknown\n");

    printf("name:%s\n", serv->s_name);
    char **aliase = serv->s_aliases;
    while (*aliase != NULL)
    {
        printf("aliase:%s\n", *aliase);
        aliase++;
    }
    printf("port:%d\n", serv->s_port);
    printf("proto:%s\n", serv->s_proto);
    printf("------------------------------------------------------------\n");
}

int main(int argc, char **argv)
{
    struct servent *sptr;

    sptr = getservbyname("domain", "udp");
    print_server_info(sptr);

    sptr = getservbyname("ftp", "tcp");
    print_server_info(sptr);

    sptr = getservbyport(htons(53), "udp");
    print_server_info(sptr);

    sptr = getservbyport(htons(21), "tcp");
    print_server_info(sptr);

    return 0;
}