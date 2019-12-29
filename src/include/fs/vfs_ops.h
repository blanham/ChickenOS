#ifndef C_OS_VFS_OPS_H
#define C_OS_VFS_OPS_H
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <poll.h>

struct inode;
typedef struct vfs_fs vfs_fs_t;

typedef size_t (*vfs_read_inode_t) (struct inode *, void *_buf, size_t length, off_t offset);
typedef size_t (*vfs_write_inode_t) (struct inode *, void *_buf, size_t length, off_t offset);
typedef struct inode * (*vfs_namei_t) (struct inode *dir, char *path);
typedef int (*vfs_read_sb_t) (vfs_fs_t *fs, uint16_t dev);
typedef int (*vfs_mount_t) (uint16_t dev);
typedef int (*vfs_getdents_t)(struct inode *, struct dirent *, int, off_t*);

typedef struct vfs_superblock_ops {
	vfs_read_inode_t read;
	vfs_write_inode_t write;
	//vfs_creat_t creat;
	//vfs_mkdir_t mkdir;
	vfs_read_sb_t read_sb;
	vfs_namei_t namei;
   	vfs_getdents_t getdents;
} vfs_ops_t;

#endif