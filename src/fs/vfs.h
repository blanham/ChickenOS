#ifndef C_OS_VFS_H
#define C_OS_VFS_H
//#include "../include/kernel/types.h"
/* only define when STDIO is not included */
#include <stdint.h>
#ifndef _STDIO_H
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif
typedef uint8_t * (*vfs_read_inode_t) (void * fs, uint32_t inode, size_t length, size_t offset);
typedef uint32_t (*vfs_write_inode_t) (void *fs, uint32_t inode, size_t length, size_t offset, uint8_t *buffer);




/*int open(const char *path, int oflag, ...);*/
int open(const char *path, int oflag);
ssize_t read(int fildes, void *buf, size_t nbyte);
ssize_t write(int filedes, void *buf, size_t nbyte);

/*int creat(const char *path, mode_t mode);*/
int creat(const char *path, uint32_t mode);

off_t lseek(int fildes, off_t offset, int whence);

#endif
