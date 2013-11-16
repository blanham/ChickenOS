#include <common.h>
#include <sys/types.h>
#include <kernel/thread.h>

pid_t sys_fork(registers_t *regs)
{
	return thread_create(regs, NULL, NULL);
}

pid_t sys_getpid()
{
	thread_t *cur = thread_current();
	pid_t pid = cur->pid;
//	printf("cur %X pid = %i\n",cur,pid);
	return pid;
}

pid_t sys_getpgrp()
{
	thread_t *cur = thread_current();
	pid_t pgid = cur->pgid;
//	printf("cur %X pid = %i\n",cur,pgid);
	return pgid;
}

int sys_brk(void *_addr)
{
	thread_t *cur = thread_current();
	uintptr_t addr = (uintptr_t)_addr; 
	if(addr == 0)
	{
		return (int)cur->brk;
	}
	
	cur->brk = _addr;
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
		return cur->brk;
	}	
	else
	{
		old = cur->brk;
		cur->brk = cur->brk + ptr;
		return old;	
	}
}

