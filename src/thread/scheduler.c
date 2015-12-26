/*	ChickenOS - thread/scheduler.c
 *	Very basic scheduler, called on every timer interrupt
 */
#include <common.h>
#include <kernel/thread.h>
#include <kernel/memory.h>
#include <device/console.h>
#include <kernel/interrupt.h>
#include <mm/vm.h>
#include <thread/syscall.h>
#include <stdio.h>
#include <util/utlist.h>

thread_t *thread_list = NULL;

extern void thread_reschedule(registers_t *regs, thread_t *cur, thread_t *next);

void exit(int i)
{
	printf("Exit %i\n", i);
	PANIC("exit() called!");
}

void thread_scheduler_init(thread_t *kernel_thread)
{
	HASH_ADD_INT(thread_list, pid, kernel_thread);
	thread_set_ready(kernel_thread);
}

void thread_scheduler(registers_t *regs)
{
	thread_t *cur = thread_current();
	thread_t *next;

	if(cur->status == THREAD_UNINTERRUPTIBLE)
	{
		thread_reschedule(regs, cur, cur);
	}
	next = thread_next();

	if(next == NULL)
		next = cur;

	//save position of regs on stack for signal handling
	cur->regs = regs;
	//serial_printf("next %X %X %X\n", regs, cur, next);
	thread_reschedule(regs, cur, next);
}

void thread_set_ready(thread_t *thread)
{
	THREAD_ASSERT(thread);
	thread->status = THREAD_READY;
}

void thread_queue(thread_t *thread)
{
	THREAD_ASSERT(thread);
	HASH_ADD_INT(thread_list, pid, thread);
}

thread_t *thread_dequeue()
{
	thread_t *ret = thread_current();

	return ret;
}

//TODO: Implement this as a heap-based priority queue
thread_t *thread_next()
{
	thread_t *next, *cur = thread_current();

	next = cur->hh.next;

	if(next == NULL)
		next = thread_list;

	while(next->status != THREAD_READY)
	{
		next = next->hh.next;
		if(next == NULL)
			next = thread_list;

	}

	return next;
}

thread_t *thread_by_pid(pid_t pid)
{
	thread_t *ret = NULL;
	HASH_FIND_INT(thread_list, &pid, ret);
	return ret;
}


void thread_exit(int status)
{
	thread_t *cur = thread_current();
	thread_t *next = thread_next();
//	thread_t *parent;

	interrupt_disable();

//	parent = thread_by_pid(cur->ppid);

	HASH_DEL(thread_list, cur);

	//printf("exit (%i)\n",status);

	//Check if we have children
	//if we do
	//	re parent them

	sys_kill(cur->ppid, SIGCHLD);

	//Will need to reap eventually
	cur->status = THREAD_DEAD;
	cur->ret_val = status;
	thread_reschedule(cur->regs, cur, next);
}


