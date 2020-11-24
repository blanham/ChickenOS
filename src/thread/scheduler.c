/*	ChickenOS - thread/scheduler.c
 *	Very basic scheduler, called on every timer interrupt
 */
#include <stdio.h>
#include <chicken/common.h>
#include <chicken/device/console.h>
#include <chicken/interrupt.h>
#include <chicken/lib/rbtree.h>
#include <chicken/mm/vm.h>
#include <chicken/thread.h>
#include <chicken/thread/syscall.h>
#include <util/utlist.h>

thread_t *ready_list = NULL;
thread_t *idle_thread;

thread_t *thread_table[1024];
thread_t *thread_list = NULL;

rbtree_t *thread_pid_tree = NULL;

void scheduler_init(thread_t *_idle_thread)
{
	idle_thread = _idle_thread;

	// Manually set this up, we don't want PID 0 in the tree
	idle_thread->status = THREAD_READY;

	thread_pid_tree = rbtree_alloc();
}

thread_t *thread_by_pid(pid_t pid)
{
	uintptr_t thread = rbtree_search(thread_pid_tree, pid);
	if (thread != 0xFFFFFFFF)
		return (thread_t *)thread;
	return NULL;
}

void thread_set_ready(thread_t *thread)
{
	thread->status = THREAD_READY;

	thread->next = ready_list;
	ready_list = thread;

	rbtree_insert_ptr(thread_pid_tree, thread->pid, thread);
	//Might add an assertion that checks if thread is still on a blocked queue
}

thread_t * thread_current()
{
	uintptr_t sp;
	stackpointer_get(sp);
	thread_t *ret = (thread_t *)(sp & STACK_MASK);
    THREAD_ASSERT(ret);
	return ret;
}

// TODO: Array? Bitmap? Possibly should be in scheduler instead?
pid_t pid_allocate()
{
	static pid_t pid_count = 0;

	pid_count++;

	return pid_count;
}

void thread_add_child(thread_t *parent, thread_t *child)
{
	LL_APPEND2(parent->children, child, child_next);
}

thread_t *thread_next()
{
	//thread_t *cur = thread_current();
	thread_t *next = ready_list;

	if (next == NULL)
		return NULL;

	//TODO: Next thread selection logic
	while (next->status != THREAD_READY) {
		next = next->next;
	}

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

	thread_yield();
}

void thread_yield()
{
	scheduler_run(NULL);
}

//Need usermode stack pointer for signal handling
//Might have to be an architecture specific thing?
void scheduler_run(registers_t *regs UNUSED)
{
	//A later optimization could be to still pass in the interrupt context,
	//since it's on the kernel stack we can mask and get the current thread,
	//saving a bit of overhead
	thread_t *cur = thread_current();

	//printf("Scheduling lol:\n");
	//dump_regs(regs);

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

	//registers_t *next_regs = (void*)((uintptr_t)next + STACK_SIZE - sizeof(registers_t));
	//printf("NEXT:\n");
	//dump_regs(next_regs);
	//Switch address spaces here?
	//mm_set_address_space(next->mm);

	pagedir_activate(next->mm->pd);
	//Handle signals here
	if (next->sig_info->signal_pending != 0) {
		//signal_setup_userstack(next);
	}
	//serial_printf("CUR: pid: %i tgid: %i Next: pid: %i tgid: %i\n", cur->pid, cur->tgid, next->pid, next->tgid);
	//if (cur->registers)
	//	registers_dump(cur->registers);
	//if (next->registers)
	//	registers_dump(next->registers);

	arch_thread_reschedule(cur, next);
}



//XXX: I think this is used in the hash library?
//     Another reason to get rid of it (the hash library)
void exit(int i)
{
	printf("Exit %i\n", i);
	PANIC("exit() called!");
}