#include <common.h>
#include <mm/liballoc.h>
#include <kernel/interrupt.h>
#include <device/console.h>
#include <kernel/thread.h>
#include <kernel/timer.h>
#include <net/net_core.h>
#include <thread/syscall.h>
#include <stdio.h>
//#include <kernel/vfs.h>
//needs to be seperated into two files

int dummy()
{
	return SYSCALL_0N(SYS_DUMMY);
}
int get_pid()
{
	int test = SYSCALL_0N(SYS_GETPID);
	return test;
//	return 0;
}
//define a couple of internal syscalls so
//we can fork and exec in the kernel 
int fork()
{
	return SYSCALL_0N(SYS_FORK);
}

//int uputs(char *str)
//{
//	int test = SYSCALL_1N(SYS_PUTS,str);
//	return test;
//}
#include <string.h>
int execv(const char *path, char * const argv[])
{
	return SYSCALL_3N(SYS_EXECVE, path, argv, NULL);
}

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
//FIXME: This does not belong here
void *sys_brk(uintptr_t ptr)
{
	thread_t *cur = thread_current();
//	printf("brk %x incr %x\t", cur->brk, ptr);
	void * old;
	if(ptr == 0)
		return cur->brk;
	else
	{
		old = cur->brk;
		cur->brk = cur->brk + ptr;
	//	printf("set brk to %x\n", cur->brk);
		return old;	
	}
}
extern void test_signals();
//TODO: need to verify pointers before letting functions dereference them
void syscall_handler (struct registers *regs)
{
	int call = regs->eax;
	char *buf = NULL;
	buf = buf;
	//printf("call %i\n",call);
	switch (call)
	{
		case SYS_READ:
		//	printf("read\n");
			if((int)regs->ebx == -1)

				{
					regs->eax = -1;
					break;
				}
		//	printf("read %x %X %X\n",regs->ebx, regs->ecx, regs->edx);
			regs->eax = sys_read((int)regs->ebx, (char *)regs->ecx, (int)regs->edx);
			break;
		case SYS_WRITE:
		//	printf("write %x %X %X\n",regs->ebx, regs->ecx, regs->edx);
		//	printf("%s", (char *)regs->ecx);
			regs->eax = sys_write((int)regs->ebx, (char *)regs->ecx, (int)regs->edx);
		//	printf("\n");
			break;
		case SYS_OPEN:
			regs->eax = sys_open((char *)regs->ebx, 0, NULL);
		//	printf("path %s fd %i\n",(char *)regs->ebx, regs->eax);
			return;
		case SYS_LSEEK:
			regs->eax = sys_lseek((int)regs->ebx, (off_t)regs->ecx, (int) regs->edx);
			return;
		case SYS_GETPID:
		//	printf("getpid\n");
		//	while(1);
			regs->eax = sys_getpid();
			return;
		case SYS_GETPGRP:
			regs->eax = sys_getpgrp();
			break;
		case SYS_FORK:
			regs->eax = sys_fork(regs);
			//dump_regs(regs);
			return;
		case SYS_EXECVE:
			//printf("exec\n");
		//	dump_regs(regs);
			regs->eax = sys_execve((char *)regs->ebx, (char **)regs->ecx, (char **)regs->edx);
		//	dump_regs(regs);
			return;
		case SYS_SBRK:
			regs->eax = (uintptr_t)sys_brk(regs->ebx);
			return;
	//	case 146://writev(int fd, struct iovec *vec, int count)
//
			//dump_regs(regs);
		//	printf("%s", *(int *)regs->ecx);
		//	regs->eax = -1;
		//	printf("%i\n", regs->edx);
		//	sys_write(regs->ebx, (char *)*(int *)regs->ecx, *((int*)regs->ecx + 1));
		//	printf("\n");
			//sys_write(0, (char *()*((int*)regs->ecx + 2), *((int*)regs->ecx + 3));
		//	return;
		case SYS_IOCTL:
			regs->eax = sys_ioctl((int)regs->ebx, (int)regs->ecx, regs->edx);
			return;
		
//		case 57:

		//	regs->eax  =0;
	//		return;
	//	case 65:
	//		regs->eax = 0;
	//		return;
	//	case 37:
	//		sys_kill((int)regs->ebx, (int)regs->ecx);
	//		regs->eax = 0;
		//	return;
//		case 252:
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
	//	case 174:
	//		//if(regs->ecx != 0 && regs->edx != 0 && regs->ebx <20)
		//	if(regs->ebx == 21)
		//	printf("%i %x %x\n", regs->ebx, *(int *)regs->ecx, *(int *)regs->edx);
			break;
	//	case 200:
		//	thread_scheduler(regs);
		//	break;
	//	case SYS_NETWORK:
		//	regs->eax = sys_network_setup();
		//	break;
	//	case SYS_DUMMY:

		//	regs->eax = sys_dummy();
		//	break;	
		default:
			printf("undefined system call %i!\n",call);
		//	while(1);
			regs->eax = 0;
			return;
	}
}

void syscall_init()
{
	interrupt_register(0x80, &syscall_handler);
}


