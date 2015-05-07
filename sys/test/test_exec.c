#include "test.h"

const char *argv[] = {"-h", "hi", NULL};
const char *envp[] = {"PATH=/:", "HOME=/", NULL};

void test_exec(void) {
    int err;

    err = task_files_init(curr_task);
    if(err) {
        kpanic("task_files_init failed: %s\n", strerror(-err));
    }
    err = do_execve("/bin/forktest", argv, envp);
    if(err) {
        kpanic("do_execve failed: %s\n", strerror(-err));
    }

    kill_curr_task(0);
}
