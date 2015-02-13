#include <stdio.h>

int main() {
    int rv;
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

    return 0;
}
