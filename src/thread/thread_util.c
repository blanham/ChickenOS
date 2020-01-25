#include <chicken/common.h>
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

// TODO: Array? Bitmap? Possibly should be in scheduler instead?
pid_t tgid_allocate()
{
	static pid_t tgid_count = 0;

	tgid_count++;

	return tgid_count;
}

void thread_add_child(thread_t *parent, thread_t *child)
{
	LL_APPEND2(parent->children, child, child_next);
}

thread_files_t *thread_files_alloc(thread_files_t *old)
{
	thread_files_t * new = kcalloc(sizeof(*new), 1);
	if (old) {
		memcpy(new, old, sizeof(*new));
		new->files = kcalloc(sizeof(struct file *), 32);
		new->files_flags = kcalloc(sizeof(int), 32);
		//FIXME: increment ref count on root and cur_dir inodes
		for (int i = 0; i < old->files_count; i++) {
			//new->files[i] = vfs_reopen(old->files[i]);
			new->files[i] = old->files[i];
			//new->files_flags[i] = 0;
		}
	} else {
		new->files_count = 32;
		new->files = kcalloc(sizeof(struct file *), 32);
		new->files_flags = kcalloc(sizeof(int), 32);
	}
	return new;
}
