/*	ChickenOS - fs/vfs.c - virtual file system systemcalls
 *
 */
#include <common.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <chicken/thread.h>
#include <fs/vfs.h>
#include <fs/vfs_ops.h>
/*
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <chicken/thread.h>
#include <kernel/common.h>
#include <kernel/memory.h>
#include <mm/vm.h>
#include <fs/vfs.h>
#include <fs/ext2/ext2.h>
#include <mm/liballoc.h>*/

int sys_fcntl64(int fd UNUSED, int cmd, unsigned long arg UNUSED)
{
	//struct file *fp = get_file(fildes);

	switch(cmd)
	{
		case F_DUPFD:
		//FIXME? Not sure if this needs to eventually be implemented or not
		//case F_DUPFD_CLOEXEC:
		case F_GETFD:
		case F_SETFD:
		case F_GETFL:
		case F_SETFL:
		case F_GETLK:
		case F_SETLK:
		case F_SETLKW:
			serial_printf("fcntl: unimplemented cmd %d\n", cmd);
			return -ENOSYS;
		default:
			//FIXME: invalid commands should actually return 0
			serial_printf("fcntl: unknown cmd %d\n", cmd);
			return -ENOSYS;

	}
	return -ENOSYS;
}

int sys_getdents(int fildes, struct dirent *dirp, unsigned int count)
{
	// FIXME: struct file *fp = get_file(fildes);
    struct file *fp = thread_current()->file_info->files[fildes];
	if(fp == NULL)
		return -EBADFD;

	//if(verify_pointer(dirp, sizeof *dirp))
	//	return -EFAULT;

	if(!S_ISDIR(fp->inode->info.st_mode))
	{
		serial_printf("MODE: %o\n", fp->inode->info.st_mode);
		return -ENOTDIR;
	}
	//TODO: Add this case
	//if(fp->inode was deleted after we opened it, but we still have a reference)
	//if(fp->inode->deleted)?
	//	return -ENOENT;
	//XXX: This needs to go through the dentry cache so mountpoints will work
	//XXX: Also, this is nasty, four levels of indirection?
	//return fp->inode->fs->ops->getdents(fp->inode, dirp, count, &fp->offset);
    int64_t offset = fp->offset;
	int ret = fp->inode->fs->ops->getdents(fp->inode, dirp, count, &offset);
    fp->offset = (uint32_t)offset;
    return ret;
}

struct file *vfs_open(char *path, int oflags, mode_t mode)
{
	(void)mode;
	struct inode *cur = thread_current()->file_info->cur;
	struct file *tmp = kcalloc(sizeof(*tmp), 1);
	tmp->inode = cur;
	struct inode *lookup = vfs_pathsearch(tmp, path);

	if(lookup == NULL)
	{
		if((oflags & O_CREAT) != 0)
		{
	//		printf("need to create file\n");
		}
		return NULL;
	}
	kfree(tmp);
	struct file *new = vfs_file_new(lookup, path);
	return new;
}

int vfs_close(struct file *file)
{
	if(file == NULL)
		return -1;

	//vfs_file_free(file);

	return 0;
}

size_t vfs_read(struct file *file, void *buf, size_t nbyte)
{
	int ret = 0;
	if(file == NULL || buf == NULL)
		return -1;
	if((file->inode->info.st_mode & S_IFCHR) != 0){
		ret = char_device_read(file->inode->info.st_rdev,
			buf, file->offset, nbyte);
	}else if((file->inode->info.st_mode & S_IFBLK) != 0){
		ret = block_device_readn(file->inode->info.st_rdev,
			buf, 0, file->offset, nbyte);
	}else if((file->inode->info.st_mode & S_IFREG) != 0){
		//printf("READ OFF %x\n", file->offset);
		if(file->fs == NULL || file->fs->ops == NULL || file->fs->ops->read == NULL)
			return -1;
		ret = file->fs->ops->read(file->inode, buf,
			nbyte, file->offset);
	}
	file->offset += ret;
	return ret;

}

