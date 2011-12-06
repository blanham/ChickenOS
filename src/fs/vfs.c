//#include "include/kernel/types.h"
#include <kernel/types.h>
#include <kernel/memory.h>
#include <kernel/thread.h>
#include <stdio.h>
#include <string.h>
#include "vfs.h"
//#include "ext2fs.h"
#include "initrd.h"
#define UNUSED(X) X = X
vfs_fs_t *main_fs;
uint8_t file_count = 0;
struct file files[100];


struct file *open_files[100];
/*
int vfs_read_blocknum(vfs_fs_t *fs, uint8_t *dst, uint32_t blocknum)
{

	return i-1;

}*/
struct file root_storage;
struct file *root = &root_storage;

//wrap malloc(), so we can later call c_os's kernel allocator
void *vfs_malloc(size_t size)
{
	UNUSED(size);
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


int vfs_register_fs(vfs_fs_t *fs)
{
	if(fs == NULL)
		return -1;
	main_fs = fs;

	return 0;
}

struct file *dir_search(struct file *dir, char *name)
{
	struct file *elem = dir->dir;
	if(elem == NULL)
		return NULL;
	if(!strncmp(dir->name, name, 256))
		return dir;

	for(;elem->file != NULL; elem = elem->file)
	{	
		if(!strncmp(elem->name, name, 256))
			return elem;
	}
	return elem;
}

struct file * pathsearch(struct file *dir, char *_path)
{
	//UNUSED(vfs);
	UNUSED(_path);
	char *saveptr;
	//if dir is not a directory, we need to throw an error
	//char *path = _path;
//	char *path = malloc(500);
	char path[500];
	strcpy(path, _path);
	char *token = (char *)strtok_r(path, "/", &saveptr);
	if( token == NULL)
		return NULL;
	struct file *elem = dir_search(dir, token);
	if(elem == NULL){
		printf("dir search or insert broken\n");
		return NULL;
	}
	while((token = (char *)strtok_r(NULL, "/", &saveptr)) != NULL)
	{
		elem = dir_search(elem, token);
		if(elem == NULL)
		{
			printf("file not found?\n");
			return NULL;
		}
	}

	

	return elem;
}

int insert_file(struct file *dir, struct file *file)
{
	struct file *elem = dir->dir;
	
	if(elem == NULL)
	{
		dir->dir = file;
		file->parent = dir;
		return 0;
	}
	
	for(;elem->file != NULL; elem = elem->file);
		elem->file = file;
	elem->file->parent = dir;	
	return 0;
}



struct file *file_new(char *name)
{
	struct file *new = file_alloc();//malloc(sizeof(struct file));
	kmemset((uint8_t *)new,0, sizeof(*new));
	strcpy(new->name, name);
	return new;
}

struct file *file_new3(char *name, uint8_t type, uint16_t device)
{
	struct file *new = file_alloc();//malloc(sizeof(struct file));
	kmemset((uint8_t *)new,0, sizeof(*new));
	strcpy(new->name, name);
	new->device = device;
	new->type = type;
	return new;
}


struct file *file_new2(vfs_fs_t *fs, char *name, uint32_t inode)
{
	struct file *new = file_alloc();//malloc(sizeof(struct file));
	kmemset((uint8_t *)new,0, sizeof(*new));
	strcpy(new->name, name);
	new->inode = inode;
	new->fs = fs;
	return new;
}




struct file * dir_new(struct file *dir, char *name)
{
	struct file *dot = file_new(".");
	struct file *dotdot = file_new("..");
	struct file *new = file_new(name);
	insert_file(dir, new);	
	

	insert_file(new, dot);
	insert_file(new, dotdot);
	dot->dir = new->dir;
	dotdot->dir = new->parent->dir;
	
	return new;

}
void dir_list(struct file *dir)
{
	struct file * elem = dir->dir;
	if(dir == NULL)
	{
		printf("passed NULL dir to dir_list\n");
		return;
	}
	if(elem == NULL)
	{
		printf("File %s is not a directory\n", dir->name);
		return;
	}
	printf("Directory listing for %s\n",elem->parent->name);
	for(;elem != NULL; elem = elem->file)
		printf("%s\n",elem->name);

}

void vfs_init()
{
	strcpy(root->name,"/");
	root->parent = root;
	struct file *dot = file_new(".");
	struct file *dotdot = file_new("..");
	printf("Initialzing VFS\n");
//	printf("dotdot %x %x\n", (uintptr_t)dotdot->dir, (uintptr_t)root->dir);	
	
	insert_file(root, dot);
	insert_file(root, dotdot);
	struct file *test = file_new("test");
	insert_file(root, test);
	dot->dir = root->dir;
	dotdot->dir = root->dir;
	struct file * dir = dir_new(root, "foo");
	dir_new(dir, "dev");

	thread_current()->cur_dir = root; 
//	dir_list(root);
//	dir_list(dir);
}


#include <stdio.h>
struct file * vfs_root()
{


	return root;
}
uint8_t *ramdisk;
void vfs_mount(vfs_fs_t *fs, struct file *dir)
{
	UNUSED(fs);
	UNUSED(dir);
	


}

vfs_fs_t *root_fs;
int old_main(int argc, char **argv)
{
	UNUSED(argc);
	UNUSED(argv);
	

	if(argc < 2)
		return -1;
		
//	ext2_init(root_fs,argv[1]);
//	ext2(argv[1]);
/*	return 0;
	FILE *fp = fopen(argv[1], "rb");
	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	printf("SIZE %x\n",(uint32_t)size);
	ramdisk = malloc(size);
	fread(ramdisk, size, 1, fp);
	if(ramdisk == NULL)
		printf("fuck\n");
	initrd_init((uintptr_t)ramdisk, (uintptr_t)ramdisk + (uintptr_t)size);
	fclose(fp);

	uint8_t *first_sector = malloc(SECTOR_SIZE);
	if(initrd_read_block(first_sector, 0) < 0)
		return -1;
	fp = fopen("out","wb");
	fwrite(first_sector, SECTOR_SIZE,1,fp);
	fclose(fp);
	printf("%s\n",first_sector+3);
//	uint16_t *test = (uint16_t *)first_sector;
	printf("test %.2x%.2x\n", first_sector[510],first_sector[511]);
*/
	return 0;
}
/*int open(const char *path, int oflag, ...)*/
int fd_new()
{
	static int fd = 3;
	

	return fd++;


}

int open(const char *path, int oflag)
{
	UNUSED(path);
	UNUSED(oflag);
	struct file *dir = thread_current()->cur_dir;
	struct file * fp = pathsearch(dir, (char *)path);
	if(fp == NULL)
		return -1;
	int fd = fd_new();
	open_files[fd] = fp;

	return fd;
}

size_t vfs_read(struct file *file, void *buf, size_t nbyte)
{
	int ret;

	if((file->type & (FILE_CHAR | FILE_BLOCK)) != 0){
		printf("REAFDASDF\n");
		ret = device_read(file, buf, file->offset,nbyte);
		printf("ASDFASDF");
	}else{
		if(file->fs == NULL || file->fs->ops->read == NULL)
			return -1;

		ret = file->fs->ops->read(file->fs, file->inode, nbyte, file->offset, buf);
	}
	file->offset += ret; 
	return ret;

}

size_t vfs_write(struct file *file, void *buf, size_t nbyte)
{
	int ret;

	if((file->type & (FILE_CHAR | FILE_BLOCK)) != 0){
		ret = device_write(file, buf, file->offset,nbyte);
	}else{
		if(file->fs == NULL || file->fs->ops->write == NULL)
			return -1;

		ret = file->fs->ops->write(file->fs, file->inode, nbyte, file->offset, buf);
	}
	file->offset += ret; 
	return ret;

}


//off_t vfs_seek(struct file *
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

int sys_chdir(const char *path)
{
	path = path;
	return -1;
}
