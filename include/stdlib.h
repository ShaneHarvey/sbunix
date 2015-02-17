#ifndef _STDLIB_H
#define _STDLIB_H

#include <sys/defs.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <dirent.h>

/* These two were in stdlib.h, but we moved them */
#include <unistd.h>
#include <time.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define BUFSIZ 4096

void exit(int status);

/* memory */
typedef uint64_t size_t;
void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);

/**
* Follows glibc wrapper
* Returns: 0 on success, -1 on failure
*/
int brk(void *end_data_segment);

/* processes */
pid_t fork(void);
pid_t getpid(void);
pid_t getppid(void);
int execve(const char *filename, char *const argv[], char *const envp[]);
pid_t waitpid(pid_t pid, int *status, int options);

unsigned int alarm(unsigned int seconds);

/* paths */
char *getcwd(char *buf, size_t size);
int chdir(const char *path);

/* environment */
char *getenv(const char *name);
int setenv(const char *envname, const char *envval, int overwrite);
int putenv(char *string);
int unsetenv(const char *name);

/* files */
typedef int64_t ssize_t;
enum { O_RDONLY = 0, O_WRONLY = 1, O_RDWR = 2, O_CREAT = 0x40, O_DIRECTORY = 0x10000 };
int open(const char *pathname, int flags);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
enum { SEEK_SET = 0, SEEK_CUR = 1, SEEK_END = 2 };
off_t lseek(int fildes, off_t offset, int whence);
int close(int fd);
int pipe(int filedes[2]);
int dup(int oldfd);
int dup2(int oldfd, int newfd);

int atoi(const char *nptr);
#endif
