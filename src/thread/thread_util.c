#include <chicken/common.h>
#include <chicken/thread.h>

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
