#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/signal.h>
#include <sys/ioctls.h>
#include <termios.h>
#include "syscall.h"
//#include <_ansi.h>
#include <errno.h>
//need to macro-ize this file

// --- Process Control ---

int
_exit(int val){
  int ret = SYSCALL_1N(SYS_EXIT, val);
//	exit(val);
  return ret;
}

int
execve(char *name, char **argv, char **env) {
//	errno = ENOMEM;
	int ret = SYSCALL_3N(SYS_EXECVE, name, argv, env);
	return ret;
}

/*
 * getpid -- only one process, so just return 1.
 */
#define __MYPID 1
int
getpid()
{
	int ret = SYSCALL_0N(SYS_GETPID);
	return ret;
}


int 
fork(void) {
	//errno = ENOTSUP;
	int ret = SYSCALL_0N(SYS_FORK);
	return ret;
}


/*
 * kill -- go out via exit...
 */
int
kill(int pid, int sig)
{
 // if(pid == __MYPID)
 //   _exit(sig);
	int ret = SYSCALL_2N(SYS_KILL, pid, sig);

//	errno = EINVAL;
  return ret;
}

int
wait(int *status) {
//	errno = ECHILD;
	int ret = SYSCALL_1N(SYS_WAITPID, status);
	
	return ret;
}

// --- I/O ---

/* TODO
 * isatty -- returns 1 if connected to a terminal device,
 *           returns 0 if not. Since we're hooked up to a
 *           serial port, we'll say yes and return a 1.
 */
int
isatty(fd)
     int fd;
{

  return (1);
}

int
chdir(const char *path)
{
//	printf("%s\n",__func__);	
	return SYSCALL_1N(SYS_CHDIR, path);
}

int
close(int file) {
	int ret = SYSCALL_1N(SYS_CLOSE, file);
	return ret;
}

int
getcwd(char *buf, size_t size)
{
//	printf("%s\n",__func__);	

	return SYSCALL_2N(SYS_GETCWD, buf, size);
}

int
link(char *old, char *new) {
	//errno = EMLINK;
	int ret = SYSCALL_2N(SYS_LINK, old, new);
	return ret;
}

int
lseek(int file, int ptr, int dir) {
	int ret = SYSCALL_3N(SYS_LSEEK, file, ptr, dir);
	return ret;
}

int
ioctl(int fildes, int request, ...)
{
	int ret;
	va_list va;
	va_start(va, request);
	ret = SYSCALL_3N(SYS_IOCTL, fildes, request, va);
	va_end(va);
	return ret;
}

int
open(const char *name, int flags, ...) {
	int ret = SYSCALL_3N(SYS_OPEN, name, flags, NULL);
	return ret;
}

int
read(int file, char *ptr, int len) {
	// XXX: keyboard support
	int ret = SYSCALL_3N(SYS_READ, file, ptr, len);
	return ret;
}

int
fcntl(int fd, int cmd, ...)
{
	va_list ap;
	int ret;

	va_start(ap, cmd);
	ret = SYSCALL_3N(SYS_FCNTL, fd, cmd, ap);
//	printf("%s\n",__func__);
	va_end(ap);
	return ret;
}

int 
fstat(int file, struct stat *st)
{
	return SYSCALL_2N(SYS_FSTAT, file, st);
}


int
stat(const char *file, struct stat *st)
{
	//st->st_mode = S_IFCHR;
	//write(0, "stat\n", 5);	
	return SYSCALL_2N(SYS_STAT, file, st);
}

int
lstat(const char *file, struct stat *st)
{
	st->st_mode = S_IFCHR;


	write(0, "lstat\n", 6);	
	return 0;
}

mode_t
umask(mode_t mask)
{
//	printf("%s\n",__func__);	

	return 0777;
}

int
unlink(char *name) {
	int ret = SYSCALL_1N(SYS_UNLINK, name);
	return ret;
}


int
write(int file, char *ptr, int len) {
	int ret = SYSCALL_3N(SYS_WRITE, file, ptr, len);
	return ret;
}

int
dup(int fildes)
{
	return SYSCALL_1N(SYS_DUP, fildes);
}


int
dup2(int file1, int file2)
{
	return SYSCALL_2N(SYS_DUP2, file1, file2);
}

int
pipe(int fildes[2])
{
	return SYSCALL_1N(SYS_PIPE, fildes);
}

int
sigaction(int sig, const struct sigaction *act, struct sigaction *oact)
{
	return SYSCALL_3N(SYS_SIGACTION, sig, act, oact);
}

