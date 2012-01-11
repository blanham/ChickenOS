#ifndef C_OS_SYSCALL_H
#define C_OS_SYSCALL_H
enum { SYS_EXIT = 1, SYS_FORK, SYS_READ, SYS_WRITE, SYS_OPEN, SYS_CLOSE, SYS_WAITPID, SYS_CREAT, SYS_LINK, SYS_UNLINK, SYS_EXECVE,SYS_GETPID = 20, SYS_BRK = 45, SYS_PUTS, SYS_SBRK, SYS_DUMMY = 256};
#define SYSCALL_0N(num) ({	\
					int ret;			\
	asm volatile (	"int $0x80"		\
				   	: "=a"(ret) 		\
				   	: "0" (num)		\
				   	);					\
					ret;})

#define SYSCALL_1N(num, arg0) ({	\
					int ret;			\
	asm volatile (	"int $0x80"		\
				   	: "=a"(ret) 		\
				   	: "0" (num),		\
					  "b" (arg0) 		\
				   	);					\
					ret;})

#define SYSCALL_2N(num, arg0,arg1) ({	\
					int ret;			\
	asm volatile (	"int $0x80"		\
				   	: "=a"(ret) 		\
				   	: "0" (num),		\
					  "b" (arg0), 		\
					  "c" (arg1)		\
								\
				   	);					\
					ret;})
#define SYSCALL_3N(num, arg0,arg1, arg2) ({	\
					int ret;			\
	asm volatile (	"int $0x80"		\
				   	: "=a"(ret) 		\
				   	: "0" (num),		\
					  "b" (arg0), 		\
					  "c" (arg1),		\
					  "d" (arg2) 		\
				   	);					\
					ret;})

void syscall_init();
#define ENOSYS 255
int fork();
int get_pid();
int uputs(char *str);
#endif
