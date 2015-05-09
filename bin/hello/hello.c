#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[], char* envp[]) {
    char *user;

    user = getenv("USER");
    if(!user || !*user)
        user = "meatbag";

    printf("Hello, %s!\n", user);
    return 0;
}
