#ifndef _STDLIB_H
#define _STDLIB_H

#include <sys/defs.h>
#include <sys/wait.h>
#include <sys/types.h>


int main(int argc, char* argv[], char* envp[]);
void exit(int status);

/* memory */
typedef uint64_t size_t;
void *malloc(size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);

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

/* directories */
#define NAME_MAX 255
struct dirent
{
	long d_ino;
	off_t d_off;
	unsigned short d_reclen;
	char d_name [NAME_MAX+1];
};
void *opendir(const char *name);
struct dirent *readdir(void *dir);
int closedir(void *dir);

int atoi(const char *nptr);
#endif
