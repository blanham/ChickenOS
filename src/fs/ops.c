/*	ChickenOS - fs/ops.c - virtual file system ops
 *	Patterned after the Linux 2.4 vfs
 *
 */
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>
#include <chicken/common.h>
#include <chicken/fs/vfs.h>
#include <chicken/thread.h>

int fd_new()
{
	static int fd = 3;

	return fd++;
}

int sys_open(const char *path, int oflag, mode_t mode)
{
	struct file * file = NULL;
	int ret = vfs_open(&file, path, oflag, mode);

	if(file == NULL)
		return ret;

	serial_printf("opening file: %s flag %x %p %x\n", path, oflag, file, mode);
	ret = put_file(file, 0);

	if(oflag & O_APPEND)
		file->offset = file->inode->info.st_size;

	return ret;
}

ssize_t sys_read(int fd, void *buf, size_t nbyte)
{
	struct file *fp = vfs_file_get(fd);
	if(fp == NULL)
		return -EBADFD;

	struct inode *inode = fp->inode;

	if (S_ISDIR(inode->info.st_mode)) // XXX: Do I care?
		return -EISDIR;

	return vfs_read(fp, buf, nbyte);
}

ssize_t sys_write(int fd, void *buf, size_t nbyte)
{
	struct file *fp = vfs_file_get(fd);
	if (fp == NULL)
		return -EBADFD;

	return vfs_write(fp, buf, nbyte);
}

ssize_t sys_readv(int fd, const struct iovec *iov, int iovcnt)
{
	ssize_t count = 0, ret = 0;

	if (iovcnt == 0 || iovcnt > UIO_MAXIOV)
		return -EINVAL;

	if (verify_pointer(iov, sizeof(*iov)*iovcnt, VP_WRITE))
		return -EFAULT;

	for (int i = 0; i < iovcnt; i++) {
		ret = sys_read(fd, iov[i].iov_base, iov[i].iov_len);
		if (ret < 0)
			return ret;

		count += ret;
	}
	//If we overflowed:
	if (count < 0)
		return -EINVAL;

	return count;
}

ssize_t sys_writev(int fd, const struct iovec *iov, int iovcnt)
{
	ssize_t count = 0, ret = 0;


	if (iovcnt == 0 || iovcnt > UIO_MAXIOV)
		return -EINVAL;

	if (verify_pointer(iov, sizeof(*iov)*iovcnt, 1))
		return -EFAULT;

	for (int i = 0; i < iovcnt; i++) {
		ret = sys_write(fd, iov[i].iov_base, iov[i].iov_len);
		if (ret < 0)
			return ret;

		count += ret;
	}
	//If we overflowed:
	if (count < 0)
		return -EINVAL;

	return count;
}

/*int creat(const char *path, mode_t mode)*/
/*int creat(const char *path UNUSED, uint32_t mode UNUSED)
  {
  return -1;
  }*/

int sys_access(const char *path, mode_t mode)
{
	thread_t *cur = thread_current();
	//dentry_t *cwd = cur->file_info->cur;

	dentry_t *lookup = NULL;
	serial_printf("Lookinf for %s\n", path);
	int ret = vfs_pathsearch(path, &lookup, NULL);
	if (lookup == NULL) {
		serial_printf("Not found\n");
		goto finish;
	}

	//get mode
	mode_t imode = lookup->inode->info.st_mode & 0777;
	mode &= 0007;

	//if uid:
	if (cur->uid == lookup->inode->info.st_uid) {
		imode >>= 6;
	} else if (cur->gid == lookup->inode->info.st_gid){
		imode >>= 3;
	}
	//mask bottom three bits
	//S_IRWXU
	//
	//check them
	// FIXME: !!!!
	if (mode != imode) {
		//ret = -EACCES;
		goto finish;
	}

	ret = 0;

finish:
	return ret;
}

