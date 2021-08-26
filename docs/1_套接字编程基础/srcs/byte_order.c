#include <stdio.h>

void byte_order()
{
    union
    {
        short s;
        char ch[sizeof(short)];
    } test;
    test.s = 0x0102;
    if (test.ch[0] == 1 && test.ch[1] == 2)
        printf("big endian\n");
    else if (test.ch[0] == 2 && test.ch[1] == 1)
        printf("little endian\n");
    else
        printf("unknown...\n");
}

int main()
{
    byte_order();
    return 0;
}