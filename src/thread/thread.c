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

//struct process_tree_node {
//	struct process_tree_node *sibling;
//	struct process_tree_node *children;
//};
//struct process_tree_node process_tree_root;

// XXX: This should be removed
extern struct file *root;

int thread_stack_offset = offsetof(thread_t, sp);

static struct thread_files *thread_files_alloc(struct thread_files *old);

void threading_init()
{
	interrupt_disable();
	
	extern thread_t *kernel_thread;

	kernel_thread->magic = THREAD_MAGIC;
	kernel_thread->mm = mm_alloc();
	kernel_thread->file_info = thread_files_alloc(NULL);
	kernel_thread->sig_info = kcalloc(sizeof(struct thread_signals), 1);

	kernel_thread->name = strdup("idle");

	scheduler_init(kernel_thread);
}

void thread_add_child(thread_t *parent, thread_t *child)
{
	LL_APPEND2(parent->children, child, child_next);
}

//I think this should be thread_alloc, should put it on some not ready to run list?
thread_t *thread_alloc()
{
	thread_t *new = pallocn(STACK_PAGES);
	kmemset(new, 0, STACK_SIZE);
	new->magic = THREAD_MAGIC;
	return new;
}

void thread_free(thread_t *dead UNUSED)
{
	//Some of this can possibly handled in thread_exit()
	//close files
	//lower ref counts on root and cur directory inodes
	//decrease ref counts on address spaces
	//reap/reparent children (forgot how this is supposed to work in unix-likes)
	//other cleanup
	//dead->magic = 0xDEADBEEF;
	//kfree(dead->sig_info);
	//kfree(dead->file_info->files);
	//kfree(dead->file_info->file_flags);
	//kfree(dead->file_info);
	//palloc_free(dead);
	//
}


//Write a new thread_new that sets up a thread, similar to thread_clone,
//but without the signals, mm, and file copy overs
thread_t *thread_new()
{
	thread_t *new = thread_alloc();
	thread_t *cur = thread_current();
	new->ppid = cur->pid;
	new->pgid = cur->pgid;
	new->pid = pid_allocate();
	new->status = THREAD_NEW;
	new->sp = (void*)new;
	new->sp += STACK_SIZE;

	new->sig_info = kcalloc(sizeof(struct thread_signals), 1);
	//FIXME: It might make sense to do the file allocation here
	thread_add_child(cur, new);
	return new;
}

//Write a new thread_clone, that uses thread_new + bullshit
thread_t * thread_clone2(thread_t *parent)
{
	thread_t *new = thread_new();
	new->mm = mm_clone(parent->mm);
	new->file_info = thread_files_alloc(parent->file_info);

	kmemcpy(new->sig_info->signals, parent->sig_info->signals,
			sizeof(struct k_sigaction) * NUM_SIGNALS);
	return new;
}

void user_thread_exited(void) __attribute__((section(".user")));
void user_thread_exited(void)
{
	//Should probably call SYSCALL_1N(SYS_EXIT, current_value_of_eax);
	PANIC("User thread exited!");
}

pid_t thread_create2(uintptr_t eip, uintptr_t _esp, void *aux)
{
	enum intr_status old_level = interrupt_disable();
	thread_t *new;
	uintptr_t esp;

	printf("Creating thread with eip: %x _esp: %x\n", eip, _esp);

	if(_esp) {
		thread_t *cur = thread_current();
		new = thread_clone2(cur);

		esp = _esp;
	} else {
		new = thread_new();
		new->mm = mm_alloc();
		new->file_info = thread_files_alloc(NULL);
		new->file_info->cur = root->inode;
		new->file_info->root = root->inode;
		//
		uint32_t *usersp = palloc();
		memregion_map_data(new->mm, PHYS_BASE - PAGE_SIZE, PAGE_SIZE, PROT_GROWSDOWN,
			MAP_GROWSDOWN | MAP_FIXED, usersp);

		usersp += 1024;
		*--usersp = (uintptr_t)user_thread_exited;
		*--usersp = (uintptr_t)aux;
		esp = PHYS_BASE-8;
		printf("ESP %X USERSP %X\n", esp, usersp);
	}
	//XXX: Doesn't handle fork returning new pid
	thread_build_stackframe((void *)new, eip, esp, 0);

	uint8_t *new_sp = (void*)((uintptr_t)new + STACK_SIZE);
	// XXX: This is platform specific, and the 5 uint32_t's are so that the stack
	//		is ready for switch_thread()
	new->sp = (uint8_t *)(new_sp - (sizeof(registers_t) + sizeof(uint32_t)*5));

	thread_set_ready(new);

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

void thread_yield()
{
	// XXX: Why though?
	PANIC("Don't call thread_yield()!");
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

static struct thread_files *thread_files_alloc(struct thread_files *old)
{
	struct thread_files * new = kcalloc(sizeof(struct thread_files), 1);
	//thread_t *cur = thread_current();
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
		//new->root = cur->file_info->root;
		//new->cur = cur->file_info->cur;
	//	new->root = new->cur = root->inode;
		new->files_count = 32;
		new->files = kcalloc(sizeof(struct file *), 32);
		new->files_flags = kcalloc(sizeof(int), 32);
	}
	return new;
}
