#ifndef C_OS_SYSCALL_H
#define C_OS_SYSCALL_H

#ifdef ARCH_I386
#include <arch/i386/syscall.h>
#elif ARCH_ARM
#include <arch/arm/syscall.h>
#else
#error unsupported architecture
#endif

enum { 
	SYS_EXIT = 1, 
	SYS_FORK, 
	SYS_READ, 
	SYS_WRITE, 
	SYS_OPEN, 
	SYS_CLOSE, 
	SYS_WAITPID, 
	SYS_CREAT,
	SYS_LINK,
	SYS_UNLINK,
	SYS_EXECVE,
	SYS_CHDIR,
	SYS_TIME,
	SYS_MKNOD,
	SYS_LCHOWN,
	SYS_STAT = 18,
	SYS_LSEEK,
	SYS_GETPID,
	SYS_MOUNT,
	SYS_OLDUMOUNT,
	SYS_SETUID16,
	SYS_GETUID16,
	SYS_STIME,
	SYS_PTRACE,
	SYS_ALARM,
	SYS_FSTAT,
	SYS_PAUSE,
	SYS_UTIME,
	SYS_STTY,
	SYS_GTTY,
	SYS_ACCESS = 33,
	SYS_NICE,
	SYS_SYNC = 36,
	SYS_KILL,//37
	SYS_RENAME,
	SYS_MKDIR,
	SYS_RMDIR,
	SYS_DUP,
	SYS_PIPE,
	SYS_TIMES,
	SYS_BRK = 45,
	SYS_SETGID16,
	SYS_GETGID16,
	SYS_SIGNAL,
	SYS_GETEUID16,
	SYS_GETEGID16,
	SYS_ACCT,
	STS_UMOUNT,
	SYS_IOCTL = 54,
	SYS_FCNTL,
	SYS_SETPGID = 57,
	SYS_OLDUNAME = 59,
	SYS_UMASK,
	SYS_CHROOT,
	SYS_USTAT,
	SYS_DUP2,
	SYS_GETPPID,
	SYS_GETPGRP,
	SYS_SETSID,
	SYS_SIGACTION,
	SYS_SGETMASK,
	SYS_SIGSUSPEND = 72,
	SYS_GETTIMEOFDAY = 78,
	SYS_WAIT4 = 114,
	SYS_SIGRETURN = 119,
	SYS_READV = 145,
	SYS_WRITEV = 146,
	SYS_POLL = 168,
	SYS_RT_SIGACTION = 174,
	SYS_RT_SIGPROCMASK,
	SYS_RT_SIGSUSPEND = 179,
	SYS_CHOWN = 182,
	SYS_GETCWD,
	SYS_SBRK,
	SYS_MMAP2 = 192,
	SYS_STAT64 = 195,
	SYS_GETUID = 199,
	SYS_GETGID,
	SYS_GETEUID,
	SYS_SETUID = 213,
	SYS_SETGID ,
	SYS_FCNTL64 = 221,
	SYS_FUTEX = 240,
	SYS_NETWORK = 255, 
	SYS_DUMMY = 256,
	SYS_CLOCK_GETTIME = 265
};
#define ERROR_RETURN(x) return (-x)

void syscall_init();
int fork();
int dummy();
int get_pid();
int uputs(char *str);
int network_setup();
#endif
