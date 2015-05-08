#include "test.h"

void exec_preemptuser(void) {
    long err;

    err = task_files_init(curr_task);
    if(err) {
        kpanic("task_files_init failed: %s\n", strerror(-err));
    }
    err = do_execve("/bin/preemptuser", NULL, NULL, 0);
    if(err) {
        kpanic("do_execve failed: %s\n", strerror(-err));
    }

    kill_curr_task(0);
}