#ifndef C_OS_SYSCALL_H
#define C_OS_SYSCALL_H
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
	SYS_SETUID,
	SYS_GETUID,
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
	SYS_SETGID,
	SYS_GETGID,
	SYS_SIGNAL,
	SYS_GETEUID,
	SYS_GETEGID,
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
	SYS_CHOWN = 182,
	SYS_GETCWD,
	SYS_SBRK,
	SYS_NETWORK = 255, 
	SYS_DUMMY = 256
};
#define SYSCALL_0N(num) ({				\
					int ret;			\
	asm volatile (	"int $0x80"			\
				   	: "=a"(ret) 		\
				   	: "0" (num)			\
				   	);					\
					ret;})

#define SYSCALL_1N(num, arg0) ({		\
					int ret;			\
	asm volatile (	"int $0x80"			\
				   	: "=a"(ret) 		\
				   	: "0" (num),		\
					  "b" (arg0) 		\
				   	);					\
					ret;})

#define SYSCALL_2N(num, arg0,arg1) ({	\
					int ret;			\
	asm volatile (	"int $0x80"			\
				   	: "=a"(ret) 		\
				   	: "0" (num),		\
					  "b" (arg0), 		\
					  "c" (arg1)		\
										\
				   	);					\
					ret;})
#define SYSCALL_3N(num, arg0,arg1, arg2) ({	\
					int ret;			\
	asm volatile (	"int $0x80"			\
				   	: "=a"(ret) 		\
				   	: "0" (num),		\
					  "b" (arg0), 		\
					  "c" (arg1),		\
					  "d" (arg2) 		\
				   	);					\
					ret;})
#define SYSCALL_4N(num, arg0,arg1, arg2, arg3) ({	\
					int ret;			\
	asm volatile (	"int $0x80"			\
				   	: "=a"(ret) 		\
				   	: "0" (num),		\
					  "b" (arg0), 		\
					  "c" (arg1),		\
					  "d" (arg2), 		\
					  "S" (arg3)		\
				   	);					\
					ret;})

void syscall_init();
int fork();
int dummy();
int get_pid();
int uputs(char *str);
int network_setup();
#endif
