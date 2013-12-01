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
	typedef long * arg56;

//TODO: need to verify pointers before letting functions dereference them
void syscall_handler (struct registers *regs)
{
#ifdef ARCH_ARM
	(void)regs;
#else
	int call = regs->eax;
	struct vec {
				void *shit;
		size_t len;
			};
	int count;
	struct vec *ass; 
//	char *buf = NULL;
//	buf = buf;
	//	if(call != 45)
	long *arg;
	//printf("call %i\n",call);
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
		//getppid
		case 64:
			regs->eax = 2;
			return;
		//readv
		case 145:
			ass = (void *)regs->ecx;
			if((int)regs->edx < 0)
			{
				regs->eax = -1;
				return;
			}	
			regs->eax = 0;
		//	if(regs->ebx < 2)
		//		regs->edx = 1;
		//	uint8_t * buf;

	
	
	for(int i = 0; i < (int)regs->edx; i++)
			{
			
			//	printf("address %p len %i\n", ass[i]->shit, ass[i]->len);
			}
			for(int i = 0; i < (int)regs->edx; i++)
			{
				//	printf("address %p len %i\n", ass[i]->shit, ass[i]->len);
			//	if(ass[i]->shit == NULL)
			//	{
				//	dump_regs(regs);
				//	regs->eax++;
			//	}else
				if(ass[i].len == 0)
					continue;
			//	if(regs->ebx == 0)
				{//	strcpy(ass[i]->shit, "ass\n");
				//	regs->eax = strlen(ass[i]->shit) + 1;

				//	break;
				}

			//	memset(ass[i]->shit, 0, ass[i]->len);
			//	printf("fd %i\n", regs->ebx);
				count = sys_read((int)regs->ebx, ass[i].shit, ass[i].len);
			//	printf("ret = %u\n", count);
			//	if(count >= 0)
		//	printf("count %i %x\n", regs->eax, * (char *)ass[i]->shit);
				regs->eax += count;
		//	printf("OUT %s\n", ass[i]->shit);	
		}
			//if(regs->eax == 20) regs->eax =21;
	//if(regs->ebx < 2)
	//		regs->eax = 1024;	
		return;

		//writev
		case 146:
			ass = (void *)regs->ecx;
			if((int)regs->edx < 0)
			{
				regs->eax = -1;
				return;
			}	
			regs->eax = 0;
			for(int i = 0; i < (int)regs->edx; i++)
			{
				//	printf("address %p len %i\n", ass[i]->shit, ass[i]->len);
			//	if(ass[i]->shit == NULL)
			//	{
				//	dump_regs(regs);
				//	regs->eax++;
			//	}else
				count = sys_write((int)regs->ebx, ass[i].shit, ass[i].len);
			//	printf("ret = %u\n", count);
				if(count >= 0)
				regs->eax += count;
			}
		//	printf("count %i\n", regs->eax);
			//if(regs->eax == 20) regs->eax =21;
			return;
		case SYS_OPEN:
			regs->eax = sys_open((char *)regs->ebx, regs->ecx, (mode_t)regs->edx);
			return;
		case SYS_LSEEK:
			regs->eax = sys_lseek((int)regs->ebx, (off_t)regs->ecx, (int) regs->edx);
			return;
		case 140:
			regs->eax = sys_lseek((int)regs->ebx, (off_t)regs->edx, (int) regs->edi);
			*(long *)regs->esi = regs->eax;
			return;
		case SYS_STAT64:
			regs->eax = sys_stat64((char *)regs->ebx, (struct stat64 *)regs->ecx);
			break;
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
			return;
		case SYS_EXECVE:
			regs->eax = sys_execve((char *)regs->ebx, (char **)regs->ecx, (char **)regs->edx);
			//FIXME: Is this needed?
			//thread_yield();
			return;
		case SYS_BRK:
			regs->eax = (uint32_t)sys_brk((void *)regs->ebx);
			return;
	//	case SYS_SBRK:
		//	regs->eax = (uintptr_t)sys_sbrk(regs->ebx);
		//	return;
		case SYS_IOCTL:
			regs->eax = sys_ioctl((int)regs->ebx, (int)regs->ecx, (uint32_t)regs->edx);
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
			thread_exit();
			return;
		case SYS_GETTIMEOFDAY:
			regs->eax = sys_gettimeofday((struct timeval *)regs->ebx, (void *)regs->ecx);
			return;
		case SYS_GETCWD:
			regs->eax = (uint32_t)sys_getcwd((char *)regs->ebx, (size_t)regs->ecx);
			printf("ret cwd %x\n", regs->eax);
			return;
		case SYS_MMAP2:
			arg = (void *)regs->useresp-8;
			printf("arg %x\n", regs->edi);
			dump_regs(regs);
			regs->eax = (uint32_t)sys_mmap2((void *)regs->ebx, (size_t)regs->ecx, (int)regs->edx, 
											(int)regs->esi, (int)arg[0], (off_t)arg[1]);
			return;
				//case SYS_NETWORK:
			//regs->eax = sys_network_setup();
			//break;
		case SYS_DUMMY:
			printf("DUMMY\n");
			//regs->eax = sys_dummy();
			break;
		case 29:
			while(1);
		//access
		case 33:
			regs->eax = 0;
			return;
		//wait4
		case 114:
		//rt_sigsuspend	
		case 179: case 168: case 174: case 175:
			regs->eax = 0;//ENOSYS;
			return;
		case SYS_CLOCK_GETTIME:
			regs->eax = sys_clock_gettime((int)regs->ebx, (struct timespec *)regs->ecx);
			return;
		case SYS_FUTEX:
			printf("Futexes not yet supported, halting process\n");
			printf("op %x\n", regs->ecx);
			while(1);
			regs->eax = 0;
			return;
		default:
			printf("undefined system call %i!\n",call);
			regs->eax = 0;//ENOSYS;
			return;
	}
	#endif
}

void syscall_init()
{
	interrupt_register(0x80, &syscall_handler);
}
