#include "test.h"

void test_exec(void) {
    int err;

    char *argv[] = {"-h", "hi", NULL};
    char *envp[] = {"PATH=/:", "HOME=/", NULL};
    err = do_execve("/bin/hello", argv, envp);
    if(err) {
        kpanic("do_execve failed: %s\n", strerror(-err));
    }

    kill_curr_task();
}