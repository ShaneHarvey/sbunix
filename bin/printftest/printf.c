#include <stdio.h>

int main(int argc, char* argv[], char* envp[]) {
    printf("Testing %%c functionality %d\n", sizeof(long long));
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

    printf("Testing %%l functionality\n");
    printf("987654321999       : %l\n", 987654321999L);
    printf("123456789          : %l\n", 123456789L);
    printf("1                  : %l\n", 1L);
    printf("0                  : %l\n", 0L);
    printf("-1                 : %l\n", -1L);
    printf("-123456789         : %l\n", -123456789L);
    printf("-987654321999      : %l\n", -987654321999L);
    printf("\n");

    printf("Testing %%ll functionality\n");
    printf("987654321999       : %ll\n", 987654321999LL);
    printf("123456789          : %ll\n", 123456789LL);
    printf("1                  : %ll\n", 1LL);
    printf("0                  : %ll\n", 0LL);
    printf("-1                 : %ll\n", -1LL);
    printf("-987654321999      : %ll\n", -987654321999LL);
    printf("\n");

    printf("Testing %%p functionality\n");
    printf("0xffffffffeeeeeeee : %p\n", 0xffffffffeeeeeeee);
    printf("0xffffffff         : %p\n", 0xffffffff);
    printf("0xdeadbcf          : %p\n", 0xdeadbcf);
    printf("0x1                : %p\n", 0x1);
    printf("0x0                : %p\n", 0x0);
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

    printf("Testing %%xl functionality\n");
    printf("1234567890abcdef   : %xl\n", 0x1234567890abcdef);
    printf("ffffeeee           : %xl\n", 0xffffeeee);
    printf("ffffffff           : %xl\n", 0xffffffff);
    printf("deadbcf            : %xl\n", 0xdeadbcf);
    printf("1                  : %xl\n", 0x1);
    printf("0                  : %xl\n", 0x0);
    printf("\n");
    return 0;




}
