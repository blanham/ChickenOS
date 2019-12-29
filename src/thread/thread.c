/*	ChickenOS - thread.c
 *  Threading system
 *
 */
#include <common.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <chicken/thread.h>
#include <kernel/interrupt.h>
#include <kernel/memory.h>
#include <mm/liballoc.h>
#include <mm/paging.h>
#include <mm/vm.h>
#include <util/utlist.h>

// XXX: This should be removed
extern struct file *root;

// This is used to find the sp member in switch()
const int thread_stack_offset = offsetof(thread_t, sp);

//I think this should be thread_alloc, should put it on some not ready to run list?
thread_t *thread_alloc()
{
	thread_t *new = pallocn(STACK_PAGES);
	kmemset(new, 0, STACK_SIZE);
	new->magic = THREAD_MAGIC;
	return new;
}

//Write a new thread_new that sets up a thread, similar to thread_clone,
//but without the signals, mm, and file copy overs
thread_t *thread_new(bool lol)
{
	thread_t *new = thread_alloc();
	thread_t *cur = thread_current();
	if (lol) {
		kmemcpy(new, cur, STACK_SIZE);
		cur->children = NULL;
		cur->child_next = NULL;
		cur->child_prev = NULL;
	}
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

thread_t * thread_clone2(thread_t *parent)
{
	thread_t *new = thread_new(true);
	new->mm = mm_clone(parent->mm);
	new->file_info = thread_files_alloc(parent->file_info);

	if (parent->tls != NULL) {
		//new->tls = kcalloc(17,1);
		//kmemcpy(new->tls, parent->tls, 17);
	}
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
	uintptr_t ebp = 0;

	if(_esp) {
		thread_t *cur = thread_current();
		new = thread_clone2(cur);

		esp = _esp;
		ebp = (uintptr_t)aux;
		//thread_copy_stackframe(cur, new, 00);
	thread_build_stackframe((void *)new, eip, esp, ebp);
	} else {
		new = thread_new(false);
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
	thread_build_stackframe((void *)new, eip, esp, 0);
	}
	//XXX: Doesn't handle fork returning new pid

	uint8_t *new_sp = (void*)((uintptr_t)new + STACK_SIZE);
	// XXX: This is platform specific, and the 5 uint32_t's are so that the stack
	//		is ready for switch_thread()
	new->sp = (uint8_t *)(new_sp - (sizeof(registers_t) + sizeof(uint32_t)*5));
	serial_printf("CLONE\n");
	dump_regs((void *)new->sp + (5*4));

	thread_set_ready(new);

	interrupt_set(old_level);

	return new->pid;
}
void thread_yield()
{
	// XXX: Why though?
	PANIC("Don't call thread_yield()!");
}

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