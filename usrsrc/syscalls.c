/* note these headers are all provided by newlib - you don't need to provide them */
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>
#include "../src/include/thread/syscall.h"

int uputs(char *str)
{
	int test = SYSCALL_1N(SYS_PUTS,str);
	return test;
}

void _exit(int ret)
{
	SYSCALL_0N(SYS_EXIT);

}
int close(int file)
{
	return 0;
}
char **environ; /* pointer to array of char * strings that define the current environment variables */
int execve(char *name, char **argv, char **env)
{
	return ENOSYS;
}
int fork()
{
	return SYSCALL_0N(SYS_FORK);
}
int fstat(int file, struct stat *st)
{
	return 0;
}
int getpid()
{
	return 0;
}
int isatty(int file)
{
	return 0;
}
int kill(int pid, int sig)
{
	return 0;
}
int link(char *old, char *new)
{
	return 0;
}
int lseek(int file, int ptr, int dir)
{
	return 0;
}
int open(const char *name, int flags, ...)
{
	return 0;
}
int read(int file, char *ptr, int len)
{
	return SYSCALL_3N(SYS_READ, file, ptr, len);
}
caddr_t sbrk(int incr)
{
	return SYSCALL_0N(SYS_SBRK);
}
int stat(const char *file, struct stat *st)
{
	return 0;
}
clock_t times(struct tms *buf)
{
	return 0;
}
int unlink(char *name)
{
	return 0;
}
int wait(int *status)
{
	return 0;
}
int write(int file, char *ptr, int len)
{
	return SYSCALL_3N(SYS_WRITE, file, ptr, len);
}
int gettimeofday(struct timeval *p, void *z)
{
	return 0;
}
