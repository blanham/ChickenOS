/*	ChickenOS - thread.c
 *  Threading system
 *
 */
#include <common.h>
#include <stdint.h>
#include <string.h>
#include <mm/vm.h>
#include <mm/paging.h>
#include <kernel/thread.h>
#include <kernel/memory.h>
#include <kernel/interrupt.h>
#include <mm/liballoc.h>


struct sigaction *default_signals[32];
char *_main_thread_name = "idle";


void thread_init()
{
	thread_t *kernel_thread;
	
	interrupt_disable();

	//do this inline, otherwise assertion fails in thread_current()	
	stackpointer_get(kernel_thread);

	kernel_thread = (thread_t *) ((uintptr_t)kernel_thread & ~(STACK_SIZE -1));
	
	kernel_thread->pid = 0;
	kernel_thread->parent = 0;
	kernel_thread->pgid = 0;

	kernel_thread->pd = pagedir_new();
	kernel_thread->signals = default_signals;

	//FIXME:
	kernel_thread->name = _main_thread_name;
	kernel_thread->magic = THREAD_MAGIC;

	thread_scheduler_init(kernel_thread);

	arch_thread_init();
}

thread_t *
thread_new()
{
	thread_t *new = pallocn(STACK_PAGES);
	kmemset(new, 0, STACK_SIZE);
	new->magic = THREAD_MAGIC;
	return new;
}


thread_t *
thread_clone(thread_t *cur)
{
	thread_t *new;
	
	new = thread_new();
	kmemcpy(new, cur, sizeof(thread_t));

	new->parent = cur->pid;
	new->pgid = cur->pgid;
	new->pid = pid_allocate();
	
	new->cur_dir = cur->cur_dir;

	//FIXME: Should be a vfs call that increases reference counts
	//FIXME: Also should be more than 8 files	
	kmemcpy(new->files, cur->files, sizeof(struct file)*8);

	new->signals = (struct sigaction **)
		kcalloc(sizeof(struct sigaction*), NUM_SIGNALS);
	kmemcpy(new->signals, cur->signals, sizeof(struct sigaction*) * NUM_SIGNALS);

	new->magic = THREAD_MAGIC;
	new->name = strdup(cur->name);
	return new;
}

//The lazy way of copying most shit from a passed in regs, works
//but better to just always build a stack

//FIXME: doesn't copy open files over to new process in the case of a fork
pid_t 
thread_create(registers_t *regs, void (*eip)(void *), void * esp)
{
	thread_t *new, *cur;
	enum intr_status old_level;
	registers_t *reg_frame;
	uint8_t *kernel_stack, *user_stack;
	uintptr_t new_sp;
	uint32_t *usersp;	
	old_level = interrupt_disable();
	
	cur = thread_current();
	new = thread_clone(cur);
	
	kernel_stack = (uint8_t *)new;
	
	//Allocates one page, if we page fault under the stack pointer we add more
	user_stack = palloc();
	kmemset(user_stack,0, 4096);

	usersp = (uint32_t *)(user_stack + 4096);
	new_sp = (uintptr_t)kernel_stack + STACK_SIZE;
	
	new->pd = pagedir_clone(cur->pd);
	pagedir_insert_page(new->pd, (uintptr_t)user_stack, 
		(uintptr_t)PHYS_BASE - 0x1000, 0x7);
	
	reg_frame = (void *)((kernel_stack + STACK_SIZE) - sizeof(*reg_frame));
	new->regs = (struct registers *)reg_frame;
	
	if(regs != NULL)
	{
		kmemcpy(user_stack, (void *)(PHYS_BASE - 0x1000), 0x1000);	
		kmemcpy(reg_frame, regs, sizeof(registers_t));		
		//this is a fork, so we want to be 0
		reg_frame->eax = 0;	
		new->user = (void *)(PHYS_BASE - 0x1000);
	}
	else
	{
		usersp--;
		*usersp = (uint32_t)esp;

		//Build new register frame
		reg_frame->eip = (uintptr_t)eip;
		reg_frame->ebp = PHYS_BASE - 16;	
		reg_frame->useresp = PHYS_BASE - 8;
		reg_frame->cs = 0x1b;
		reg_frame->ds = reg_frame->es = reg_frame->fs = 
		reg_frame->gs = reg_frame->ss = 0x23;
		reg_frame->eflags = 0x200;
	}
	
//	dump_regs(reg_frame);	

	new->sp = (uint8_t *)(new_sp - (sizeof(registers_t) + 4));
	
	thread_set_ready(new);
	thread_queue(new);

	interrupt_set(old_level);

	return new->pid;	
}

pid_t pid_allocate()
{
	static pid_t pid_count = 0;
	
	pid_count++;

	return pid_count;
}

thread_t * thread_current()
{
	thread_t *ret; 
	
	stackpointer_get(ret);

	ret = (thread_t *) ((uintptr_t)ret & ~(STACK_SIZE -1));
	
	THREAD_ASSERT(ret);

	return ret;
}