int sigprocmask(int how, const sigset_t *set, sigset_t *oset)
{

//	printf("%s\n",__func__);	
//	return SYSCALL_3N(SYS_SIGPROCMASK;
	return -1;
}

int sigsuspend(const sigset_t *sigmask)
{
//	printf("%s\n",__func__);	
 	return SYSCALL_1N(SYS_SIGSUSPEND, sigmask);
}

int
wait3(int *stat_loc, int options, struct rusage *rusage)
{
//	printf("%s\n",__func__);	

	return SYSCALL_4N(SYS_WAIT4, -1, stat_loc, options, rusage);
}

//pid_t _pgid;


int
tcsetpgrp(int fildes, pid_t pid)
{
//	_pgid = pid;
//	printf("%s\n",__func__);	
	return ioctl(fildes, TIOCGPGRP, pid);
}
int
tcgetpgrp(int fildes)
{
	pid_t pid;
	int ret;
//	printf("%s\n",__func__);	
	ret = ioctl(fildes, TIOCSPGRP, &pid);
	if(ret != 0)
		return -1;
	else
		return pid; 
}
int
setpgid(pid_t pid, pid_t pgid)
{

//	printf("%s\n",__func__);	
	return SYSCALL_2N(SYS_SETPGID, pid, pgid);
}
pid_t
getpgrp(void)
{
//	printf("%s\n",__func__);	

	return SYSCALL_0N(SYS_GETPGRP);
}

int
setpgrp(pid_t pid, pid_t pgid)
{
//	printf("%s\n",__func__);	

	return setpgid(pid, pgid);
}

uid_t
getegid(void)
{
//	printf("%s\n",__func__);	

	return 1;
}

uid_t
getgid(void)
{

//	printf("%s\n",__func__);	
	return 1;
}
pid_t
getppid(void)
{
//	printf("%s\n",__func__);	
	return 1;
}
uid_t
geteuid(void)
{
//	printf("%s\n",__func__);	

	return 1;
}

uid_t
getuid(void)
{
//	printf("%s\n",__func__);	

	return 1;
}

int
getgroups(int gidsetsize, gid_t grouplist[])
{

//	printf("%s\n",__func__);	

	return 1;
}
// --- Memory ---

/* _end is set in the linker command file */
extern caddr_t _end;

#define PAGE_SIZE 4096ULL
#define PAGE_MASK 0xFFFFFFFFFFFFF000ULL
#define HEAP_ADDR (((unsigned long long)&_end + PAGE_SIZE) & PAGE_MASK)

/*
 * sbrk -- changes heap size size. Get nbytes more
 *         RAM. We just increment a pointer in what's
 *         left of memory on the board.
 */
caddr_t
sbrk(int nbytes){
  static caddr_t heap_ptr = NULL;
  caddr_t base;
	base = (caddr_t)SYSCALL_1N(SYS_SBRK, nbytes);
	return base;/* 
  int temp;

  if(heap_ptr == NULL){
    heap_ptr = (caddr_t)HEAP_ADDR;
  }

  base = heap_ptr;
	
  if(((unsigned long long)heap_ptr & ~PAGE_MASK) != 0ULL){
    temp = (PAGE_SIZE - ((unsigned long long)heap_ptr & ~PAGE_MASK));

    if( nbytes < temp ){
      heap_ptr += nbytes;
      nbytes = 0;
    }else{
      heap_ptr += temp;
      nbytes -= temp;
    }
  }

  while(nbytes > PAGE_SIZE){
    //allocPage(heap_ptr);
		
    nbytes -= (int) PAGE_SIZE;
    heap_ptr = heap_ptr + PAGE_SIZE;
  }
  
  if( nbytes > 0){
    //allocPage(heap_ptr);

    heap_ptr += nbytes;
  }


  return base;*/
	/*
  static caddr_t heap_ptr = NULL;
  caddr_t        base;

  if (heap_ptr == NULL) {
    heap_ptr = (caddr_t)&_end;
  }

  if ((RAMSIZE - heap_ptr) >= 0) {
    base = heap_ptr;
    heap_ptr += nbytes;
    return (base);
  } else {
    errno = ENOMEM;
    return ((caddr_t)-1);
		}*/
}


// --- Other ---
int gettimeofday(struct timeval *p, void *z){
	 
	int ret = SYSCALL_2N(SYS_GETTIMEOFDAY, p, z);
	return ret;
}