off_t vfs_write(struct file *file, void *buf, size_t nbyte)
{
	int ret = 0;
	if(nbyte == 0)
		return 0;
	if(file == NULL || buf == NULL)
		return -1;
	if((file->inode->info.st_mode & S_IFCHR) != 0){
		ret = char_device_write(file->inode->info.st_rdev,
			buf, file->offset, nbyte);
	}else if((file->inode->info.st_mode & S_IFBLK) != 0){
//FIXME: This returns -1 so we don't fuxxor our disk image accidentally
	//	ret = block_device_readn(file->inode->rdev,
	//		buf, 0, file->offset, nbyte);
		printf("Writing it currently disabled\n");
		return -1;
	}else if((file->inode->info.st_mode & S_IFREG) != 0){
		if(file->fs == NULL || file->fs->ops->write == NULL)
			return -1;
		ret = file->fs->ops->write(file->inode, buf,
			nbyte, file->offset);
	}
	file->offset += ret;
	return ret;
}

int  vfs_ioctl(struct file *file, int request, char * args)
{
	int ret = 0;
	if(file == NULL)
		return -1;
	if((file->inode->info.st_mode & S_IFCHR) != 0){
		ret = char_device_ioctl(file->inode->info.st_rdev,
			request, args);
	}else if((file->inode->info.st_mode & S_IFBLK) != 0){
	//	ret = block_device_readn(file->inode->rdev,
	//		buf, 0, file->offset, nbyte);
		return -1;
	}else if((file->inode->info.st_mode & S_IFREG) != 0){
	//	if(file->fs == NULL || file->fs->ops->write == NULL)
	//		return -1;
	//	ret = file->fs->ops->write(file->inode, buf,
	//		nbyte, file->offset);
		return -1;
	}
	return ret;
}



//FIXME: Error catching
//		check if fd is open and return EBADF
//		check if file offset is negative and return EINVAL
//		check if offset overflows off_t and return EOVERFLOW
//		check if pipe and return ESPIPE
off_t vfs_seek(struct file *file, off_t offset, int whence)
{
	switch(whence)
	{
		case SEEK_SET:
			file->offset = offset;
			break;
		case SEEK_CUR:
			file->offset += offset;
			break;
		case SEEK_END:
			//file->offset = file->end + offset;
			//break;
		default:
			//EINVAL?
			return 0;

	}

	return file->offset;
}
//FIXME: I'm not sure if the logic here checks out
//FIXME: Add various errors, most importantly
//		EBADF, EACCES, ENOTDIR and EFAULT
int vfs_chdir(const char *_path)
{
	struct file *file;
	int ret = -1;
	char *path = strdup(_path);
	file  = vfs_open(path, 0, 0);
	if(file != NULL)
	{
		//vfs_close(thread_current()->cur_dir);
		thread_current()->file_info->cur = file->inode;
		ret = 0;
	}
	kfree(path);
	return ret;
}

int vfs_stat(const char *path, struct stat *buf)
{
	(void)path;
	(void)buf;
	PANIC("NOT IMPLEMENTED");

	return 0;
}

int vfs_stat64(const char *path, struct stat64 *buf)
{
	struct inode *cur = thread_current()->file_info->cur;
	struct file *tmp = kcalloc(sizeof(*tmp), 1);
	tmp->inode = cur;
	struct inode *lookup = vfs_pathsearch(tmp, (char *)path);

	if(lookup == NULL)
		return -(ENOENT);
	kfree(tmp);
	memcpy(buf, &lookup->info, sizeof(*buf));
	serial_printf("MODE: %o\n", lookup->info.st_mode);

	return 0;
}
int vfs_fstat64(struct inode *inode, struct stat64 *buf)
{
	// FIXME: this isn't the only error I don't think
	if(inode == NULL)
		return -(ENOENT);
	memcpy(buf, &inode->info, sizeof(*buf));

	return 0;
}


/*
struct inode {
	uint32_t inode_num;
	uint16_t mode;
	uint16_t pad;
	uint32_t size;
	uint32_t atime;
	uint32_t ctime;
	uint32_t dtime;
	uint32_t mtime;
	uint32_t time;
	uint16_t gid;
	uint16_t uid;
	uint16_t links_count;
	uint16_t rdev;
	//if part of mount point,keep in cache
	uint32_t flags;
	void *storage;
	//may need parent
	vfs_fs_t *fs;
};

struct stat64
{
	dev_t st_dev;
	int __st_dev_padding;
	long __st_ino_truncated;
	mode_t st_mode;
	nlink_t st_nlink;
	uid_t st_uid;
	gid_t st_gid;
	dev_t st_rdev;
	int __st_rdev_padding;
	off64_t st_size;
	blksize_t st_blksize;
	blkcnt64_t st_blocks;
	struct timespec st_atim;
	struct timespec st_mtim;
	struct timespec st_ctim;
	ino64_t st_ino;
};:*/

