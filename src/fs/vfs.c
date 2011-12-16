#include <kernel/common.h>
#include <kernel/types.h>
#include <kernel/memory.h>
#include <kernel/thread.h>
#include <kernel/vm.h>
#include <kernel/fs/vfs.h>
#include <kernel/fs/ext2/ext2.h>
#include <mm/liballoc.h>
#include <stdio.h>
#include <string.h>
vfs_fs_t *main_fs;
vfs_fs_t * filesystems[10];
//FIXME: temporary storage vars



struct file files[100];
uint8_t file_count = 0;

struct inode *open_inodes[100];
struct file *root;
vfs_fs_t *root_fs;
/*
//wrap malloc(), so we can later call c_os's kernel allocator
void *vfs_malloc(size_t size UNUSED)
{
	//return malloc(size);
	return NULL;
}



struct file *file_alloc()
{
	struct file *ret = &files[file_count];
	file_count++;
	if(file_count > 127)
		return NULL;
	else
		return ret;
}


*/


struct inode * vfs_namei(struct inode *dir, char *file)
{
	if(dir->flags & I_MOUNT && dir != root->inode)
	{
		PANIC("traversing mounts not yet implemented");

	} 

	if((dir->mode & S_IFDIR) == 0)
	{
		PANIC("calling vfs_namei on a non-directory inode");
	}
	//need to lookup things in a cache
	return dir->fs->ops->namei(dir, file);
}

struct inode * vfs_pathsearch(struct inode *dir, char *_path)
{
	char * saveptr;
	char * tok;
	struct inode *res = dir;
	char path[500];
	strcpy(path, _path);

	if(path[0] == '/')
		res = root->inode;
	
	if((tok = (char *)strtok_r(path, "/", &saveptr)) == NULL)
		return NULL;
	
	if((res = vfs_namei(res, tok)) == NULL)
	{
		printf("dir search or insert broken\n");
		return NULL;
	}

	while((tok = (char *)strtok_r(NULL, "/", &saveptr)) != NULL)
	{
		if((res = vfs_namei(res, tok)) == NULL)
		{
			printf("file not found?\n");
			return NULL;
		}
	}

	return res;
}
//void vfs_mount(uint16_t dev, char *path, char * type)

vfs_fs_t * vfs_find_fs(char *type)
{
	vfs_fs_t *find = NULL;
	int i;
	for(i = 0; i < 10; i++)
	{
		if(!strncmp(type, filesystems[i]->name, 10))
		{
			find = filesystems[i];
			return find;
		}
	}

	return find;
}
void vfs_file_print(struct file *file)
{
	printf("File: %s\n",file->name);
	printf("Device %x\n",file->dev);

}
void vfs_mount_root(uint16_t dev, char *type)
{
	vfs_fs_t *fs;
	int ret = 0;
	if((fs = vfs_find_fs(type)) == NULL)
		goto error;
	ret++;
	
	if(fs->ops->read_sb == NULL)
		goto error;	
	ret++;
	if(fs->ops->read_sb(fs, dev) < 0)
		goto error;
	//needs to be a list	
	open_inodes[0] = fs->superblock->root;
	root->inode = fs->superblock->root;
	strcpy(root->name, "/");
	root->dev = dev;
	root->offset = 0;
	root->fs = fs;
	thread_current()->cur_dir = root;
//	vfs_file_print(thread_current()->cur_dir);
	root_fs = fs;
	printf("Mounted %s fs @ dev %i:%i as root\n",type, MAJOR(dev),MINOR(dev));

	return;	

error:	
	printf("stage %i\n",ret);	
	PANIC("mounting root filesystem failed");
}

vfs_fs_t *vfs_alloc()
{
	vfs_fs_t *new = kmalloc(sizeof(*new));
	new->superblock = kmalloc(sizeof(vfs_sb_t));
	return new;
}

int vfs_register_fs(vfs_fs_t *fs)
{
	//return error if name blank
	if(fs->name[0] == '\0')
		return -1;

	for(int i = 0; i < 10; i++)
	{
		if(filesystems[i] == NULL)
		{
			filesystems[i] = fs;
			return 1;
		}
	}

	return 0;
}
void vfs_init()
{
	root = kmalloc(sizeof(struct inode));

	printf("Initialzing VFS\n");
	if(ext2_init() < 0)
		PANIC("failed to init ext2 fs\n");

	vfs_mount_root(0x0400, "ext2");
	thread_current()->cur_dir = root; 
}
/*

void vfs_mount(vfs_fs_t *fs UNUSED, struct file *dir UNUSED)
{
	


}
*/
/*int open(const char *path, int oflag, ...)*/
/*
int fd_new()
{
	static int fd = 3;
	

	return fd++;


}

int open(const char *path, int oflag UNUSED)
{
	struct file *dir = thread_current()->cur_dir;
	struct file * fp = vfs_pathsearch(dir, (char *)path);
	if(fp == NULL)
		return -1;
	int fd = fd_new();
	open_files[fd] = fp;

	return fd;
}
*/
size_t vfs_read(struct file *file UNUSED, void *buf UNUSED, size_t nbyte UNUSED)
{
	int ret = 0;
/*
	if(file == NULL || buf == NULL)
		return -1;
	if((file->type & (FILE_CHAR | FILE_BLOCK)) != 0){
//		ret = device_read(*file->type*//*file, buf, file->offset,nbyte);
	}else{
		if(file->fs == NULL || file->fs->ops->read == NULL)
			return -1;

	//	ret = file->fs->ops->read(file->fs, file->inode, nbyte, file->offset, buf);
	}
	file->offset += ret; */
	return ret;

}

size_t vfs_write(struct file *file UNUSED, void *buf UNUSED, size_t nbyte UNUSED)
{
	int ret = 0;
/*
	if(file == NULL || buf == NULL)
		return -1;

	if((file->type & (FILE_CHAR | FILE_BLOCK)) != 0){
//		ret = device_write(file, buf, file->offset,nbyte);
	}else{
		if(file->fs == NULL || file->fs->ops->write == NULL)
			return -1;

	//	ret = file->fs->ops->write(file->fs, file->inode, nbyte, file->offset, buf);
	}
	file->offset += ret; */
	return ret;
}


//off_t vfs_seek(struct file *i
/*
ssize_t read(int fildes, void *buf, size_t nbyte)
{
	struct file *fp = open_files[fildes];
	if(fp == NULL)
		return -1;
	return vfs_read(fp, buf, nbyte);
}
ssize_t write(int fildes, void *buf, size_t nbyte)
{
	struct file *fp = open_files[fildes];
	if(fp == NULL)
		return -1;
	return vfs_write(fp, buf, nbyte);
}*/
/*int creat(const char *path, mode_t mode)*/
/*int creat(const char *path UNUSED, uint32_t mode UNUSED)
{
	return -1;
}
off_t lseek(int fildes UNUSED, off_t offset UNUSED, int whence UNUSED)
{
	return -1;
}

int sys_chdir(const char *path)
{
	path = path;
	return -1;
}*/
