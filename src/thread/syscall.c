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

int sys_dummy()
{
//	PANIC("YAY!");
	printf("dummy syscall\n");
	return 0xcafebabe;	
}

void *sys_brk(uintptr_t ptr)
{
	thread_t *cur = thread_current();
	printf("brk %x incr %x\n", cur->brk, ptr);
	if(ptr == 0)
		return cur->brk;
	else
	{
		cur->brk = (void *)ptr;
		printf("set brk to %x\n", cur->brk);
		return cur->brk;	
	}
}
extern void test_signals();
void syscall_handler (struct registers *regs)
{
	int call = regs->eax;
	char *buf = NULL;
	buf = buf;
	switch (call)
	{
		
	//	case SYS_PUTS:
	//		printf("%s", (char *)regs->ebx);
	//		regs->eax = 0;
	//		break;
		case SYS_READ:
			regs->eax = sys_read((int)regs->ebx, (char *)regs->ecx, (int)regs->edx);
			break;
		case SYS_WRITE:
			regs->eax = sys_write(0, (char *)regs->ecx, (int)regs->edx);
			printf("\n");
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
		case SYS_EXECVE:
			regs->eax = sys_execve((char *)regs->ebx, (char **)regs->ecx, (char **)regs->edx);
			return;
		case SYS_BRK:
			regs->eax = (uintptr_t)sys_brk(regs->ebx);
			printf("trying to set brk to %X\n",regs->ebx);
			return;
		case 146://writev(int fd, struct iovec *vec, int count)
			//dump_regs(regs);
		//	printf("%s", *(int *)regs->ecx);
			regs->eax = -1;
			printf("%i\n", regs->edx);
			sys_write(regs->ebx, (char *)*(int *)regs->ecx, *((int*)regs->ecx + 1));
			//sys_write(0, (char *()*((int*)regs->ecx + 2), *((int*)regs->ecx + 3));
			return;
		case 54:
			regs->eax = sys_ioctl((int)regs->ebx, (int)regs->ecx, NULL);
			return;
		case 65:
	//		regs->eax = 12;
			return;
		case 37:
			//printf("kill %i %i\n",regs->ebx, regs->ecx);
			//printf("eip %X\n",regs->eip);
			regs->eax = sys_kill((int)regs->ebx, (int)regs->ecx);
			regs->eax = 0;
			return;
//		case 252:
//		case SYS_EXIT:
//			thread_exit();
//			return;
		case 174:
			if(regs->ecx != 0 && regs->edx != 0 && regs->ebx <20)
			printf("%i %x %x\n", regs->ebx, *(int *)regs->ecx, *(int *)regs->edx);
			break;
		case 200:
			thread_scheduler(regs);
			//printf("test\n");
		//	test_signals();
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


