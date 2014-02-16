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
#include <util/utlist.h>

//XXX: Remove this when we reintegrate fs code
extern struct file *root;

struct thread_files *thread_files_alloc(struct thread_files *old)
{
	struct thread_files * new = kcalloc(sizeof(struct thread_files), 1);
	if(old)
	{
		kmemcpy(new, old, sizeof(struct thread_files));
		new->files = kcalloc(sizeof(struct file *), 32);
		new->files_flags = kcalloc(sizeof(int), 32);
		//FIXME: increment ref count on root and cur_dir inodes
		for(int i = 0; i < old->files_count; i++)
		{
			//new->files[i] = vfs_reopen(olf->files[i]);
			new->files[i] = old->files[i];
			//new->files_flags[i] = 0;
		}
	}
	else
	{
	//	new->root = new->cur = root->inode; 
		new->files_count = 32;
		new->files = kcalloc(sizeof(struct file *), 32);
		new->files_flags = kcalloc(sizeof(int), 32);
	}
	return new;
}

void thread_init()
{
	thread_t *kernel_thread;
	
	interrupt_disable();

	//do this inline, otherwise assertion fails in thread_current()	
	stackpointer_get(kernel_thread);
	kernel_thread = (thread_t *) ((uintptr_t)kernel_thread & ~(STACK_SIZE -1));

	kmemset(kernel_thread,0, sizeof(thread_t));	
	kernel_thread->magic = THREAD_MAGIC;

	kernel_thread->pd = pagedir_new();
	kernel_thread->file_info = thread_files_alloc(NULL); 
	kernel_thread->sig_info = kcalloc(sizeof(struct thread_signals), 1);

	kernel_thread->name = strdup("idle");

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

void
thread_add_child(thread_t *parent, thread_t *child)
{
	LL_APPEND2(parent->children, child, child_next);
}

thread_t *
thread_clone(thread_t *cur)
{
	thread_t *new = thread_new();

	//FIXME: Just manually move things over
	kmemcpy(new, cur, sizeof(thread_t));

	new->ppid = cur->pid;
	new->pgid = cur->pgid;
	new->pid = pid_allocate();
	
	new->file_info = thread_files_alloc(cur->file_info);
	new->sig_info = kcalloc(sizeof(struct thread_signals), 1);

	kmemcpy(new->sig_info->signals, cur->sig_info->signals, 
			sizeof(struct k_sigaction) * NUM_SIGNALS);

	new->name = strdup(cur->name);
	new->magic = THREAD_MAGIC;

	thread_add_child(cur, new);

	return new;
}

//The lazy way of copying most shit from a passed in regs, works
//but better to just always build a stack
pid_t 
thread_create(registers_t *regs, void (*eip)(void *), void * esp)
{
	thread_t *new, *cur;
	enum intr_status old_level;
	registers_t *reg_frame;
	uint8_t *kernel_stack, *user_stack;//, *signal_stack;
	uintptr_t new_sp;
	uint32_t *usersp;	
	old_level = interrupt_disable();
	
	cur = thread_current();
	new = thread_clone(cur);
	
	kernel_stack = (uint8_t *)new;
	cur->children = new;
	
	//Allocates one page, if we page fault under the stack pointer we add more
	user_stack = palloc();
	kmemset(user_stack,0, 4096);

	usersp = (uint32_t *)(user_stack + 4096);
	new_sp = (uintptr_t)kernel_stack + STACK_SIZE;


	new->pd = pagedir_clone(cur->pd);
	//FIXME: what if user stack is bigger?
	pagedir_insert_page(new->pd, (uintptr_t)user_stack, 
		(uintptr_t)PHYS_BASE - 0x1000, 0x7);

	reg_frame = (void *)((kernel_stack + STACK_SIZE) - sizeof(*reg_frame));
	new->regs = (struct registers *)reg_frame;
	
	if(regs != NULL)
	{
		//FIXME: what if user stack is bigger?
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
/*
void thread_add_child(thread_t *thread, thread_t *child)
{
}
*/
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
	if(ret->magic != THREAD_MAGIC)
	{	
		printf("PID %i\n", ret->pid);
		THREAD_ASSERT(ret);
	}
	return ret;
}


