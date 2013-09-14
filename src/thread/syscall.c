#include <common.h>
#include <mm/liballoc.h>
#include <string.h>
#include <kernel/interrupt.h>
#include <device/console.h>
#include <kernel/thread.h>
#include <kernel/timer.h>
#include <net/net_core.h>
#include <thread/syscall.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
//#include <kernel/vfs.h>
//needs to be seperated into two files

//define a couple of internal syscalls so
//we can fork and exec in the kernel 

int temp_stack[0x1000];
int *__esp = temp_stack;
int dummy()
{
//	static int ret = 0xdeadbeef;
//	static int test;
	
//	asm volatile("mov %%esp, %0":"=m"(test));
//	memset(__esp, 0, 4096);
//	__esp += 1024;
	
//	ret = (int)thread_current() + STACK_SIZE;
//	ret = test;
//	asm volatile("mov %0,%%esp\n"	:: "r"(__esp)	);
//	printf("top stack %x current pos %x\n",ret, test);

//	for(int i = 0; i < 32; i++)
	//	printf("stack %x %x\n", (ret - i*4), *(int *)(ret - i*4));

//	for(int i = 0; i < 32; i++)
	//	printf("stack %x %x\n", (test - 4*4 + i*4), *(int *)(test - 4*4 + i*4));
	int ret = SYSCALL_0N(SYS_DUMMY);
//	printf("top stack %x current pos %x\n",ret, test);
//	printf("dummy return\n");
	//asm volatile("mov %%esp, %0":"=m"(ret));



//	for(int i = 0; i < 10; i++)
	//	printf("stack %x %x\n", (ret - 8 + i*4), *(int *)(ret - 8 + i*4));
//	while(1)
//		printf("COCKS\n");
	return ret;
}
int get_pid()
{
	return SYSCALL_0N(SYS_GETPID);
}

int fork()
{
	return SYSCALL_0N(SYS_FORK);
}

int execv(const char *path, char * const argv[])
{
	return SYSCALL_3N(SYS_EXECVE, path, argv, NULL);
}

//FIXME: Old network stuff that should be moved or removed
extern void send_packet();
int sys_dummy()
{
//	PANIC("YAY!");
	send_packet();
//	printf("dummy syscall\n");
	return 0xcafebabe;	
}
int network_setup()
{
	return SYSCALL_0N(SYS_NETWORK);
}

extern void test_signals();

//TODO: need to verify pointers before letting functions dereference them
void syscall_handler (struct registers *regs)
{
	int call = regs->eax;
	char *buf = NULL;
	buf = buf;
//	printf("call %i\n",call);
	switch (call)
	{
		case SYS_READ:
			if((int)regs->ebx == -1)
			{
				regs->eax = -1;
				break;
			}
			regs->eax = sys_read((int)regs->ebx, (char *)regs->ecx, (int)regs->edx);
			break;
		case SYS_WRITE:
			regs->eax = sys_write((int)regs->ebx, (char *)regs->ecx, (int)regs->edx);
			break;
		case SYS_OPEN:
			regs->eax = sys_open((char *)regs->ebx, 0, NULL);
			return;
		case SYS_LSEEK:
			regs->eax = sys_lseek((int)regs->ebx, (off_t)regs->ecx, (int) regs->edx);
			return;
		case SYS_STAT:
			regs->eax = sys_stat((char *)regs->ebx, (struct stat *)regs->ecx);
			return;
		case SYS_GETPID:
			regs->eax = sys_getpid();
			return;
		case SYS_GETPGRP:
			regs->eax = sys_getpgrp();
			break;
		case SYS_FORK:
			regs->eax = sys_fork(regs);
			printf("fork @ eip %X\n", regs->eip);
			//dump_regs(regs);
			return;
		case SYS_EXECVE:
			regs->eax = sys_execve((char *)regs->ebx, (char **)regs->ecx, (char **)regs->edx);
			//FIXME: Is this needed?
			thread_yield();
			return;
		case SYS_SBRK:
			regs->eax = (uintptr_t)sys_brk(regs->ebx);
			return;
		case SYS_IOCTL:
			regs->eax = sys_ioctl((int)regs->ebx, (int)regs->ecx, regs->edx);
			return;
		case SYS_KILL:
			regs->eax = sys_kill(regs->ebx, regs->ecx);
			break;
		case SYS_SIGACTION:
			regs->eax = sys_sigaction(regs->ebx, (void*)regs->ecx, (void *)regs->edx);
			break;
		case SYS_SIGSUSPEND:
			regs->eax = sys_sigsuspend((void*)regs->ebx);
			break; 
		case SYS_CLOSE:
			regs->eax = sys_close(regs->ebx);
			break;
		case SYS_EXIT:
			//FIXME: This needs to be implemented properly
			printf("exit (%i)\n",regs->ebx);
			while(1);
			thread_exit();
			return;
		case SYS_GETTIMEOFDAY:
			regs->eax = sys_gettimeofday((struct timeval *)regs->ebx, (void *)regs->ecx);
			return;
		case SYS_GETCWD:
			regs->eax = (int)sys_getcwd((char *)regs->ebx, (size_t)regs->ecx);
			break;
		//case SYS_NETWORK:
			//regs->eax = sys_network_setup();
			//break;
		case SYS_DUMMY:
			printf("DUMMY\n");
			//regs->eax = sys_dummy();
			break;	
		default:
			printf("undefined system call %i!\n",call);
			regs->eax = 0;//ENOSYS;
			return;
	}
}

void syscall_init()
{
	interrupt_register(0x80, &syscall_handler);
}
