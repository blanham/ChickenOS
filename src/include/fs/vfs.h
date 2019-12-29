#ifndef C_OS_VFS_H
#define C_OS_VFS_H
#include <stdatomic.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <poll.h>
#include <fs/vfs_ops.h>

typedef int64_t ino64_t;
typedef int64_t off64_t;
typedef uint64_t blkcnt64_t;

//default block/buffer size
#define SECTOR_SIZE 512

typedef struct vfs_fs vfs_fs_t;
struct inode;


typedef struct vfs_sb {
	void *sb;
	struct inode *root;
} vfs_sb_t;

struct vfs_fs {
	vfs_sb_t *superblock;
	char name[10];
	void *aux;
	uint16_t dev;
	vfs_ops_t *ops;
};

struct inode {
	// XXX: Is this the best way to implement this? Should we have seperate fields?
	struct stat64 info;
	//if part of mount point,keep in cache
	uint32_t flags;
	void *storage;
	//may need parent
	vfs_fs_t *fs;
};

#define I_MOUNT 0x1

struct file {
	char name[256];
	struct inode *inode;
	uint32_t offset;
	vfs_fs_t *fs;
};


/* vfs.c */
vfs_fs_t *vfs_alloc();
struct inode * vfs_pathsearch(struct file *dir, char *_path);
struct file *vfs_file_new(struct inode *inode, char *name);
int vfs_register_fs(vfs_fs_t *fs);
void vfs_init();
void vfs_mount_root(uint16_t dev, char *type);

/* vfs_ops.c - internal/kernel ops */
struct file *vfs_open(char *path, int oflags, mode_t mode);
int vfs_close(struct file *file);
size_t vfs_read(struct file *file, void *buf, size_t nbyte);
off_t vfs_write(struct file *file, void *buf, size_t nbyte);
off_t vfs_seek(struct file *file, off_t offset, int whence);
int vfs_ioctl(struct file *file, int request, char *args);
int vfs_chdir(const char *path);
int vfs_stat(const char *path, struct stat *buf);
int vfs_stat64(const char *path, struct stat64 *buf);
int vfs_fstat64(struct inode *inode, struct stat64 *buf);

/* user_ops.c - userspace file ops */
int sys_open(const char *path, int oflag, mode_t mode);
int sys_close(int fd);
ssize_t sys_read(int fildes, void *buf, size_t nbyte);
ssize_t sys_write(int filedes, void *buf, size_t nbyte);
ssize_t sys_readv(int fd, const struct iovec *iov, int iovcnt);
ssize_t sys_writev(int fd, const struct iovec *iov, int iovcnt);
char* sys_getcwd(char *buf, size_t size);
int sys_chdir(const char *path);

int sys_creat(const char *path, mode_t mode);
int sys_fcntl64(int fd, int cmd, unsigned long arg);
int sys_stat(const char *filename, struct stat *statbuf);
int sys_fstat64(int fd, struct stat64 *buf);
off_t sys_lseek(int fildes, off_t offset, int whence);
int sys_ioctl(int fildes, int request, char *args);
int sys_getdents(int fildes, struct dirent *dirp, unsigned int count);
int sys_stat64(const char *path, struct stat64 *buf);

int sys_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
int sys_poll(struct pollfd *fds, nfds_t nfds, int timeout);
//device.c
#define INITRD_DEV 0x400
#define ATA0_0_DEV 0x301
#define MAJOR(x) ((x & 0xFF00) >> 8)
#define MINOR(x) (x & 0xFF)

typedef size_t (*char_read_fn)(dev_t dev, void *buf, size_t count, off_t offset);
typedef size_t (*char_write_fn)(dev_t dev, void *buf, size_t count, off_t offset);
typedef int (*char_ioctl_fn)(dev_t dev, int request, char *arg);

struct char_device_ops {
	char_read_fn read;
	char_write_fn write;
	char_ioctl_fn ioctl;
};

typedef size_t (*block_read_fn) (uint16_t dev, void *buf, int block);
typedef size_t (*block_write_fn)(uint16_t dev, void *buf, int block);

void  device_register(uint16_t type, dev_t dev, void *read, void *write, void *ioctl);
typedef int(*block_access_fn)(void *aux, void *buf, int block);

int char_device_ioctl(dev_t dev, int request, char *args);
size_t char_device_read(dev_t dev, void *buf, off_t offset, size_t nbyte);
size_t char_device_write(dev_t dev, void *buf, off_t offset, size_t nbyte);
size_t block_device_read(uint16_t dev, void *buf, uint32_t block);
size_t block_device_readn(uint16_t dev, void *buf, uint32_t block, off_t offset, size_t nbyte);

int read_block_at(uint16_t dev, void * _buf, int block,int block_size, off_t offset, size_t nbytes);
int read_block(uint16_t dev, void * _buf, int block, int block_size);

int write_block_at(uint16_t dev, void * _buf, int block,int block_size, off_t offset, size_t nbytes);
int write_block(uint16_t dev, void * _buf, int block, int block_size);

#endif
