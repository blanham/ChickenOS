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


// TODO: Not really sure what I want to do here
void syscall_handler (registers2_t *regs)
//int syscall_handler (uint32_t call, void *arg0, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5)
{
	(void)regs;
	int call = regs->return_value;
	long *arg;

#ifdef DEBUG
	serial_printf("Regs-> %p\n", regs);
	serial_printf("%s(%i): by %i @ %x: ",syscall_names[call], call, thread_current()->pid, regs->instruction_pointer);
	serial_printf("%x, %x, %x, %x, %x, %x\n", regs->arg0, regs->arg1, regs->arg2, regs->arg3, regs->arg4, regs->arg5);
#endif

	switch (call)
	{
		case SYS_EXIT:
			thread_exit(regs->arg0);
			PANIC("THREAD_EXIT returned!\n");
			break;
		case SYS_READ:
			regs->return_value = sys_read((int)regs->arg0, (char *)regs->arg1, (int)regs->arg2);
			break;
		case SYS_WRITE:
			regs->return_value = sys_write((int)regs->arg0, (char *)regs->arg1, (int)regs->arg2);
			break;
		case SYS_CLOSE:
			regs->return_value = sys_close(regs->arg0);
			break;
		case SYS_DUP:
			regs->return_value = sys_dup(regs->arg0);
			break;
		case SYS_GETPPID:
			regs->return_value = sys_getppid();
			break;
		case 162://nanosleep()
			regs->return_value = 0;
			break;
		case SYS_SELECT:
			arg = (void *)regs->useresp-8;
			regs->return_value = sys_select(regs->arg0, (fd_set *)regs->arg1, (fd_set *)regs->arg2, (fd_set *)regs->arg3, (struct timeval *)regs->arg4);
			break;
		case SYS_POLL:
			regs->return_value = sys_poll((struct pollfd *)regs->arg0, regs->arg1, regs->arg2);
			break;
		case SYS_READV:
			regs->return_value = sys_readv((int)regs->arg0, (const struct iovec *)regs->arg1, (int)regs->arg2);
			break;
		case SYS_WRITEV:
			regs->return_value = sys_writev((int)regs->arg0, (const struct iovec *)regs->arg1, (int)regs->arg2);
			break;
		case SYS_OPEN:
			regs->return_value = sys_open((char *)regs->arg0, regs->arg1, (mode_t)regs->arg2);
			break;
		case SYS_CHDIR:
			regs->return_value = sys_chdir((const char *)regs->arg0);
			break;
		case SYS_FACCESSAT:
			regs->return_value = sys_faccessat(regs->arg0, (const char *)regs->arg1, regs->arg2, regs->arg3);
			break;
		case SYS_ACCESS:
			regs->return_value = sys_access((const char *)regs->arg0, regs->arg1);
			break;
		case SYS_MKDIR:
			regs->return_value = sys_mkdir((const char *)regs->arg0, regs->arg1);
			break;
		case SYS_LSEEK:
			regs->return_value = sys_lseek((int)regs->arg0, (off_t)regs->arg1, (int) regs->arg2);
			break;
		case SYS_LLSEEK:
			regs->return_value = sys_llseek((int)regs->arg0, regs->arg1, regs->arg2, (off_t *)regs->arg3, regs->arg4);
			break;
		case SYS_LSTAT64:
			regs->return_value = sys_lstat64((char *)regs->arg0, (struct stat64 *)regs->arg1);
			break;
		case SYS_STAT64:
			regs->return_value = sys_stat64((char *)regs->arg0, (struct stat64 *)regs->arg1);
			break;
		case SYS_FSTAT64:
			regs->return_value = sys_fstat64((int)regs->arg0, (struct stat64 *)regs->arg1);
			break;
		// FIXME: This is a hack
		case SYS_FSTATAT64:
			regs->return_value = 0;
			break;
		case SYS_STAT:
			regs->return_value = sys_stat((char *)regs->arg0, (struct stat *)regs->arg1);
			break;
		case SYS_GETPID:
			regs->return_value = sys_getpid();
			break;
		case SYS_GETPGRP:
			regs->return_value = sys_getpgrp();
			break;
		case SYS_SETPGID:
			regs->return_value = sys_setpgid(regs->arg0, regs->arg1);
			break;
		case SYS_GETEGID:
			regs->return_value = sys_getegid();
			break;
		case SYS_GETEUID:
		case SYS_GETEUID16:
			regs->return_value = sys_geteuid();
			break;
		case SYS_GETUID:
			regs->return_value = sys_getuid();
			break;
		case SYS_SETUID:
			regs->return_value = sys_setuid(regs->arg0);
			break;
		case SYS_SETGID:
			regs->return_value = sys_setgid(regs->arg0);
			break;
		case SYS_GETGID:
			regs->return_value = sys_getgid();
			break;
		case SYS_FORK:
			regs->return_value = sys_fork();
			break;
		case SYS_EXECVE:
			regs->return_value = sys_execve((char *)regs->arg0, (char **)regs->arg1, (char **)regs->arg2);
			break;
		case SYS_BRK:
			regs->return_value = (uint32_t)sys_brk((void *)regs->arg0);
			break;
		case SYS_SBRK:
			regs->return_value = (uintptr_t)sys_sbrk(regs->arg0);
			break;
		case SYS_IOCTL:
			regs->return_value = sys_ioctl((int)regs->arg0, (int)regs->arg1, (char *)regs->arg2);
			break;
		case SYS_KILL:
			regs->return_value = sys_kill(regs->arg0, regs->arg1);
			break;
		case SYS_RT_SIGACTION:
		case SYS_SIGACTION:
			regs->return_value = sys_sigaction(regs->arg0, (void*)regs->arg1, (void *)regs->arg2);
			break;
		case SYS_RT_SIGPROCMASK:
			regs->return_value = sys_sigprocmask(regs->arg0, (void *)regs->arg1, (void *)regs->arg2);
			break;
		case SYS_RT_SIGSUSPEND:
		case SYS_SIGSUSPEND:
			printf("IN syscall\n");
			registers_dump(regs);
			regs->return_value = sys_sigsuspend((void*)regs->arg0);
			break;
		case SYS_RT_SIGRETURN:
		case SYS_SIGRETURN:
			regs->return_value = sys_sigreturn(regs, regs->arg0);
			break;
		case SYS_CLONE:
			regs->return_value = sys_clone(regs->arg0, (void *)regs->arg1, (int *)regs->arg2, regs->arg3, (int *)regs->arg4);
			break;
		case SYS_WAIT4:
			regs->return_value = sys_wait4((pid_t)regs->arg0, (int *)regs->arg1, (int)regs->arg2, (struct rusage *)regs->arg3);
			//printf("REturn %i\n", regs->return_value);
			break;
		case SYS_GETDENTS64:
			regs->return_value = sys_getdents((int)regs->arg0, (void *)regs->arg1, (unsigned int)regs->arg2);
			break;
		case SYS_FCNTL64:
			regs->return_value = sys_fcntl64(regs->arg0, regs->arg1, regs->arg2);
			break;
		case SYS_GETTIMEOFDAY:
			regs->return_value = sys_gettimeofday((struct timeval *)regs->arg0, (void *)regs->arg1);
			break;
		case SYS_READLINK:
			regs->return_value = sys_readlink((const char *)regs->arg0, (char *)regs->arg1, regs->arg2);
			break;
		case SYS_MUNMAP:
			regs->return_value = sys_munmap((void *)regs->arg0, regs->arg1);
		case SYS_NEWUNAME:
			regs->return_value = sys_uname((void *)regs->arg0);
			break;
		case SYS_MPROTECT:
			regs->return_value = sys_mprotect((void *)regs->arg0, regs->arg1, regs->arg2);
			break;
		case SYS_GETCWD:
			regs->return_value = (uint32_t)sys_getcwd((char *)regs->arg0, (size_t)regs->arg1);
			break;
		case SYS_MMAP2:
			//arg = (void *)regs->useresp-8;
			//printf("arg %x\n", regs->arg4);
			//dump_regs(regs);
			regs->return_value = (uint32_t)sys_mmap2((void *)regs->arg0, (size_t)regs->arg1, (int)regs->arg2,
											(int)regs->arg3, (int)regs->arg4, (off_t)regs->arg5);
			break;
		case SYS_FUTEX:
			regs->return_value = sys_futex((int *)regs->arg0, regs->arg1, regs->arg2, (const struct timespec *)regs->arg3, (int *)regs->arg4, regs->arg5);
			break;
		case SYS_SET_THREAD_AREA:
			regs->return_value = sys_set_thread_area((void *)regs->arg0);
			break;
		case SYS_GET_THREAD_AREA:
			regs->return_value = sys_get_thread_area((void *)regs->arg0);
			break;
		case SYS_SET_TID_ADDRESS:
			regs->return_value = sys_set_tid_address((int *)regs->arg0);
			break;
		case SYS_CLOCK_GETTIME:
			regs->return_value = sys_clock_gettime((int)regs->arg0, (struct timespec *)regs->arg1);
			break;
		// TODO: here are syscalls we won't support any time soon:
		case SYS_BPF: case SYS_SOCKET:
			regs->return_value = -ENOSYS;
			break;
		default:
			printf("undefined system call: ");
			printf("%s(%i): by %i @ %x\n",syscall_names[call], call, thread_current()->pid, regs->instruction_pointer);
			//PANIC("Unimplemented system call");
			// FIXME: This failing silently allows things to get further than they would otherwise
			regs->return_value = -ENOSYS;
	}
#ifdef DEBUG
	serial_printf("%8i/%.8x\n", regs->return_value, regs->return_value);
#endif
}
