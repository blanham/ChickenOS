#ifndef C_OS_FS_SYS_H
#define C_OS_FS_SYS_H
#include <poll.h>
#include <sys/uio.h> // iovec

/* user_ops.c - userspace file ops */
int sys_open(const char *path, int oflag, mode_t mode);
int sys_close(int fd);
ssize_t sys_read(int fildes, void *buf, size_t nbyte);
ssize_t sys_write(int filedes, void *buf, size_t nbyte);
ssize_t sys_readv(int fd, const struct iovec *iov, int iovcnt);
ssize_t sys_writev(int fd, const struct iovec *iov, int iovcnt);
int sys_getcwd(char *buf, size_t size);
int sys_chdir(const char *path);
int sys_mkdir(const char *path, mode_t mode);

int sys_creat(const char *path, mode_t mode);
int sys_fcntl64(int fd, int cmd, unsigned long arg);
int sys_access(const char *path, mode_t mode);
int sys_faccessat(int dirfd, const char *path, mode_t mode, int flags);
int sys_readlink(const char *filename, char *buf, size_t size);
int sys_stat(const char *filename, struct stat *statbuf);
int sys_lstat64(const char *filename, struct stat64 *buf);
int sys_fstat64(int fd, struct stat64 *buf);
off_t sys_lseek(int fildes, off_t offset, int whence);
off_t sys_llseek(int fildes, unsigned long off_h, unsigned long off_l, off_t *result, int whence);
int sys_ioctl(int fildes, int request, char *args);
int sys_getdents(int fildes, struct dirent *dirp, unsigned int count);
int sys_stat64(const char *path, struct stat64 *buf);


int sys_dup(int oldfd);
int sys_dup2(int oldfd, int newfd);
int sys_dup3(int oldfd, int newfd, int flags);
int sys_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
int sys_poll(struct pollfd *fds, nfds_t nfds, int timeout);

#endif