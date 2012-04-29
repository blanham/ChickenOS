#include <common.h>
#include <mm/liballoc.h>
#include <kernel/interrupt.h>
#include <kernel/console.h>
#include <kernel/thread.h>
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

int uputs(char *str)
{
	int test = SYSCALL_1N(SYS_PUTS,str);
	return test;
}
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

void *sys_brk(uintptr_t ptr)
{
	thread_t *cur = thread_current();
//	printf("brk %x incr %x\n", cur->brk, ptr);
	if(ptr == 0)
		return cur->brk;
	else
	{
		cur->brk = cur->brk + ptr;
	//	printf("set brk to %x\n", cur->brk);
		return cur->brk;	
	}
}
extern void test_signals();
void syscall_handler (struct registers *regs)
{
	int call = regs->eax;
	char *buf = NULL;
	buf = buf;
//	printf("call %i\n",call);
	switch (call)
	{
		
	//	case SYS_PUTS:
	//		printf("%s", (char *)regs->ebx);
	//		regs->eax = 0;
	//		break;
		case SYS_READ:
		//	printf("read\n");
			regs->eax = sys_read((int)regs->ebx, (char *)regs->ecx, (int)regs->edx);
		//	printf("read %x %X %X\n",regs->ebx, regs->ecx, regs->edx);
		//	memset((void *)thread_current()->brk - 400, 0xcc, 400);
		//	strcpy((char *)regs->ecx, "/forktest\n");
		//	regs->eax = 10;
			break;
		case SYS_WRITE:
			regs->eax = sys_write(0, (char *)regs->ecx, (int)regs->edx);
		//	printf("\n");
		//	printf("write %x %X %X\n",regs->ebx, regs->ecx, regs->edx);
			break;
		case SYS_OPEN:
		//	printf("path %s\n",(char *)regs->ebx);
			regs->eax = sys_open((char *)regs->ebx, 0, NULL);
			return;
		case SYS_GETPID:
			regs->eax = sys_getpid();
			return;
		case SYS_FORK:
			printf("forking\n");
			regs->eax = sys_fork(regs);
			return;
		case SYS_EXECVE:
		//	printf("exec\n");
			regs->eax = sys_execve((char *)regs->ebx, (char **)regs->ecx, (char **)regs->edx);
			return;
		case SYS_BRK:
			regs->eax = (uintptr_t)sys_brk(regs->ebx);
			return;
		case 146://writev(int fd, struct iovec *vec, int count)
			//dump_regs(regs);
		//	printf("%s", *(int *)regs->ecx);
			regs->eax = -1;
		//	printf("%i\n", regs->edx);
			sys_write(regs->ebx, (char *)*(int *)regs->ecx, *((int*)regs->ecx + 1));
		//	printf("\n");
			//sys_write(0, (char *()*((int*)regs->ecx + 2), *((int*)regs->ecx + 3));
			return;
		case 54:
			regs->eax = sys_ioctl((int)regs->ebx, (int)regs->ecx, NULL);
			return;
		case 57:
			regs->eax  =0;
			return;
		case 65:
			regs->eax = 0;
			return;
		case 37:
			sys_kill((int)regs->ebx, (int)regs->ecx);
			regs->eax = 0;
			return;
//		case 252:
		case SYS_CLOSE:
			regs->eax = sys_close(regs->ebx);
			break;
		case SYS_EXIT:
			printf("exit (%i)\n",regs->ebx);
			thread_exit();
			return;
		case 174:
			//if(regs->ecx != 0 && regs->edx != 0 && regs->ebx <20)
		//	if(regs->ebx == 21)
			printf("%i %x %x\n", regs->ebx, *(int *)regs->ecx, *(int *)regs->edx);
			break;
		case 200:
			thread_scheduler(regs);
			break;
		case SYS_DUMMY:

			regs->eax = sys_dummy();
			break;	
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


