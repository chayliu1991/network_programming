#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef union
{
    unsigned char ch;
    unsigned short si;
} Un;

int main()
{
    Un un;
    un.si = 0x01;

    if (un.ch == 0x00) //@ 0x0001
    {
        printf("bid-endian\n");
    }
    else if (un.ch == 0x01) //@ 0x0100
    {
        printf("little-endian\n");
    }
    else
    {
        printf("unknown\n");
    }

    exit(EXIT_SUCCESS);
}