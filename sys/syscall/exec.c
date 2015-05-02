#include <sbunix/sbunix.h>
#include <sbunix/tarfs.h>
#include <sbunix/sched.h>
#include <sbunix/elf64.h>
#include <sbunix/mm/vmm.h>
#include <errno.h>
#include <sbunix/string.h>

int do_execve(char *filename, char *argv[], char *envp[]) {
    struct file *fp;
    struct mm_struct *mm;
    int err;
    /* TODO: validate user pointers */
    /* TODO: resolve filename to absolute path */
    fp = kmalloc(sizeof(struct file));
    if(!fp)
        return -ENOMEM;
    err = tarfs_open(filename, fp);
    if(err)
        goto cleanup_kmalloc;
    err = elf_validiate_exec(fp);
    if(err)
        goto cleanup_file;
    /* create new mm_struct */
    mm = mm_create();
    if(!mm) {
        err = -ENOMEM;
        goto cleanup_file;
    }
    err = elf_load(fp, mm);
    if(err)
        goto cleanup_mm;

    err = add_heap(mm);
    if(err)
        goto cleanup_mm;
    err = add_stack(mm, argv, envp);
    if(err)
        goto cleanup_mm;
    /* TODO: free old mm_struct */
    write_cr3(mm->pml4);
    /* If current task is a user, destroy it's mm_struct  */
    if(curr_task->type == TASK_KERN) {
        curr_task->mm = mm;
        curr_task->type = TASK_USER;
    } else {
        mm_destroy(curr_task->mm);
    }
    /* Update curr_task->cmdline  */
    strncpy(curr_task->cmdline, filename, TASK_CMDLINE_MAX);
    /* TODO: Copy on write stuff????? */

    return 0;
cleanup_mm:
    mm_destroy(mm);
cleanup_file:
    fp->f_op->close(fp);
    return err;
cleanup_kmalloc:
    kfree(fp);
    return err;
}