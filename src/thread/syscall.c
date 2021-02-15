#include <errno.h>
#include <stdio.h>
#include <chicken/common.h>
#include <chicken/thread.h>
#include <chicken/thread/signal.h>
#include <chicken/thread/syscall.h>
#include <chicken/thread/syscall-names.h>
#include <chicken/time.h>
//#include <fcntl.h>

#define DEBUG


void syscall_handler (registers_t *regs)
//int syscall_handler (uint32_t call, void *arg0, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5)
{
	(void)regs;
	/*
	int call = regs->eax;
	long *arg;

#ifdef DEBUG
	serial_printf("Regs-> %p\n", regs);
	serial_printf("%s(%i): by %i @ %x: ",syscall_names[call], call, thread_current()->pid, regs->eip);
	serial_printf("%x, %x, %x, %x, %x, %x\n", regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi, regs->ebp);
#endif

	switch (call)
	{
		case SYS_EXIT:
			thread_exit(regs->ebx);
			PANIC("THREAD_EXIT returned!\n");
			break;
		case SYS_READ:
			regs->eax = sys_read((int)regs->ebx, (char *)regs->ecx, (int)regs->edx);
			break;
		case SYS_WRITE:
			regs->eax = sys_write((int)regs->ebx, (char *)regs->ecx, (int)regs->edx);
			break;
		case SYS_CLOSE:
			regs->eax = sys_close(regs->ebx);
			break;
		case SYS_DUP:
			regs->eax = sys_dup(regs->ebx);
			break;
		case SYS_GETPPID:
			regs->eax = sys_getppid();
			break;
		case 162://nanosleep()
			regs->eax = 0;
			break;
		case SYS_SELECT:
			arg = (void *)regs->useresp-8;
			regs->eax = sys_select(regs->ebx, (fd_set *)regs->ecx, (fd_set *)regs->edx, (fd_set *)regs->esi, (struct timeval *)regs->edi);
			break;
		case SYS_POLL:
			regs->eax = sys_poll((struct pollfd *)regs->ebx, regs->ecx, regs->edx);
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
		case SYS_CHDIR:
			regs->eax = sys_chdir((const char *)regs->ebx);
			break;
		case SYS_FACCESSAT:
			regs->eax = sys_faccessat(regs->ebx, (const char *)regs->ecx, regs->edx, regs->esi);
			break;
		case SYS_ACCESS:
			regs->eax = sys_access((const char *)regs->ebx, regs->ecx);
			break;
		case SYS_MKDIR:
			regs->eax = sys_mkdir((const char *)regs->ebx, regs->ecx);
			break;
		case SYS_LSEEK:
			regs->eax = sys_lseek((int)regs->ebx, (off_t)regs->ecx, (int) regs->edx);
			break;
		case SYS_LLSEEK:
			regs->eax = sys_llseek((int)regs->ebx, regs->ecx, regs->edx, (off_t *)regs->esi, regs->edi);
			break;
		case SYS_LSTAT64:
			regs->eax = sys_lstat64((char *)regs->ebx, (struct stat64 *)regs->ecx);
			break;
		case SYS_STAT64:
			regs->eax = sys_stat64((char *)regs->ebx, (struct stat64 *)regs->ecx);
			break;
		case SYS_FSTAT64:
			regs->eax = sys_fstat64((int)regs->ebx, (struct stat64 *)regs->ecx);
			break;
		// FIXME: This is a hack
		case SYS_FSTATAT64:
			regs->eax = 0;
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
		case SYS_GETEGID:
			regs->eax = sys_getegid();
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
			regs->eax = sys_fork();
			break;
		case SYS_EXECVE:
			regs->eax = sys_execve((char *)regs->ebx, (char **)regs->ecx, (char **)regs->edx);
			break;
		case SYS_BRK:
			regs->eax = (uint32_t)sys_brk((void *)regs->ebx);
			break;
		case SYS_SBRK:
			regs->eax = (uintptr_t)sys_sbrk(regs->ebx);
			break;
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
			registers_dump(regs);
			regs->eax = sys_sigsuspend((void*)regs->ebx);
			break;
		case SYS_RT_SIGRETURN:
		case SYS_SIGRETURN:
			regs->eax = sys_sigreturn(regs, regs->ebx);
			break;
		case SYS_CLONE:
			regs->eax = sys_clone(regs->ebx, (void *)regs->ecx, (int *)regs->edx, regs->esi, (int *)regs->edi);
			break;
		case SYS_WAIT4:
			regs->eax = sys_wait4((pid_t)regs->ebx, (int *)regs->ecx, (int)regs->edx, (struct rusage *)regs->esi);
			//printf("REturn %i\n", regs->eax);
			break;
		case SYS_GETDENTS64:
			regs->eax = sys_getdents((int)regs->ebx, (void *)regs->ecx, (unsigned int)regs->edx);
			break;
		case SYS_FCNTL64:
			regs->eax = sys_fcntl64(regs->ebx, regs->ecx, regs->edx);
			break;
		case SYS_GETTIMEOFDAY:
			regs->eax = sys_gettimeofday((struct timeval *)regs->ebx, (void *)regs->ecx);
			break;
		case SYS_READLINK:
			regs->eax = sys_readlink((const char *)regs->ebx, (char *)regs->ecx, regs->edx);
			break;
		case SYS_MUNMAP:
			regs->eax = sys_munmap((void *)regs->ebx, regs->ecx);
		case SYS_NEWUNAME:
			regs->eax = sys_uname((void *)regs->ebx);
			break;
		case SYS_MPROTECT:
			regs->eax = sys_mprotect((void *)regs->ebx, regs->ecx, regs->edx);
			break;
		case SYS_GETCWD:
			regs->eax = (uint32_t)sys_getcwd((char *)regs->ebx, (size_t)regs->ecx);
			break;
		case SYS_MMAP2:
			//arg = (void *)regs->useresp-8;
			//printf("arg %x\n", regs->edi);
			//dump_regs(regs);
			regs->eax = (uint32_t)sys_mmap2((void *)regs->ebx, (size_t)regs->ecx, (int)regs->edx,
											(int)regs->esi, (int)regs->edi, (off_t)regs->ebp);
			break;
		case SYS_FUTEX:
			regs->eax = sys_futex((int *)regs->ebx, regs->ecx, regs->edx, (const struct timespec *)regs->esi, (int *)regs->edi, regs->ebp);
			break;
		case SYS_SET_THREAD_AREA:
			regs->eax = sys_set_thread_area((void *)regs->ebx);
			break;
		case SYS_GET_THREAD_AREA:
			regs->eax = sys_get_thread_area((void *)regs->ebx);
			break;
		case SYS_SET_TID_ADDRESS:
			regs->eax = sys_set_tid_address((int *)regs->ebx);
			break;
		case SYS_CLOCK_GETTIME:
			regs->eax = sys_clock_gettime((int)regs->ebx, (struct timespec *)regs->ecx);
			break;
		// TODO: here are syscalls we won't support any time soon:
		case SYS_BPF: case SYS_SOCKET:
			regs->eax = -ENOSYS;
			break;
		default:
			printf("undefined system call: ");
			printf("%s(%i): by %i @ %x\n",syscall_names[call], call, thread_current()->pid, regs->eip);
			//PANIC("Unimplemented system call");
			// FIXME: This failing silently allows things to get further than they would otherwise
			regs->eax = -ENOSYS;
	}
#ifdef DEBUG
	serial_printf("%8i/%.8x\n", regs->eax, regs->eax);
#endif
	*/
}
