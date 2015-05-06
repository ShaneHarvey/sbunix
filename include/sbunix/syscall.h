#ifndef _SBUNIX_SYSCALL_H
#define _SBUNIX_SYSCALL_H

int do_execve(char *filename, char *argv[], char *envp[]);



int do_dup(int oldfd);
int do_dup2(int oldfd, int newfd);



#endif //_SBUNIX_SYSCALL_H
