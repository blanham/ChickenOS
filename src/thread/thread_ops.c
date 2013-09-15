#include <common.h>
#include <sys/types.h>
#include <kernel/thread.h>

pid_t sys_fork(registers_t *regs)
{
	pid_t pid;
	thread_t *new;
	new = thread_create(regs, regs->eip, (PHYS_BASE - 4096) + (regs->useresp & 0xfff));
	if(new == NULL)
		return -1;
	pid = new->pid;
	return pid;
}

pid_t sys_getpid()
{
	thread_t *cur = thread_current();
	pid_t pid = cur->pid;
	printf("cur %X pid = %i\n",cur,pid);
	return pid;
}

pid_t sys_getpgrp()
{
	thread_t *cur = thread_current();
	pid_t pgid = cur->pgid;
//	printf("cur %X pid = %i\n",cur,pgid);
	return pgid;
}


//FIXME: Needs more error/bounds checking
void *sys_brk(uintptr_t ptr)
{
	void * old;
	thread_t *cur = thread_current();

	if(ptr == 0)
	{
		return cur->brk;
	}	
	else
	{
		old = cur->brk;
		cur->brk = cur->brk + ptr;
		return old;	
	}
}