// TODO: make sys_access a call to this instead
int sys_faccessat(int dirfd, const char *path, mode_t mode, int flags)
{
	if (dirfd == AT_FDCWD) {
		return sys_access(path, mode);
	}

	if (flags & AT_EACCESS) {
		// FIXME: user EUID/EGID for access checks instead of UID/GID
	}

	bool follow_link = true;
	if (flags & AT_SYMLINK_NOFOLLOW)
		follow_link = false;

	struct file *fp = vfs_file_get(dirfd);
	if (fp == NULL)
		return -EBADFD;

	if (!S_ISDIR(fp->dentry->inode->info.st_mode))
		return -ENOTDIR;

	dentry_t *dir = fp->dentry; 
	dentry_t *lookup;
	int ret = dcache_pathsearch(path, &lookup, dir, NULL, 0, follow_link);
	if (lookup == NULL) {
		serial_printf("Not found\n");
		goto finish;
	}

	//get mode
	mode_t imode = lookup->inode->info.st_mode & 0777;
	mode &= 0007;

	thread_t *cur = thread_current();
	//if uid:
	if (cur->uid == lookup->inode->info.st_uid) {
		imode >>= 6;
	} else if (cur->gid == lookup->inode->info.st_gid){
		imode >>= 3;
	}
	//mask bottom three bits
	//S_IRWXU
	//
	//check them
	// FIXME: !!!!
	if (mode != imode) {
		//ret = -EACCES;
		goto finish;
	}

	ret = 0;

finish:
	return ret;
	return 0;
}

off_t sys_llseek(int fd, unsigned long off_h, unsigned long off_l, off_t *result, int whence)
{
	struct file *fp = vfs_file_get(fd);
	if(fp == NULL)
		return -EBADFD;

	off_t offset = off_l + ((off_t)off_h << 32);
	off_t ret = vfs_seek(fp, offset, whence);
	if ((signed)ret < 0)
		return ret;

	*result = ret;

	return 0;
}

off_t sys_lseek(int fd, off_t offset, int whence)
{
	struct file *fp = vfs_file_get(fd);
	if(fp == NULL)
		return -EBADFD;
	return vfs_seek(fp, offset, whence);
}

int sys_lstat64(const char *filename, struct stat64 *buf)
{
	dentry_t *lookup;

	int ret = vfs_pathsearch_nofollow(filename,  &lookup, NULL);
	if(lookup == NULL)
		return ret;

	// TODO: it might make more sense to have this as a separate inode function
	memcpy(buf, &lookup->inode->info, sizeof(*buf));

	return 0;
}

int sys_readlink(const char *filename, char *buf, size_t size)
{
	dentry_t *lookup;

	int ret = vfs_pathsearch_nofollow(filename,  &lookup, NULL);
	if(lookup == NULL)
		return ret;

	if (lookup->inode == NULL) {
		return -ENOENT;
	}

	return lookup->inode->fs->ops->readlink(lookup->inode, buf, size);
}

int sys_mkdir(const char *path UNUSED, mode_t mode UNUSED)
{
	return -ENOSYS;;
}

//FIXME: Doesn't handle EACCES
int sys_getcwd(char *buf, size_t size)
{
	dentry_t *cur = thread_current()->fs_info->cur;
	if (cur->inode == NULL)
		return -ENOENT;

	size_t path_len = strlen(cur->path) + 1;
	if (path_len > size)
		return -ERANGE;

	if (verify_pointer(buf, size, VP_WRITE))
		return -EFAULT;

	if (size == 0)
		return -EINVAL;

	strncpy(buf, cur->path, size);

	*(buf + path_len) = '\0';

// For Linux compatibility, return length not buf
	return path_len;
}

int _sys_dup(int oldfd, int newfd, int flags)
{
	struct file *oldfp = vfs_file_get(oldfd);
	int ret = -EBADFD;

	if(oldfp == NULL)
		return -EBADFD;

	if(newfd != -1)
	{
		ret = sys_close(newfd);
		//XXX: Not sure if this is correct
		if(ret != 0 && ret != -EBADFD)
			return ret;
	}
	ret = put_file2(oldfp, newfd, flags);

	return ret;
}

int sys_dup(int oldfd)
{
	return _sys_dup(oldfd, -1, 0);
}

int sys_dup2(int oldfd, int newfd)
{
	//probably needs a check that newfd isn't -1
	return _sys_dup(oldfd, newfd, 0);
}

int sys_dup3(int oldfd, int newfd, int flags)
{
	return _sys_dup(oldfd, newfd, flags);
}

int sys_ioctl(int fd, int request, char * argp)
{
	struct file *fp = vfs_file_get(fd);
	if(fp == NULL)
		return -EBADFD;

	return vfs_ioctl(fp, request, argp);
}