#include <stdint.h>
#include <stdlib.h>
#include <chicken/fs/vfs.h>
#include <chicken/thread.h>

//FIXME: this function should increase the struct files' refcount
int put_file2(struct file *file, int givenfd, int flags)
{
	struct file **p, **orig = thread_current()->files->files;
	int files_cnt = thread_current()->files->files_count;

	if(givenfd != -1)
	{
		//FIXME: will fuck itself if givenfd goes off end of array
		//       function should be rewritten
		orig[givenfd] = file;
		thread_current()->files->files_flags[givenfd] = flags;
		return givenfd;
	}

	for(p = orig; p - orig < files_cnt; p++)
	{
		if(*p == NULL)
		{
			*p = file;
			thread_current()->files->files_flags[p - orig] = flags;
		   	return p - orig;
		}
	}

	//TODO: limit open files
	//if(files_cnt == NR_FILES)
	//	return -EMFILE;

	orig = thread_current()->files->files = krealloc(orig,
			sizeof (struct file *) * files_cnt*2);
	thread_current()->files->files_flags = krealloc(thread_current()->files->files_flags,
			sizeof (int) * files_cnt*2);
	thread_current()->files->files_count = files_cnt * 2;

	p = orig + files_cnt;
	*p = file;
	thread_current()->files->files_flags[files_cnt] = flags;
	return files_cnt;
}

int put_file(struct file *file, int flags)
{
	return put_file2(file, -1, flags);
}

/*
void dirent_print(struct dirent *dirp, int length)
{
	struct dirent *p = dirp;
	while(length > 0)
	{
		printf("Name %s\n", p->d_name);
		length -= p->d_reclen;
		p = (void *)p + p->d_reclen;
	}
}
*/