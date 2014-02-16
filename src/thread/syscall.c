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
#include <thread/syscall-names2.h>
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
	char *envp[] = {"PATH=/", NULL};
	return SYSCALL_3N(SYS_EXECVE, path, argv, envp);
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
#define DEBUG
//TODO: need to verify pointers before letting functions dereference them
void syscall_handler (struct registers *regs)
{
	int call = regs->eax;
	struct vec {
				void *shit;
		size_t len;
			};
	int count;
	struct vec *ass; 
	long *arg;

#ifdef DEBUG
	//printf("call %i %i\t", call, sizeof(syscall_names));
	serial_printf("%s(%i): by %i @ %x\n",syscall_names[call], call, thread_current()->pid,
			regs->eip);
#endif
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

		case SYS_GETPPID:
			regs->eax = sys_getppid();
			break;
		case 142:
			regs->eax = 7;
			break;
		case SYS_POLL:
			regs->eax = 7;
			break;
		case SYS_READV:
			ass = (void *)regs->ecx;
			if((int)regs->edx < 0)
			{
				regs->eax = -1;
				break;
			}	
			regs->eax = 0;
			for(int i = 0; i < (int)regs->edx; i++)
			{
				if(ass[i].len == 0)
					continue;

				count = sys_read((int)regs->ebx, ass[i].shit, ass[i].len);
				//if(count >= 0)
				regs->eax += count;
			}
			break;

		case SYS_WRITEV:
			ass = (void *)regs->ecx;
			if((int)regs->edx < 0)
			{
				regs->eax = -1;
				break;
			}	
			regs->eax = 0;
			for(int i = 0; i < (int)regs->edx; i++)
			{
				count = sys_write((int)regs->ebx, ass[i].shit, ass[i].len);
				//printf("len %i po %p fd %i\n", ass[0].len, ass[0].shit, regs->edx);
				if(count >= 0)
				regs->eax += count;
			}
			break;
		case SYS_OPEN:
			regs->eax = sys_open((char *)regs->ebx, regs->ecx, (mode_t)regs->edx);
			break;
		case SYS_LSEEK:
			regs->eax = sys_lseek((int)regs->ebx, (off_t)regs->ecx, (int) regs->edx);
			break;
		case 140:
			regs->eax = sys_lseek((int)regs->ebx, (off_t)regs->edx, (int) regs->edi);
			*(long *)regs->esi = regs->eax;
			break;
		case SYS_STAT64:
			regs->eax = sys_stat64((char *)regs->ebx, (struct stat64 *)regs->ecx);
			break;
		case SYS_STAT:
			regs->eax = sys_stat((char *)regs->ebx, (struct stat *)regs->ecx);
			break;
		case SYS_GETPID:
			regs->eax = sys_getpid();
			break;
		case SYS_GETPGRP:
			regs->eax = sys_getpgrp();
			break;
		case SYS_SETPGID:
			regs->eax = sys_setpgid(regs->ebx, regs->ecx);
			break;

		case SYS_GETEUID:
		case SYS_GETEUID16:
			regs->eax = sys_geteuid();
			break;
		case SYS_GETUID:
			regs->eax = sys_getuid();
			break;
		case SYS_SETUID:
			regs->eax = sys_setuid(regs->ebx);
			break;
		case SYS_SETGID:
			regs->eax = sys_setgid(regs->ebx);
			break;
		case SYS_GETGID:
			regs->eax = sys_getgid();
			break;
		case SYS_FORK:
			regs->eax = sys_fork(regs);
			break;
		case SYS_EXECVE:
			regs->eax = sys_execve((char *)regs->ebx, (char **)regs->ecx, (char **)regs->edx);
			break;
		case SYS_BRK:
			regs->eax = (uint32_t)sys_brk((void *)regs->ebx);
			break;
	//	case SYS_SBRK:
		//	regs->eax = (uintptr_t)sys_sbrk(regs->ebx);
		//	break;
		case SYS_IOCTL:
			regs->eax = sys_ioctl((int)regs->ebx, (int)regs->ecx, (char *)regs->edx);
			break;
		case SYS_KILL:
			regs->eax = sys_kill(regs->ebx, regs->ecx);
			break;
		case SYS_RT_SIGACTION:
		case SYS_SIGACTION:
			regs->eax = sys_sigaction(regs->ebx, (void*)regs->ecx, (void *)regs->edx);
			break;
		case SYS_RT_SIGPROCMASK:
			regs->eax = sys_sigprocmask(regs->ebx, (void *)regs->ecx, (void *)regs->edx);
			break;
		case SYS_RT_SIGSUSPEND:
		case SYS_SIGSUSPEND:
			printf("IN syscall\n");
			dump_regs(regs);
			regs->eax = sys_sigsuspend((void*)regs->ebx);
			break; 
		//case 173:
		case SYS_SIGRETURN:
			regs->eax = sys_sigreturn(regs, regs->ebx);
			break;
		case SYS_WAIT4:
			regs->eax = sys_wait4((pid_t)regs->ebx, (int *)regs->ecx, (int)regs->edx, 
											(struct rusage *)regs->esi);
			//printf("REturn %i\n", regs->eax);
			break;
		case SYS_CLOSE:
			regs->eax = sys_close(regs->ebx);
			break;
		case SYS_FCNTL64:
		case SYS_FCNTL:
			regs->eax = 0;
			break;
		case SYS_EXIT:
			thread_exit(regs->ebx);
			PANIC("THREAD_EXIT returned!\n");
			break;
		case SYS_GETTIMEOFDAY:
			regs->eax = sys_gettimeofday((struct timeval *)regs->ebx, (void *)regs->ecx);
			break;
		case SYS_GETCWD:
			regs->eax = (uint32_t)sys_getcwd((char *)regs->ebx, (size_t)regs->ecx);
			break;
		case SYS_MMAP2:
			arg = (void *)regs->useresp-8;
			//printf("arg %x\n", regs->edi);
			dump_regs(regs);
			regs->eax = (uint32_t)sys_mmap2((void *)regs->ebx, (size_t)regs->ecx, (int)regs->edx, 
											(int)regs->esi, (int)arg[0], (off_t)arg[1]);
			break;
				//case SYS_NETWORK:
			//regs->eax = sys_network_setup();
			//break;
		case SYS_DUMMY:
			printf("DUMMY\n");
			//regs->eax = sys_dummy();
			break;
		case SYS_CLOCK_GETTIME:
			regs->eax = sys_clock_gettime((int)regs->ebx, (struct timespec *)regs->ecx);
			break;
		case SYS_FUTEX:
			printf("Futexes not yet supported, halting process\n");
			printf("op %x\n", regs->ecx);
			while(1);
			regs->eax = 0;
		default:
#ifdef DEBUG
			serial_printf("undefined system call\n");
#endif
//	printf("undefined system call %i!\n",call);
			regs->eax = -ENOSYS;
	}
#ifdef DEBUG
	serial_printf("%8i/%.8x\n", regs->eax, regs->eax);
#endif
}

void syscall_init()
{
	interrupt_register(0x80, &syscall_handler);
}
