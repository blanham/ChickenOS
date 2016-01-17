#include <common.h>
#include <sys/types.h>
#include <errno.h>
#include <kernel/thread.h>

pid_t sys_fork(registers_t *regs)
{
	return thread_create2(regs->eip, regs->useresp, NULL);
}

uid_t sys_geteuid()
{
	return thread_current()->euid;
}

int sys_setuid(uid_t uid)
{
	thread_current()->uid = uid;
	return 0;
}

uid_t sys_getuid()
{
	return thread_current()->uid;
}

pid_t sys_getpid()
{
	return thread_current()->pid;
}

pid_t sys_getppid()
{
	return thread_current()->ppid;
}

pid_t sys_getpgrp()
{
	thread_t *cur = thread_current();
	pid_t pgid = cur->pgid;
//	printf("cur %X pid = %i\n",cur,pgid);
	return pgid;
}

int sys_setpgid(pid_t pid, pid_t pgid)
{
	thread_t *thread = thread_current();
	(void)pid;
	thread->pgid = pgid;
	return 0;
}

gid_t sys_getgid()
{
	return thread_current()->gid;
}

int sys_setgid(gid_t gid)
{
	thread_t *cur = thread_current();
	cur->gid = gid;
	return 0;
}

//XXX: This and sbrk should be using the memregion interface
int sys_brk(void *_addr)
{
	thread_t *cur = thread_current();
	uintptr_t addr = (uintptr_t)_addr;
	if(addr == 0)
	{
		return (int)cur->mm->brk;
	}

	cur->mm->brk = _addr;
//	printf("addr %x cur %x\n", addr, cur->brk);
//	cur->brk = (void *)((uintptr_t)0x8000000 + (uintptr_t)addr);
	return (int)addr;
}

//FIXME: Needs more error/bounds checking
void *sys_sbrk(intptr_t ptr)
{
	void * old;
	thread_t *cur = thread_current();

	if(ptr == 0)
	{
		return cur->mm->brk;
	}
	else
	{
		old = cur->mm->brk;
		cur->mm->brk = cur->mm->brk + ptr;
		return old;
	}
}

void sys_exit(int exit_code)
{

	thread_exit(exit_code);
}

//TODO: For threading, exit()s all with the same tgid
void sys_exit_group(int status)
{
	(void)status;
}

//FIXME: Busy waiting :c
pid_t sys_wait4(pid_t pid, int *status, int options, struct rusage *rusage)
{
	(void)pid; (void)status; (void)options; (void)rusage;
	thread_t *cur = thread_current();
	thread_t *child;
	pid_t ret = 0;
//	printf("STATUS %i\n", *status);

	while(1)
	{
		LL_FOREACH2(cur->children, child, child_next)
		{
			ret = child->pid;
			if(child->status == THREAD_DEAD)
			{
				printf("Child %x %i\n", child, child->pid);
				cur->children = NULL;
				*status = 0x80;
				return ret;
			}

		}
		if(ret)
		{
			if(options & WNOHANG)
				return 0;
			//wait
			if(!sigismember(&cur->sig_info->pending, SIGCHLD))
				continue;
			else
				return -EINTR;
		}

		return -ECHILD;
	}
	PANIC("lulz\n");
	return -ECHILD;
}
