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

//FIXME: This needs to be removed and a list added
thread_t *table[MAX_THREADS];
int thread_cnt = 1;
int thread_ptr = 0;
struct list;
struct list {
	struct list *next;
	void *data;
};

void thread_scheduler_init(thread_t *kernel_thread)
{
	kmemset(table, 0, MAX_THREADS*sizeof(thread_t*));
	table[0] = kernel_thread;
	thread_set_ready(kernel_thread);
}


extern void thread_reschedule(registers_t *regs, thread_t *cur, thread_t *next);
void thread_scheduler(registers_t *regs)
{
	thread_t *cur = thread_current();
	thread_t *next = thread_next();
	
	if(next == NULL)
		next = cur;

	//save position of regs on stack for signal handling
	cur->regs = regs;
/*
	//i believe that signals go here:
	//if(next->signal_pending != 0)
	//signal(regs, next);
*/	
	//	printf("Switching to pid %i from pid %i esp %x regs->esp %x %x\n\n",
	//	next->pid, cur->pid,_esp, regs->esp,  regs->eip, next->regs->eip);
/*
	printf("resg %x next->regs %x %X %X	%X %X\n\n", regs, next->regs, cur, next, cur->sp, _esp);
	dump_regs(regs);
	printf("\n");
	dump_regs(next->regs);
	printf("\n");
	*/
	thread_reschedule(regs, cur, next);
}

//FIXME: call a assembly function that builds a reg stack,
//		and then calls thread_scheduler()
/* throw an int 32, manually invoking the timer interrupt */
/* can we just call the scheduler using the saved refs in 
 * thread struct?
 */
/*void thread_yield()
{
	asm volatile("int $32");
}*/

void thread_set_ready(thread_t *thread)
{
	THREAD_ASSERT(thread);
	thread->status = THREAD_READY;
}

void thread_queue(thread_t *thread)
{
	THREAD_ASSERT(thread);
	table[thread_cnt] = thread;
	thread_cnt++;
}

thread_t *thread_dequeue()
{
	thread_t *ret = thread_current();

	return ret;
}

//TODO: Implement this as a heap-based priority queue
thread_t *thread_next()
{
	thread_t *next;
//	thread_t *cur = thread_current();
	if(thread_ptr > thread_cnt)
	thread_ptr = 0;

	next = table[thread_ptr++];
	
	if(next == NULL || next->status != THREAD_READY)
	{
		next = thread_next();
	}

	return next;
}


void thread_exit()
{
//	asm volatile("cli");
	thread_t *cur = thread_current();
	thread_t *next = thread_next();
	//keep a tmp pointer to next process
	//which the scheduler uses to get the next process
	//need to set a value in the thread_t to tell
	//the scheduler to do this
	
	//if we aren't the original kernel thread, free the stack
	if(cur->pid != 0)
	{
		//FIXME: This needs to be done differently
//		pallocn_free(cur, STACK_PAGES);	

	}
	cur->status = THREAD_DEAD;
//	asm volatile("sti");
	thread_yield();

	thread_reschedule(cur->regs, cur, next);
}


