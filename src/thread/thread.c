/*	ChickenOS - thread.c
 *  Threading system
 *
 */
#include <common.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <mm/vm.h>
#include <mm/paging.h>
#include <kernel/thread.h>
#include <kernel/memory.h>
#include <kernel/interrupt.h>
#include <mm/liballoc.h>
#include <util/utlist.h>

int thread_stack_offset = offsetof(thread_t, sp);

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
			//new->files[i] = vfs_reopen(old->files[i]);
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

	kernel_thread->mm = mm_alloc();
	kernel_thread->file_info = thread_files_alloc(NULL);
	kernel_thread->sig_info = kcalloc(sizeof(struct thread_signals), 1);

	kernel_thread->name = strdup("idle");

	//HACK: This wastes a page, but in exchange for cleaner code on fork()
//	memregion_add(kernel_thread, PHYS_BASE - PAGE_SIZE, PAGE_SIZE, PROT_GROWSDOWN,
//			MAP_GROWSDOWN | MAP_FIXED, NULL, NULL);
//	memregion_add(kernel_thread, HEAP_BASE, PAGE_SIZE, PROT_GROWSUP,
//			MAP_PRIVATE | MAP_FIXED, NULL, NULL);

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

	//FIXME: Assign things instead of copying the struct
	kmemcpy(new, cur, sizeof(thread_t));

	new->ppid = cur->pid;
	new->pgid = cur->pgid;
	new->pid = pid_allocate();

	new->magic = THREAD_MAGIC;
	new->mm = mm_clone(cur->mm);
	new->file_info = thread_files_alloc(cur->file_info);
	new->sig_info = kcalloc(sizeof(struct thread_signals), 1);

	kmemcpy(new->sig_info->signals, cur->sig_info->signals,
			sizeof(struct k_sigaction) * NUM_SIGNALS);

	new->name = strdup(cur->name);

	thread_add_child(cur, new);
	return new;
}

//The lazy way of copying most shit from a passed in regs, works
//but better to just always build a stack
pid_t
thread_create(registers_t *_regs, void (*eip)(void *), void * esp UNUSED)
//thread_create(void (*eip)(void *), void *aux, int flags)
{
	enum intr_status old_level = interrupt_disable();
	thread_t *new, *cur;
	uint8_t *kernel_stack;
	uintptr_t new_sp;

	cur = thread_current();
	new = thread_clone(cur);

	kernel_stack = (uint8_t *)new;
	cur->children = new;

	new_sp = (uintptr_t)kernel_stack + STACK_SIZE;

	if(_regs != NULL)
	{
		thread_copy_stackframe(cur, kernel_stack, 0);
	}
	else
	{
		uint32_t *usersp = palloc();
		memregion_map_data(new->mm, PHYS_BASE - PAGE_SIZE, PAGE_SIZE, PROT_GROWSDOWN,
			MAP_GROWSDOWN | MAP_FIXED, usersp);

		usersp += 1024;
		*--usersp = (uint32_t)esp;

		thread_build_stackframe(kernel_stack, (uintptr_t)eip, PHYS_BASE);
	}


	new->sp = (uint8_t *)(new_sp - (sizeof(registers_t) + 4));

	thread_set_ready(new);
	thread_queue(new);

	interrupt_set(old_level);

	return new->pid;
}

//TODO: Array? Bitmap?
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
