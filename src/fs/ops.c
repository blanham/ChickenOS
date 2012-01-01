/*	ChickenOS - fs/ops.c - virtual file system ops
 *	Patterned after the Linux 2.4 vfs
 *	
 */
#include <kernel/common.h>
#include <kernel/types.h>
#include <kernel/memory.h>
#include <kernel/thread.h>
#include <kernel/vm.h>
#include <kernel/fs/vfs.h>
#include <kernel/fs/ext2/ext2.h>
#include <mm/liballoc.h>
#include <stdio.h>
#include <string.h>
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
	if(fp == NULL)
		return -1;
	int fd = fd_new();
	open_files[fd] = fp;
	kfree(path);
	return fd;
}
//off_t vfs_seek(struct file *i
/*
ssize_t sys_read(int fildes, void *buf, size_t nbyte)
{
	struct file *fp = open_files[fildes];
	if(fp == NULL)
		return -1;
	return vfs_read(fp, buf, nbyte);
}
ssize_t sys_write(int fildes, void *buf, size_t nbyte)
{
	struct file *fp = open_files[fildes];
	if(fp == NULL)
		return -1;
	return vfs_write(fp, buf, nbyte);
}*/
/*int creat(const char *path, mode_t mode)*/
/*int creat(const char *path UNUSED, uint32_t mode UNUSED)
{
	return -1;
}*/
off_t lseek(int fildes UNUSED, off_t offset UNUSED, int whence UNUSED)
{
	return -1;
}

int sys_chdir(const char *path)
{
	return vfs_chdir(path);
}
