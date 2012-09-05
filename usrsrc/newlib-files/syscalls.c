#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>
#include "syscall.h"
//#include <_ansi.h>
#include <errno.h>

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
  return ret;//__MYPID;
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
kill(pid, sig)
     int pid;
     int sig;
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
close(int file) {
	int ret = SYSCALL_1N(SYS_CLOSE, file);
	return ret;
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
fstat(int file, struct stat *st) {
	st->st_mode = S_IFCHR;
	return 0;
}

int
stat(const char *file, struct stat *st){
	st->st_mode = S_IFCHR;
	return 0;
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
	base = SYSCALL_1N(SYS_SBRK, nbytes);
 /* 
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

*/
  return base;
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

