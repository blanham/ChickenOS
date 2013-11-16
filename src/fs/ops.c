/*	ChickenOS - fs/ops.c - virtual file system ops
 *	Patterned after the Linux 2.4 vfs
 *	
 */
#include <kernel/common.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <kernel/memory.h>
#include <kernel/thread.h>
#include <mm/vm.h>
#include <fs/vfs.h>
#include <fs/ext2/ext2.h>
#include <mm/liballoc.h>
#include <thread/syscall.h>
#include <sys/stat.h>
#include <errno.h>
//file stuff should be seperated out
struct file *open_files[100];
uint8_t file_count = 0;

int fd_new()
{
	static int fd = 3;
	
	return fd++;
}


int sys_open(const char *_path, int oflag UNUSED, ...)
{
	char *path = strdup(_path);
	struct file * fp = vfs_open(path);
	int td = thread_current()->fd++;
	if(fp == NULL)
		return -1;
//	int fd = fd_new();
//	open_files[fd] = fp;
	thread_current()->files[td] = fp;
	kfree(path);
	return td;
}
int sys_close(int fd)
{
	struct file *fp = thread_current()->files[fd];
	thread_current()->fd--;
	if(fp == NULL)
		return -1;
	return vfs_close(fp);
}


ssize_t sys_read(int fildes, void *buf, size_t nbyte)
{
	struct file *fp = thread_current()->files[fildes];
	if(fp == NULL)
		return -1;
	return vfs_read(fp, buf, nbyte);
}
ssize_t sys_write(int fildes, void *buf, size_t nbyte)
{
	struct file *fp = thread_current()->files[fildes];
	
	if(fp == NULL)
		return -1;
	ssize_t ret;
	ret = vfs_write(fp, buf, nbyte);

	return ret;
}
/*int creat(const char *path, mode_t mode)*/
/*int creat(const char *path UNUSED, uint32_t mode UNUSED)
{
	return -1;
}*/
off_t sys_lseek(int fildes, off_t offset, int whence)
{
	struct file *fp = thread_current()->files[fildes];
	if(fp == NULL)
		return -1;
	return vfs_seek(fp, offset, whence);
}
int sys_stat(char *filename, struct stat *statbuf)
{
	printf("sys_stat: %s %p\n", filename, statbuf);
	
	return -ENOENT;
}

int sys_chdir(const char *path)
{
	return vfs_chdir(path);
}




//FIXME: Placeholder
//For Linux compatibility, return length not buf
char *sys_getcwd(char *buf, size_t size UNUSED)
{
	strcpy(buf, "/");
	
	return (char *)strlen(buf);
}



//FIXME: Placeholder
int sys_dup(int oldfd UNUSED)
{
	return -1;//ENOSYS;
}

//FIXME: Placeholder
int sys_dup2(int oldfd UNUSED, int newfd UNUSED)
{
	return -1;//ENOSYS;
}

//FIXME: doesn't handle varargs
int sys_ioctl(int fildes, int request, va_list args)
{
	struct file *fp;
	if(fildes == -1)
		return -1;
	fp = thread_current()->files[fildes];
	if(fp == NULL)
		return -1;
	return vfs_ioctl(fp, request, args);
}

