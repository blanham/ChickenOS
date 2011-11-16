//#include "include/kernel/types.h"
//#include <kernel>
#include <stdio.h>
#include <stdlib.h>
#include "vfs.h"
#include "ext2fs.h"
#define UNUSED(X) X = X
//wrap malloc(), so we can later call c_os's kernel allocator
void *vfs_malloc(size_t size)
{
	return malloc(size);
}

typedef struct vfs_superblock_ops {
	vfs_read_inode_t read;
	vfs_write_inode_t write;
} vfs_superblock_ops_t;
typedef struct vfs_superblock {
	vfs_superblock_ops_t;
} vfs_superblock_t;




typedef struct vfs_fs {
	vfs_superblock_t superblock;

} vfs_fs_t;

typedef struct inode_cached {
	uint32_t group;
	uint32_t inode;



} inode_cached_t;





typedef struct vfs_file {
	void *filesystem;
	uint32_t inode;
	char *name;//perhaps use name[255/256] instead?NO! what about other fses?
	
	

} vfs_file_t;

struct file;
struct file {
	struct file *parent;
	//left
	struct file *dir;
	//right
	struct file *file;
	struct vfs_file *vfs;	
};

struct file root;

void vfs_init()
{
	root.parent = &root;
	root.dir = NULL;
	root.file = NULL;


}
int pathsearch(char *path)
{





}


/*int open(const char *path, int oflag, ...)*/
int open(const char *path, int oflag)
{
	UNUSED(path);
	UNUSED(oflag);
	return -1;
}
ssize_t read(int fildes, void *buf, size_t nbyte)
{
	UNUSED(fildes);
	UNUSED(buf);
	UNUSED(nbyte);

	return -1;
}
ssize_t write(int fildes, void *buf, size_t nbyte)
{
	UNUSED(fildes);
	UNUSED(buf);
	UNUSED(nbyte);


	return -1;
}
/*int creat(const char *path, mode_t mode)*/
int creat(const char *path, uint32_t mode)
{
	UNUSED(path);
	UNUSED(mode);
	return -1;
}
off_t lseek(int fildes, off_t offset, int whence)
{
	UNUSED(fildes);
	UNUSED(offset);
	UNUSED(whence);
	return -1;
}
#include <stdio.h>


int main(int argc, char **argv)
{
	UNUSED(argc);
	UNUSED(argv);
	if(argc != 3)
	{
		printf("invalid number of args!\n");
		return -1;
	}
	if(argv[1][0] != '-')
	{
		printf("no flag!\n");
		return -1;
	}
	switch (argv[1][1])
	{
		case 'c':
		case 'l':
		case 'o':
		case 'r':
		case 'w':
			//printf("not imple\n");
			ext2(argv[2]);
			break;
		default:
			printf("invalid flag\n");
		return -1;



	}
	return 0;
}

