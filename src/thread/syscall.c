#include <common.h>
#include <chicken/time.h>
#include <mm/liballoc.h>
#include <string.h>
#include <kernel/interrupt.h>
#include <device/console.h>
#include <kernel/thread.h>
#include <net/net_core.h>
#include <thread/syscall.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <thread/syscall-names2.h>

#define DEBUG

void syscall_handler (registers_t *regs)
//int syscall_handler (uint32_t call, void *arg0, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5)
{
	int call = regs->eax;
	long *arg;

#ifdef DEBUG
	serial_printf("%s(%i): by %i @ %x\n",syscall_names[call], call, thread_current()->pid,
			regs->eip);
#endif

	switch (call)
	{
		case SYS_READ:
			regs->eax = sys_read((int)regs->ebx, (char *)regs->ecx, (int)regs->edx);
			break;
		case SYS_WRITE:
			regs->eax = sys_write((int)regs->ebx, (char *)regs->ecx, (int)regs->edx);
			break;

		case SYS_GETPPID:
			regs->eax = sys_getppid();
			break;
		case 162://nanosleep()
			regs->eax = 0;
			break;
		case 142:
			regs->eax = regs->ebx;
			break;
		case SYS_POLL:
			regs->eax = 1;
			break;
		case SYS_READV:
			regs->eax = sys_readv((int)regs->ebx, (const struct iovec *)regs->ecx, (int)regs->edx);
			break;
		case SYS_WRITEV:
			regs->eax = sys_writev((int)regs->ebx, (const struct iovec *)regs->ecx, (int)regs->edx);
			break;
		case SYS_OPEN:
			regs->eax = sys_open((char *)regs->ebx, regs->ecx, (mode_t)regs->edx);
			break;
		case SYS_LSEEK:
			regs->eax = sys_lseek((int)regs->ebx, (off_t)regs->ecx, (int) regs->edx);
			break;
		case 140:
			regs->eax = sys_lseek((int)regs->ebx, (off_t)regs->edx, (int) regs->edi);
			*(long *)regs->esi = regs->eax; //XXX: What is this for?
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
		case 173:
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
			//dump_regs(regs);
			regs->eax = (uint32_t)sys_mmap2((void *)regs->ebx, (size_t)regs->ecx, (int)regs->edx,
											(int)regs->esi, (int)arg[0], (off_t)arg[1]);
			break;
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
			regs->eax = -ENOSYS;
	}
#ifdef DEBUG
	serial_printf("%8i/%.8x\n", regs->eax, regs->eax);
#endif
}
