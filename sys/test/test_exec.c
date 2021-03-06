#include "test.h"

const char *argv[] = {NULL};
const char *envp[] = {"PATH=/:", "HOME=/", NULL};

void test_exec(void) {
    long err;

    err = task_files_init(curr_task);
    if(err) {
        kpanic("task_files_init failed: %s\n", strerror(-err));
    }
    err = do_execve("/bin/sbush", argv, envp, 0);
    if(err) {
        kpanic("do_execve failed: %s\n", strerror(-err));
    }

    kill_curr_task(0);
}
