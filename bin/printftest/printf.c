#include <stdio.h>
#include "../../include/stdio.h"

int main(int argc, char* argv[], char* envp[]) {
    printf("Testing %%d functionality\n");
    printf("123456789        : %d\n", 123456789);
    printf("1                : %d\n", 1);
    printf("0                : %d\n", 0);
    printf("-1               : %d\n", -1);
    printf("-123456789       : %d\n", -123456789);

    printf("Testing %%l functionality\n");
    printf("123456789        : %l\n", 123456789);
    printf("1                : %l\n", 1);
    printf("0                : %l\n", 0);
    printf("-1               : %l\n", -1);
    printf("-123456789       : %l\n", -123456789);

    printf("Testing %%ll functionality\n");
    printf("987654321999     : %ll\n", 987654321999);
    printf("123456789        : %ll\n", 123456789);
    printf("1                : %ll\n", 1);
    printf("0                : %ll\n", 0);
    printf("-1               : %ll\n", -1);
    printf("-987654321999    : %ll\n", -987654321999);

    printf("Testing %%p functionality\n");
    printf("ffffffffeeeeeeee : %p\n", 0xffffffffeeeeeeee);
    printf("ffffffff         : %p\n", 0xffffffff);
    printf("deadbcf          : %p\n", 0xdeadbcf);
    printf("1                : %p\n", 0x1);
    printf("0                : %p\n", 0x0);

    printf("Testing %%o functionality\n");
    printf("1234567          : %o\n", 01234567);
    printf("333              : %o\n", 0333);
    printf("1                : %o\n", 01);
    printf("0                : %o\n", 00);
	return 0;
}
