#ifndef C_OS_VFS_OPS_H
#define C_OS_VFS_OPS_H
#include <sys/types.h>
//#include <stdarg.h>
//#include <stdint.h>
//#include <stddef.h>
//#include <dirent.h>
//#include <fcntl.h>
//#include <sys/select.h>
//#include <sys/stat.h>
//#include <sys/time.h>
//#include <sys/types.h>
//#include <sys/uio.h>
//#include <poll.h>

#include <dirent.h>
#include <fs/vfs.h>

typedef struct inode inode_t;
typedef struct vfs_fs vfs_fs_t;

typedef size_t	  (*vfs_read_inode_t)  (inode_t *, uint8_t *buf, size_t length, off_t offset);
typedef size_t	  (*vfs_write_inode_t) (inode_t *, uint8_t *buf, size_t length, off_t offset);
typedef int    	  (*vfs_ioctl_inode_t) (inode_t *, int request, char *arg);
typedef ssize_t   (*vfs_readlink_t)    (inode_t *, char *path, size_t length);
typedef inode_t * (*vfs_namei_old_t)   (inode_t *, const char *path);
typedef inode_t * (*vfs_namei_t)       (inode_t *, const char *path, size_t length);

typedef int       (*vfs_getdents_t)    (inode_t *, struct dirent *, int, off_t*);
typedef int       (*vfs_read_sb_t)     (inode_t *, vfs_fs_t *fs);
typedef uint64_t  (*vfs_bmap_t)        (inode_t *, off_t);
typedef int       (*vfs_mount_t)       (uint16_t dev);

typedef struct vfs_superblock_ops {
	vfs_read_inode_t read;
	vfs_write_inode_t write;
	vfs_ioctl_inode_t ioctl;
	//vfs_creat_t creat;
	//vfs_mkdir_t mkdir;
	vfs_bmap_t bmap;
	vfs_read_sb_t read_sb;
	vfs_namei_t namei;
	//vfs_namei2_t namei2;
	vfs_readlink_t readlink;
   	vfs_getdents_t getdents;

	int refs;
} vfs_ops_t;

#endif