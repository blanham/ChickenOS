#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <chicken/common.h>
#include <chicken/fs/vfs.h>
#include <chicken/fs/vfs_ops.h>
#include <chicken/thread.h>

int sys_fcntl64(int fd, int cmd, unsigned long arg)
{
	struct file *fp = vfs_file_get(fd);

	serial_printf("FCNTL %i %i %i %p\n", fd, cmd, arg, fp);
	return 0;

	if (fp == NULL)
		return -EBADFD;

	switch(cmd)
	{
		case F_SETFD:
			fp->flags = arg;
			return 0;
		case F_GETFD:
			return fp->flags;
		case F_DUPFD:
		//FIXME? Not sure if this needs to eventually be implemented or not
		//case F_DUPFD_CLOEXEC:
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