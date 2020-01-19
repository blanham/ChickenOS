#include <common.h>
#include <memory.h>
#include <chicken/thread.h>

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
		thread_t *cur = thread_current();
		if (cur->file_info) {
			new->cur = cur->file_info->cur;
			new->root = cur->file_info->root;
		}

		new->files_count = 32;
		new->files = kcalloc(sizeof(struct file *), 32);
		new->files_flags = kcalloc(sizeof(int), 32);
	}
	return new;
}
