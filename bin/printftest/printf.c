#include <stdio.h>

int main(int argc, char* argv[], char* envp[]) {
    printf("Testing %%c functionality\n");
    printf("0                  : %c\n", '0');
    printf("A                  : %c\n", 'A');
    printf("!                  : %c\n", '!');
    printf("z                  : %c\n", 'z');
    printf("\\                  : %c\n", '\\');
    printf("\n");

    printf("Testing %%d functionality\n");
    printf("123456789          : %d\n", 123456789);
    printf("1                  : %d\n", 1);
    printf("0                  : %d\n", 0);
    printf("-1                 : %d\n", -1);
    printf("-123456789         : %d\n", -123456789);
    printf("\n");

    printf("Testing %%p functionality\n");
    printf("0xffffffffeeeeeeee : %p\n", (void *)0xffffffffeeeeeeee);
    printf("0xffffffff         : %p\n", (void *)0xffffffff);
    printf("0xdeadbcf          : %p\n", (void *)0xdeadbcf);
    printf("0x1                : %p\n", (void *)0x1);
    printf("0x0                : %p\n", (void *)0x0);
    printf("\n");

    printf("Testing %%o functionality\n");
    printf("1234567            : %o\n", 01234567);
    printf("333                : %o\n", 0333);
    printf("1                  : %o\n", 01);
    printf("0                  : %o\n", 00);
    printf("\n");

    printf("Testing %%x functionality\n");
    printf("ffffeeee           : %x\n", 0xffffeeee);
    printf("ffffffff           : %x\n", 0xffffffff);
    printf("deadbcf            : %x\n", 0xdeadbcf);
    printf("1                  : %x\n", 0x1);
    printf("0                  : %x\n", 0x0);
    printf("\n");
    return 0;




}
