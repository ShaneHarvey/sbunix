#include "test.h"

void exec_preemptuser(void) {
    int err;

    err = task_files_init(curr_task);
    if(err) {
        kpanic("task_files_init failed: %s\n", strerror(-err));
    }
    err = do_execve("/bin/preemptuser", NULL, NULL);
    if(err) {
        kpanic("do_execve failed: %s\n", strerror(-err));
    }

    kill_curr_task(0);
}