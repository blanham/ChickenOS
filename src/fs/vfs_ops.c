/*	ChickenOS - fs/vfs.c - virtual file system systemcalls
 *
 */
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <chicken/common.h>
#include <chicken/fs/device.h>
#include <chicken/fs/icache.h>
#include <chicken/fs/vfs.h>
#include <chicken/fs/vfs_ops.h>
#include <chicken/thread.h>



// TODO: No EINTR, EROFS, TTY stuff (O_NOCTTY)
// TODO: If this is a device file, we should check if the major/minor exist
//       and return EXIO if not
// TODO: for use in the kernel, this should probably return a dentry instead of a file tbh
int vfs_open(struct file **new, const char *path, int oflags, mode_t mode UNUSED)
{
	//struct inode *lookup, *parent, *new_inode;
	int file_flags = 0;
	//const char *new_file;

	serial_printf("Opening %s\n", path);

	dentry_t *lookup_d = NULL, *parent = NULL;
	int ret = vfs_pathsearch(path, &lookup_d, &parent);
	if (lookup_d == NULL)
		return ret;

	struct inode *lookup = lookup_d->inode;


	if (ret < 0 && ret != -ENOENT)
		return ret;

	if (ret == -ENOENT) {
		if(!(oflags & O_CREAT))
			return -ENOENT;
		// TODO
		PANIC("File creation not implemented");
		//ret = vfs_creation_helper(cur, path, &parent, &new_file);
		//if(ret < 0)
		//	return ret;
		//ret = parent->fs->ops->creat(parent, new_file, mode, &new_inode);
		//icache_put(parent);
		//if(ret < 0)
		//	return ret;
		//lookup = new_inode;

	}
	else if((oflags & O_EXCL) && (oflags & O_CREAT))
	{
		//icache_put(lookup);
		return -EEXIST;
	}
	//TODO: WR testing
	//if(lookup->fs->flags & read_only)
	//	{
	//		icache_put(lookup);
	//		return -EROFS;
	//	}

	if (S_ISCHR(lookup->info.st_mode)) {
		if (oflags & O_NOCTTY) {
			//set controlling terminal
		}
	}

	//printf("MODE %o\n", lookup->info.st_mode);

	// Could support FIFOs and sockets here too!
	if (S_ISCHR(lookup->info.st_mode) || S_ISBLK(lookup->info.st_mode)) {
		struct device *dev = get_device(lookup->info.st_mode & (S_IFBLK | S_IFCHR), lookup->info.st_rdev);
		//printf("This happned %s %p %p %x\n", path, dev->read, dev->ioctl, lookup->info.st_rdev);
		lookup->read = dev->read;
		lookup->write = dev->write;
		lookup->ioctl = dev->ioctl;
	} else if (S_ISREG(lookup->info.st_mode)) {
		lookup->read = (void *)lookup->fs->ops->read;
		//lookup->write = lookup->fs->ops->write;
	}

	//if (lookup->read == NULL) {
	//	printf("Path %s\n", path);
	//	PANIC("ASDFASDF\n");
	//}


	// O_NONBLOCK doesn't apply to regular files and block devices
	if (!(oflags & O_NONBLOCK)) {
		if (S_ISCHR(lookup->info.st_mode)) {
			//if(this device doesn't support nonblocking opens)
			//wait
		}

		if (S_ISFIFO(lookup->info.st_mode)) {
			//do FIFO shit
		}
	}

	// FIXME: Document what this is doing
	if ((oflags & 3) != 3)
		file_flags = oflags & 3;
	else
		file_flags |= O_RDWR;

	file_flags |= oflags & (O_DSYNC | O_NONBLOCK | O_RSYNC | O_SYNC);

	lookup_d->inode = lookup;

	*new = vfs_file_new(lookup_d);
	
	// XXX: Re-enable/rewrite
	//(*new)->flags = file_flags;

	return ret;
}


// TODO: Doesn't handle EINTR, EIO
int sys_close(int fd)
{
	struct file *fp = vfs_file_get(fd);
	if(fp == NULL)
		return -EBADFD;

	thread_current()->files->files[fd] = NULL;
	icache_put(fp->inode);

	// FIXME: this isn't safe yet
	//vfs_file_free(fp);
	return 0;
}

size_t vfs_read(struct file *file, void *buf, size_t count)
{
	if (file->inode->read == NULL) {
		printf("YUP %lli %llx\n", file->inode->info.st_ino, file->inode->info.st_dev);
		return -1;
	}
	int ret = file->inode->read(file->inode, buf, count, file->offset);
	if (ret > 0)
		file->offset += ret;
	return ret;
}

off_t vfs_write(struct file *file, void *buf, size_t count)
{
	int ret = file->inode->write(file->inode, buf, count, file->offset);
	if (ret > 0)
		file->offset += ret;
	return ret;
}

int  vfs_ioctl(struct file *file, int request, char * args)
{
	int ret = -ENOTTY; // Yes, that's the correct error
	if (file->inode == NULL)
		return -1;
	if (file->inode->ioctl)
		ret = file->inode->ioctl(file->inode, request, args);
	return ret;
}

// TODO : Error catching
//			* check if offset overflows off_t and return EOVERFLOW
off_t vfs_seek(struct file *file, off_t offset, int whence)
{
	if(S_ISFIFO(file->inode->info.st_mode))
		return -ESPIPE;

	if (offset < 0)
		return -EINVAL;

	switch (whence) {
		case SEEK_SET:
			if (offset < 0)
				return -EINVAL;
			file->offset = offset;
			break;
		case SEEK_CUR:
			if (file->offset + offset < 0)
				return -EINVAL;
			file->offset += offset;
			break;
		case SEEK_END:
			if (file->inode->info.st_size + offset < 0)
				return -EINVAL;
			file->offset = file->inode->info.st_size + offset;
			break;
		default:
			return -EINVAL;
	}

	return file->offset;
}

//FIXME: Add various errors, most importantly
//		EBADF, EACCES, ENOTDIR and EFAULT
int sys_chdir(const char *path)
{
	if (verify_pointer(path, -1, VP_READ))
		return -EFAULT;

	dentry_t *lookup = NULL;
	int ret = vfs_pathsearch(path, &lookup, NULL);
	if (!lookup)
		return ret;

	if (!S_ISDIR(lookup->inode->info.st_mode)) {
		return -ENOTDIR;
	}

	// FIXME: increase new dentry's ref count, decrease old one
	thread_current()->fs_info->cur = lookup;

	return 0;
}

int sys_stat(const char *filename UNUSED, struct stat *statbuf UNUSED)
{
	serial_printf("32-bit stat not implemented yet\n");
	return -ENOSYS;
}

int sys_stat64(const char *path, struct stat64 *statbuf)
{
	dentry_t *lookup;

	int ret = vfs_pathsearch(path, &lookup, NULL);
	if(lookup == NULL)
		return ret;

	if (lookup->inode == NULL)
		return -ENOENT;

	memcpy(statbuf, &lookup->inode->info, sizeof(*statbuf));

	return 0;
}

int sys_fstat64(int fd, struct stat64 *statbuf)
{
	struct file *fp = vfs_file_get(fd);
	
	if(fp == NULL)
		return -EBADFD;

	if(fp->inode == NULL)
		return -ENOENT;

	memcpy(statbuf, &fp->inode->info, sizeof(*statbuf));

	return 0;
}