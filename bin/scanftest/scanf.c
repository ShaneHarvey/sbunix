#include <stdio.h>

int main() {
    int rv, i, i2, i3;
    unsigned int x, x2, x3;
    char c, c2, c3;
    char str[16] = {0}, str2[16] = {0};

    printf("* * * * * Scanf Test * * * * *\n\n");

    printf("Testing %%s\n");

    printf("rv  = scanf(\"%%s\", str): ");
    rv = scanf("%s", str);
    printf("rv  = %d\n", rv);
    printf("str = \"%s\"\n", str);

    printf("rv  = scanf(\"%%s %%s\", str1, str2): ");
    rv = scanf("%s %s", str, str2);
    printf("rv  = %d\n", rv);
    printf("str = \"%s\"\n", str);
    printf("str2= \"%s\"\n\n", str2);

    printf("Testing %%c\n");

    printf("rv  = scanf(\"%%c\", &c): ");
    rv = scanf("%c", &c);
    printf("rv  = %d\n", rv);
    printf("c   = \'%c\'\n", c);
    printf("rv  = scanf(\"%%c\", &c): ");
    rv = scanf("%c", &c);
    printf("rv  = %d\n", rv);
    printf("c   = \'%c\'\n", c);

    printf("rv  = scanf(\"%%c %%c %%c\", &c1, &c2, &c3): ");
    rv = scanf("%c %c %c", &c, &c2, &c3);
    printf("rv  = %d\n", rv);
    printf("c1  = \'%c\'\n", c);
    printf("c2  = \'%c\'\n", c2);
    printf("c3  = \'%c\'\n\n", c3);

    printf("Testing %%d\n");

    printf("rv  = scanf(\"%%d\", &i): ");
    rv = scanf("%d", &i);
    printf("rv  = %d\n", rv);
    printf("i   = %d\n", i);

    printf("rv  = scanf(\"%%d %%d %%d\", &i1, &i2, &i3): ");
    rv = scanf("%d %d %d", &i, &i2, &i3);
    printf("rv  = %d\n", rv);
    printf("i1  = %d\n", i);
    printf("i2  = %d\n", i2);
    printf("i3  = %d\n\n", i3);

    printf("Testing %%x\n");

    printf("rv  = scanf(\"%%x\", &x): ");
    rv = scanf("%x", &x);
    printf("rv  = %d\n", rv);
    printf("x   = %x\n", x);

    printf("rv  = scanf(\"%%x %%x %%x\", &x1, &x2, &x3): ");
    rv = scanf("%x %x %x", &x, &x2, &x3);
    printf("rv  = %d\n", rv);
    printf("x1  = %x\n",x);
    printf("x2  = %x\n", x2);
    printf("x3  = %x\n\n", x3);

    return 0;
}
