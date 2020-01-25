/*	ChickenOS - thread.c
 *  Threading system
 *
 */
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <chicken/common.h>
#include <chicken/interrupt.h>
#include <chicken/thread.h>
#include <chicken/mm/liballoc.h>
#include <chicken/mm/paging.h>
#include <chicken/mm/vm.h>
//#include <util/utlist.h>

// This is used to find the sp member in switch()
const int thread_stack_offset = offsetof(thread_t, sp);

//I think this should be thread_alloc, should put it on some not ready to run list?
thread_t *thread_alloc()
{
	thread_t *new = pallocn(STACK_PAGES);
	memset(new, 0, STACK_SIZE);
	new->magic = THREAD_MAGIC;
	return new;
}

//Write a new thread_new that sets up a thread, similar to thread_clone,
//but without the signals, mm, and file copy overs
thread_t *thread_new(bool copy_old)
{
	thread_t *new = thread_alloc();
	thread_t *cur = thread_current();
	if (copy_old) {
		memcpy(new, cur, STACK_SIZE);
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

	// FIXME: remove this
	new->sig_info = kcalloc(sizeof(thread_signals_t), 1);

	//FIXME: It might make sense to do the file allocation here
	thread_add_child(cur, new);
	return new;
}

void thread_free(thread_t *dead UNUSED)
{
	//Some of this can possibly handled in thread_exit()
	//close files
	//lower ref counts on r_inodeoot and cur directory inodes
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
	kernel_thread->files = thread_files_alloc(NULL);
	kernel_thread->fs_info = kcalloc(sizeof(thread_fs_t), 1);
	kernel_thread->sig_info = kcalloc(sizeof(thread_signals_t), 1);

	kernel_thread->name = strdup("idle");
	kernel_thread->tgid = 1;

	scheduler_init(kernel_thread);
}