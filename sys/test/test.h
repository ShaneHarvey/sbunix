#ifndef _SBUNIX_TEST_H
#define _SBUNIX_TEST_H

#include <sbunix/sbunix.h>
#include <sbunix/syscall.h>
#include <sbunix/fs/elf64.h>
#include <sbunix/fs/terminal.h>
#include <sbunix/sched.h>
#include <sbunix/string.h>

void test_exec(void);
void test_terminal(void);
void test_pipe(void);
void exec_preemptuser(void);

#endif //_SBUNIX_TEST_H
