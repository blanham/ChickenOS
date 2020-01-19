#ifndef C_OS_VFS_H
#define C_OS_VFS_H
#include <stdint.h>
#include <sys/stat.h>
#include <fs/vfs_ops.h>

typedef struct vfs_sb {
	void *sb;
	struct inode *root;
} vfs_sb_t;

typedef struct vfs_fs {
	vfs_sb_t *superblock;
	const char *name;
	void *aux;
	struct inode *dev;
	vfs_ops_t *ops;
} vfs_fs_t;

typedef struct inode {
	dev_t device;
	ino_t number;
	struct stat64 info;
	uint32_t flags;
	void *storage;
	//may need parent
	vfs_fs_t *fs;
    vfs_read_inode_t read;
    vfs_write_inode_t write;
    vfs_ioctl_inode_t ioctl;

	struct inode *parent;
} inode_t;

struct file {
	// XXX: Get rid of this
	char name[256];
	uint32_t flags; // Used for O_CLOEXEC
	struct inode *inode;
	struct dentry *dentry;
	off_t offset;
	vfs_fs_t *fs;
};

/* vfs.c */
vfs_fs_t *vfs_alloc(const char *name, vfs_ops_t *ops);
vfs_fs_t * vfs_find_fs(const char *type);
int vfs_register_fs(vfs_fs_t *fs);
struct file *vfs_file_new(struct inode *inode, char *name);
struct file *vfs_file_get(int fd);
void vfs_file_free(struct file *file);
void vfs_init();

/* vfs_ops.c - internal/kernel ops */
int vfs_open(struct file **file, const char *path, int oflags, mode_t mode);
int vfs_close(struct file *file);
size_t vfs_read(struct file *file, void *buf, size_t nbyte);
off_t vfs_write(struct file *file, void *buf, size_t nbyte);
off_t vfs_seek(struct file *file, off_t offset, int whence);
int vfs_ioctl(struct file *file, int request, char *args);
int vfs_chdir(const char *path);
int vfs_stat(const char *path, struct stat *buf);
int vfs_stat64(const char *path, struct stat64 *buf);
int vfs_fstat64(struct inode *inode, struct stat64 *buf);

/* vfs_util.c - VFS Utility functions */
int put_file(struct file *file, int flags);
int put_file2(struct file *file, int givenfd, int flags);

// FIXME: further extract this later (make it an independent header)
#include <fs/sys.h>

/* device.c - device fs */
#define INITRD_DEV 0x400
#define ATA0_0_DEV 0x301
#define ATA0_3_DEV 0x303

#define SECTOR_SIZE 512

#define MAJOR(x) ((x & ~0xFF) >> 8)
#define MINOR(x) (x & 0xFF)

struct device {
    dev_t dev;
    uint16_t type;
    vfs_read_inode_t read;
    vfs_write_inode_t write;
    vfs_ioctl_inode_t ioctl;
};

void device_register(uint16_t type, dev_t dev, vfs_read_inode_t read, vfs_write_inode_t write, vfs_ioctl_inode_t ioctl);
struct device *get_device(uint16_t type, dev_t dev);
#endif