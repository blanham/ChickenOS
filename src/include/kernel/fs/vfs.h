#ifndef C_OS_VFS_H
#define C_OS_VFS_H
#include <kernel/types.h>
#include <kernel/list.h>
/* only define when STDIO is not included */
#ifndef _STDIO_H
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif
//default block/buffer size
#define SECTOR_SIZE 512
//modes, actually from ext2, will use them globally for ease
#define S_IFMT 		0xF000 	//format mask
#define S_IFSOCK 	0xA000 	//socket
#define S_IFLNK 	0xC000 	//symbolic link
#define S_IFREG 	0x8000 	//regular file
#define S_IFBLK 	0x6000 	//block device
#define S_IFDIR 	0x4000 	//directory
#define S_IFCHR 	0x2000 	//character device
#define S_IFIFO 	0x1000 	//fifo
#define S_ISUID 	0x0800 	//SUID
#define S_ISGID 	0x0400 	//SGID
#define S_ISVTX 	0x0200 	//sticky bit
#define S_IRWXU 	0x01C0 	//user mask
#define S_IRUSR 	0x0100 	//read
#define S_IWUSR 	0x0080 	//write
#define S_IXUSR 	0x0040 	//execute
#define S_IRWXG 	0x0038 	//group mask
#define S_IRGRP 	0x0020 	//read
#define S_IWGRP 	0x0010 	//write
#define S_IXGRP 	0x0008 	//execute
#define S_IRWXO 	0x0007 	//other mask
#define S_IROTH 	0x0004 	//read
#define S_IWOTH 	0x0002 	//write
#define S_IXOTH 	0x0001 	//execute

enum file_type { FILE_CHAR  = 0x2000, 
				 FILE_DIR   = 0x4000,
				 FILE_BLOCK = 0x6000}; 

typedef struct vfs_fs vfs_fs_t;
struct inode;

typedef size_t (*vfs_read_inode_t) (struct inode *, void *_buf, size_t length, off_t offset);
typedef size_t (*vfs_write_inode_t) (struct inode *, void *_buf, size_t length, off_t offset);
typedef struct inode * (*vfs_namei_t) (struct inode *dir, char *path);
typedef int (*vfs_read_sb_t) (vfs_fs_t *fs, uint16_t dev);
typedef int (*vfs_mount_t) (uint16_t dev);
typedef struct vfs_superblock_ops {
	vfs_read_inode_t read;
	vfs_write_inode_t write;
	//vfs_creat_t creat;
	//vfs_mkdir_t mkdir;
	vfs_read_sb_t read_sb;
	vfs_namei_t namei;
} vfs_ops_t;

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
	uint32_t inode_num;
	uint16_t mode;
	uint16_t pad;
	uint32_t size;
	uint32_t time;
	uint16_t rdev;
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
	uint16_t dev;
	uint32_t offset;
	vfs_fs_t *fs;	
};





vfs_fs_t *vfs_alloc();

int vfs_register_fs(vfs_fs_t *fs);

struct inode * pathsearch(struct inode *dir, char *_path);
void vfs_init();
void vfs_mount_root(uint16_t dev, char *type);

struct file *vfs_open(char *path);
size_t vfs_read(struct file *file, void *buf, size_t nbyte);
off_t vfs_write(struct file *file, void *buf, size_t nbyte);
off_t vfs_seek(struct file *file, off_t offset, int whence);
int vfs_chdir(const char *path);

/* ops.c - standard file ops */
/*int open(const char *path, int oflag, ...);*/
int sys_open(const char *path, int oflag, ...);

ssize_t sys_read(int fildes, void *buf, size_t nbyte);
ssize_t sys_write(int filedes, void *buf, size_t nbyte);

/*int creat(const char *path, mode_t mode);*/
//int sys_creat(const char *path, mode_t mode);

off_t sys_lseek(int fildes, off_t offset, int whence);




//device.c
#define INITRD_DEV 0x400
typedef uint16_t dev_t;
#define MAJOR(x) ((x & 0xFF00) >> 8)
#define MINOR(x) (x & 0xFF)
typedef size_t (*char_read_fn)(uint16_t dev, void *buf, off_t offset, size_t count);
typedef size_t (*char_write_fn)(uint16_t dev, void *buf, off_t offset, size_t count);


typedef size_t (*block_read_fn) (uint16_t dev, void *buf, int block);
typedef size_t (*block_write_fn)(uint16_t dev, void *buf, int block);

void  device_register(uint16_t type, dev_t dev, void *read, void *write);
typedef int(*block_access_fn)(void *aux, void *buf, int block);


size_t char_device_read(uint16_t dev, void *buf, off_t offset, size_t nbyte);
size_t char_device_write(uint16_t dev, void *buf, off_t offset, size_t nbyte);
size_t block_device_read(uint16_t dev, void *buf, uint32_t block);
size_t block_device_readn(uint16_t dev, void *buf, uint32_t block, off_t offset, size_t nbyte);

int read_block_at(uint16_t dev, void * _buf, int block,int block_size, off_t offset, size_t nbytes);

int read_block(uint16_t dev, void * _buf, int block, int block_size);

int write_block_at(uint16_t dev, void * _buf, int block,int block_size, off_t offset, size_t nbytes);

int write_block(uint16_t dev, void * _buf, int block, int block_size);




#endif
