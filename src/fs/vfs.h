#ifndef C_OS_VFS_H
#define C_OS_VFS_H
//#include "../include/kernel/types.h"
/* only define when STDIO is not included */
#include <kernel/types.h>
#ifndef _STDIO_H
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif
#define SECTOR_SIZE 512

enum file_type { FILE_CHAR = 1, FILE_BLOCK = 2}; 
struct file;

typedef int (*vfs_read_inode_t) (void * fs, uint32_t inode, size_t length, size_t offset, uint8_t *buffer);
typedef uint32_t (*vfs_write_inode_t) (void *fs, uint32_t inode, size_t length, size_t offset, uint8_t *buffer);
typedef int (*vfs_read_dir_t) (void *fs, struct file *dir, char *path);
typedef int (*vfs_read_sb_t) (vfs_fs_t *fs);

typedef struct vfs_superblock_ops {
	vfs_read_inode_t read;
	vfs_write_inode_t write;
	vfs_read_sb_t read_sb;
	vfs_read_dir_t read_dir;
} vfs_ops_t;
typedef struct vfs_fs {
	uint8_t *superblock;
	uint16_t type;
	void *fs;
	vfs_ops_t *ops;
} vfs_fs_t;



typedef struct vfs_file {
	vfs_fs_t *fs;
	uint32_t inode;//use as cluster number?
	char *name;//perhaps use name[255/256] instead?NO! what about other fses?
} vfs_file_t;


struct inode {
	
	char name[256];
	uint16_t mode;
	uint16_t device;
	uint32_t inode_num;
	uint32_t offset;
	vfs_fs_t *fs;	

};

typedef struct inode_cached {
	uint32_t group;
	uint32_t inode;
} inode_cached_t;

struct file {
	struct file *parent;
	//left
	struct file *dir;
	//right
	struct file *file;
	char name[256];
	uint8_t type;
	uint16_t device;
	uint32_t inode;
	uint32_t offset;
	vfs_fs_t *fs;	
};

extern struct file *root;



struct file *file_new2(vfs_fs_t *fs, char *name, uint32_t inode);
struct file *file_new3(char *name, uint8_t type, uint16_t device);
int insert_file(struct file *dir, struct file *file);

struct file * pathsearch(struct file *dir, char *_path);
void vfs_init();
/*int open(const char *path, int oflag, ...);*/
int open(const char *path, int oflag);

ssize_t read(int fildes, void *buf, size_t nbyte);
ssize_t write(int filedes, void *buf, size_t nbyte);

/*int creat(const char *path, mode_t mode);*/
//int creat(const char *path, uint32_t mode);

off_t lseek(int fildes, off_t offset, int whence);
typedef size_t (*device_read_fn)(uint16_t dev, void *buf, off_t offset, size_t count);
typedef size_t (*device_write_fn)(uint16_t dev, void *buf, off_t offset, size_t count);


#define MAJOR(x) ((x & 0xFF00) >> 8)
#define MINOR(x) (x & 0xFF)

size_t device_read(struct file *file, void *buf, off_t offset, size_t nbyte);
size_t device_write(struct file *file, void *buf, off_t offset, size_t nbyte);
void device_file_register(uint16_t dev, device_read_fn read, device_write_fn write);
int device_file_create(uint16_t dev, uint8_t type, char *name);
#endif
