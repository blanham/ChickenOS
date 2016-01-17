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

thread_t *ready_list;
thread_t *idle_thread = NULL;
thread_t *thread_table[1024];

thread_t *thread_list = NULL;

void scheduler_init(thread_t *_idle_thread)
{
	//Two options:
	//Idle thread lives on the ready list
	//thread_set_ready2(_idle_thread);
	//Or it lives in a idle_thread global:
	idle_thread = _idle_thread;
	idle_thread->status = THREAD_READY;
}

thread_t *thread_by_pid(pid_t pid)
{
	thread_t *ret = NULL;
	HASH_FIND_INT(thread_list, &pid, ret);
	return ret;
}
thread_t *other_thread = NULL;

void thread_set_ready(thread_t *thread)
{
	thread->status = THREAD_READY;
	if(ready_list == NULL)
	ready_list = thread;
	else
		other_thread = thread;
	//Might add an assertion that checks if thread is still on a blocked queue
	///XXX: add this
	//add_to_list(ready_list, thread);
}


thread_t *thread_next()
{
	thread_t *cur = thread_current();
	thread_t *next = ready_list;

	//TODO: Next thread selection logic

	if (next == NULL)
		next = idle_thread;

	if(cur == ready_list)
		next = other_thread;

	return next;
}


void thread_exit(int exit_code)
{
	thread_t *cur = thread_current();
	(void)exit_code;
	//stop interrupts

	//This should just do the notification of parent
	//reparenting of children to init


	//Remove thread from ready list

	cur->status = THREAD_DEAD;
	cur->ret_val = exit_code;
}

//Need usermode stack pointer for signal handling
//Might have to be an architecture specific thing?
void scheduler_run(registers_t *regs UNUSED)
{
	//A later optimization could be to still pass in the interrupt context,
	//since it's on the kernel stack we can mask and get the current thread,
	//saving a bit of overhead
	thread_t *cur = thread_current();

	if (cur->status == THREAD_UNINTERRUPTIBLE)
		return;

	//if (any threads on dead list) {
	//	thread_free(dead_thread);
	//}

	if (cur->status == THREAD_DEAD) {
		//put this thread on a dead list, will be cleaned up on next run of
		//scheduler
		//well, we need for it to hang around if someone needs the return code
	}

	thread_t *next = thread_next();
	//OR
	//thread_t *next = thread_dequeue();

	//XXX: Implement this
	//thread_queue(ready_list, cur);


	if (next == NULL) {
		next = idle_thread;
	}

	//Switch address spaces here?
	//mm_set_address_space(next->mm);

	//Handle signals here
	if (next->sig_info->signal_pending != 0) {
		//signal_setup_userstack(next);
	}

	arch_thread_reschedule(cur, next);
}



//XXX: I think this is used in the hash library?
//     Another reason to get rid of it (the hash library)
void exit(int i)
{
	printf("Exit %i\n", i);
	PANIC("exit() called!");
}

