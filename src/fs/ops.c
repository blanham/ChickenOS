/*	ChickenOS - fs/ops.c - virtual file system ops
 *	Patterned after the Linux 2.4 vfs
 *
 */
#include <kernel/common.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <chicken/thread.h>
#include <kernel/memory.h>
#include <mm/vm.h>
#include <fs/vfs.h>
#include <fs/ext2/ext2.h>
#include <mm/liballoc.h>
#include <thread/syscall.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/uio.h>
#include <sys/select.h>
#include <poll.h>
//file stuff should be seperated out
struct file *open_files[100];
uint8_t file_count = 0;

int fd_new()
{
	static int fd = 3;

	return fd++;
}

int sys_open(const char *_path, int oflag, mode_t mode)
{
	if (_path == NULL)
		return -EFAULT;
	thread_t *cur = thread_current();
	char *path = strdup(_path);
	struct file * fp = vfs_open(path, oflag, mode);

	int td = 0;
	serial_printf("opening file: %s flag %x %p %x\n", _path, oflag, fp, O_CREAT);

	if(fp == NULL)
	{
		goto fail;
	}

	for(int i = 0; i < cur->file_info->files_count; i++)
	{
		if(cur->file_info->files[i] == NULL)
		{
			td = i;
			cur->file_info->files[i] = fp;
			break;
		}
	}
	kfree(path);
	///	printf("FD %i\n", td);
	return td;
fail:
	kfree(path);
	return -1;
}
int sys_close(int fd)
{
	struct file *fp = thread_current()->file_info->files[fd];
	thread_current()->file_info->files[fd] = NULL;
	if(fp == NULL)
		return -1;
	return vfs_close(fp);
}


ssize_t sys_read(int fildes, void *buf, size_t nbyte)
{
	struct file *fp = thread_current()->file_info->files[fildes];
	if(fp == NULL || fildes < 0)
		return -1;
	return vfs_read(fp, buf, nbyte);
}
ssize_t sys_write(int fildes, void *buf, size_t nbyte)
{
	struct file *fp = thread_current()->file_info->files[fildes];

	if(fp == NULL)
		return -1;
	ssize_t ret;
	ret = vfs_write(fp, buf, nbyte);
	return ret;
}

ssize_t sys_readv(int fd, const struct iovec *iov, int iovcnt)
{
	ssize_t count = 0, ret = 0;

	if(iovcnt == 0 || iovcnt > UIO_MAXIOV)
		return -EINVAL;

	if(verify_pointer(iov, sizeof(*iov)*iovcnt, 0))
		return -EFAULT;

	for(int i = 0; i < iovcnt; i++)
	{
		ret = sys_read(fd, iov[i].iov_base, iov[i].iov_len);
		if(ret < 0)
			return ret;

		count += ret;
	}
	//If we overflowed:
	if(count < 0)
		return -EINVAL;

	return count;
}

int poll_toggle_hack = 0;
int sys_poll(struct pollfd *fds UNUSED, nfds_t nfds UNUSED, int timeout UNUSED)
{
	//for (unsigned i = 0; i < nfds; i++) {
		//serial_printf("Test: %i %x %x\n", fds[i].fd, fds[i].events, fds[i].revents);
	//}
	//serial_printf("Polling: %i %i\n", nfds, timeout);
	if (poll_toggle_hack)
		poll_toggle_hack = 0;
	else
		poll_toggle_hack = 1;
	return poll_toggle_hack;
}

int fart2 = 0;
int sys_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
	(void)nfds;
	(void)readfds;
	(void)writefds;
	(void)exceptfds;
	//(void)timeout;
	//serial_printf("select() nfds: %i", nfds);
	if (readfds) {
		//serial_printf(" READFDS: %x", readfds->fds_bits[0]);
	}
	if (writefds) {
		//serial_printf(" WRITEFDS: %x", writefds->fds_bits[0]);
	}
	if (exceptfds) {
		//serial_printf(" EXCEPTFDS: %x\n", exceptfds->fds_bits[0]);
	}
	if (timeout) {
		timeout->tv_sec = 0;
		timeout->tv_usec = 0;
	}
	//serial_printf("\n");
	//if (fart2 < 2)
		//fart2++;
	//else
		//fart2 = 0;
	//return nfds;
	if (fart2) {
		fart2 = 0;
		return 1;
	}
	return 0;//fart2 & 0x3 ? 1 : 0;
}

ssize_t sys_writev(int fd, const struct iovec *iov, int iovcnt)
{
	ssize_t count = 0, ret = 0;


	if(iovcnt == 0 || iovcnt > UIO_MAXIOV)
		return -EINVAL;

	if(verify_pointer(iov, sizeof(*iov)*iovcnt, 1))
		return -EFAULT;

	for(int i = 0; i < iovcnt; i++)
	{
		ret = sys_write(fd, iov[i].iov_base, iov[i].iov_len);
		if(ret < 0)
			return ret;

		count += ret;
	}
	//If we overflowed:
	if(count < 0)
		return -EINVAL;

	return count;
}

/*int creat(const char *path, mode_t mode)*/
/*int creat(const char *path UNUSED, uint32_t mode UNUSED)
  {
  return -1;
  }*/
off_t sys_lseek(int fildes, off_t offset, int whence)
{
	struct file *fp = thread_current()->file_info->files[fildes];
	if(fp == NULL)
		return -1;
	return vfs_seek(fp, offset, whence);
}

int sys_stat(const char *filename, struct stat *statbuf)
{
	return vfs_stat(filename, statbuf);
}

int sys_stat64(const char *filename, struct stat64 *statbuf)
{
	return vfs_stat64(filename, statbuf);
}

int sys_fstat64(int fd, struct stat64 *statbuf)
{
	struct file *fp = thread_current()->file_info->files[fd];
	return vfs_fstat64(fp->inode, statbuf);
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
int sys_ioctl(int fildes, int request, char * argp)
{
	struct file *fp;
	if(fildes == -1)
		return -1;
	fp = thread_current()->file_info->files[fildes];
	if(fp == NULL)
		return -1;
	//printf("fildes %i\n", fildes);
	return vfs_ioctl(fp, request, argp);
}

