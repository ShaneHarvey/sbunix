#include <stdio.h>

int main() {
    printf("* * * * * Printf Test * * * * *\n\n");

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

    printf("Testing %%ld functionality\n");
    printf("9999999999999999   : %ld\n", 9999999999999999L);
    printf("1                  : %ld\n", 1L);
    printf("0                  : %ld\n", 0L);
    printf("-1                 : %ld\n", -1L);
    printf("-9999999999999999  : %ld\n", -9999999999999999L);
    printf("\n");

    printf("Testing %%lo functionality\n");
    printf("123456777777777777 : %lo\n", 0123456777777777777UL);
    printf("333                : %lo\n", 0333UL);
    printf("1                  : %lo\n", 0UL);
    printf("0                  : %lo\n", 00UL);
    printf("\n");

    printf("Testing %%lx functionality\n");
    printf("ffffffffdeadbecf   : %lx\n", 0xffffffffdeadbecfUL);
    printf("ffffeeee           : %lx\n", 0xffffeeeeUL);
    printf("deadbcf            : %lx\n", 0xdeadbcfUL);
    printf("1                  : %lx\n", 0x1UL);
    printf("0                  : %lx\n", 0x0UL);
    printf("\n");

    return 0;
}
