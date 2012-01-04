#include <common.h>
#include <kernel/interrupt.h>
#include <kernel/console.h>
#include <kernel/thread.h>
#include <thread/syscall.h>
#include <stdio.h>
//#include <kernel/vfs.h>
//needs to be seperated into two files
#define SYSCALL_0(num) ({				\
					int ret;			\
	asm volatile ( 	"pushl %1\n"		\
					"int $0x80\n"		\
					"addl $4, %%esp\n"	\
				   	: "=a"(ret) 		\
				   	: "i" (num)			\
					: "memory"			\
				   	);					\
					ret;})

#define SYSCALL_1(num, arg0) ({			\
					int ret;			\
	asm volatile ( 						\
					"pushl %2\n"		\
					"pushl %1\n"		\
					"int $0x80\n"		\
				    "mov %%eax,%0\n"	\
					"addl $8, %%esp\n"	\
				   	: "=a"(ret) 		\
				   	: "i" (num),			\
					 "m" (arg0)		\
					: "memory"			\
				   	);					\
					ret;})
#define SYSCALL_2(num, arg0,arg1) ({	\
					int ret;			\
	asm volatile ( 	"xchg %%bx, %%bx\n"					\
					"pushl %3\n"		\
					"pushl %2\n"		\
					"pushl %1\n"		\
					"int $0x80\n"		\
					"addl $0xC, %%esp\n"	\
				   	: "=a"(ret) 		\
				   	: "i" (num),		\
					 "m" (arg0), 		\
						"m"(arg1)		\
					: "memory"			\
				   	);					\
					ret;})
int dummy()
{
	int test = SYSCALL_0(SYS_DUMMY);
	return test;
}
pid_t get_pid()
{
	int test = SYSCALL_0(SYS_GETPID);
	return test;
}
pid_t fork()
{
	int test = SYSCALL_0(SYS_FORK);
	return test;
}

int uputs(char *str)
{
	int test = SYSCALL_1(SYS_PUTS,str);
	return test;
}
#include <string.h>
int execv(const char *path, char * const argv[])
{
	path = path;
	//printf("path %X argv %x\n",path, argv);
	return SYSCALL_2(SYS_EXECV, path, argv);
//	return test;
}

int sys_dummy()
{
//	PANIC("YAY!");
	printf("dummy syscall\n");
	return 0xcafebabe;	
}


void syscall_handler (struct registers *regs)
{
	int call = *((int *)((uint32_t)regs->useresp));
	char *str;
	char **dob; 
	
	switch (call)
	{
		
		case SYS_PUTS:
			str = *((char **)((uint32_t)regs->useresp + 4));
			printf("%s", str);
			regs->eax = 0;
			break;
		case SYS_GETPID:
			regs->eax = sys_getpid();
			return;
		case SYS_FORK:
			regs->eax = sys_fork(regs);
			return;
		case SYS_DUMMY:
			regs->eax = sys_dummy();
			while(1)
				asm volatile("hlt");
			return;
		case SYS_EXECV:
			str = (char *)*((int *)(regs->useresp + 4));
			dob = (char **)*((int *)(regs->useresp + 8));
			regs->eax = sys_execv(str, dob);
			return;
		default:
			printf("undefined system call %x!\n",call);
			return;
	}
}

void syscall_init()
{
	interrupt_register(0x80, &syscall_handler);
}


