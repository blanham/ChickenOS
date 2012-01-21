/*	ChickenOS - fs/vfs.c - virtual file system main
 *	Patterned after the Linux 2.4 vfs
 *	
 */
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
vfs_fs_t *root_fs;
vfs_fs_t * filesystems[10];

struct inode *open_inodes[100];
struct file *root;

struct inode * vfs_namei(struct inode *dir, char *file)
{
	if(dir->flags & I_MOUNT && dir != root->inode)
	{
		PANIC("traversing mounts not yet implemented");
	} 

	if((dir->mode & S_IFDIR) == 0)
	{
		//TODO: return is not directory error
		PANIC("calling vfs_namei on a non-directory inode");
	}
	//need to lookup things in a cache
	return dir->fs->ops->namei(dir, file);
}

struct inode * vfs_pathsearch(struct file *dir, char *_path)
{
	char *saveptr, *tok, *filename, *path;
	struct inode *res = dir->inode;
	path = strdup(_path);
	filename = kcalloc(255, 1);;
	
	if(path[0] == '/')
		res = root->inode;
	else
		res = thread_current()->cur_dir->inode;	
	
	if((tok = (char *)strtok_r(path, "/", &saveptr)) == NULL)
		return NULL;
		
	if((res = vfs_namei(res, tok)) == NULL)
	{
		printf("dir search or insert broken\n");
		printf("path %s\n",path);
		while(1);
		return NULL;
	}

	strcpy(filename, tok);
	while((tok = (char *)strtok_r(NULL, "/", &saveptr)) != NULL)
	{
		strcpy(filename, tok);
		if((res = vfs_namei(res, tok)) == NULL)
		{
			printf("file not found?\n");
			return NULL;
		}
	}
	//FIXME: needs to use strcpy or strncpy
	kmemcpy((uint8_t *)_path, (uint8_t *)filename, 10);
//	strcpy(_path, filename);
	kfree(path);
	return res;
}
//FIXME: use linked list or hash
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

	//FIXME: use vfs_new_file instead:
	//root = vfs_new_file(fs->superblock->root, "/");
	strcpy(root->name, "/");
	root->dev = dev;
	root->offset = 0;
	root->fs = fs;
	thread_current()->cur_dir = root;
	root_fs = fs;

	printf("Mounted %s fs @ dev %i:%i as root\n",type, MAJOR(dev),MINOR(dev));
	return;	

error:	
	printf("stage %i\n",ret);	
	PANIC("mounting root filesystem failed");
}

vfs_fs_t *vfs_alloc()
{
	vfs_fs_t *new = kcalloc(sizeof(*new),1);
	new->superblock = kcalloc(sizeof(vfs_sb_t),1);
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
	root = kcalloc(sizeof(struct inode),1);

	printf("Initialzing VFS\n");
	if(ext2_init() < 0)
		PANIC("failed to init ext2 fs\n");

}
/*

int vfs_mount(const char *device, struct file *dir, char *type)
{
	dev_t dev;
	vfs_fs_t *fs;
	struct file *block = vfs_open(device);
	if(file->mode & S_IFBLK)
	{
		dev = block->rdev	
	}else{
		//not a block device
		vfs_close(block);
		return -1;
	}
	//would be nice to be able to autodetect the fs
	if((fs = vfs_find_fs(type)) == NULL)
		goto error;
	ret++;
	
	if(fs->ops->read_sb == NULL)
	{
		printf("Invalid fs! No read_sb defined!\n");
		return -1;
	}
	
	if(fs->ops->read_sb(fs, dev) < 0)
		goto error;
	
	//needs to be a list
	//insert root inode	
//	open_inodes[0] = fs->superblock->root;
	
	strcpy(root->name, "/");
	root->dev = dev;
	root->offset = 0;
	root->fs = fs;
	thread_current()->cur_dir = root;
	root_fs = fs;
	printf("Mounted %s fs @ dev %i:%i as root\n",type, MAJOR(dev),MINOR(dev));


}
*/
struct file *vfs_file_new(struct inode *inode, char *name)
{
	struct file *new = kcalloc(sizeof(*new),1);
	new->inode = inode;
	new->fs = inode->fs;
	strcpy(new->name, name);
	return new;
}

int vfs_file_free(struct file *file)
{
	struct inode *inode = file->inode;
	kfree(file);
	//decrement reference count
	//if 0 take out of cache
	inode = inode;

	return 0;
}

struct file *vfs_open(char *path)
{
	struct file *cur = thread_current()->cur_dir;
	struct inode *lookup = vfs_pathsearch(cur, path);
	if(lookup == NULL)
		return NULL;
	struct file *new = vfs_file_new(lookup, path);
	return new;
}

int vfs_close(struct file *file)
{
	if(file == NULL)
		return -1;
	
	vfs_file_free(file);

	return 0;
}

size_t vfs_read(struct file *file, void *buf, size_t nbyte)
{
	int ret = 0;

	if(file == NULL || buf == NULL)
		return -1;
	if((file->inode->mode & S_IFCHR) != 0){
		ret = char_device_read(file->inode->rdev, 
			buf, file->offset, nbyte);
	}else if((file->inode->mode & S_IFBLK) != 0){
		ret = block_device_readn(file->inode->rdev, 
			buf, 0, file->offset, nbyte);
	}else if((file->inode->mode & S_IFREG) != 0){
		if(file->fs == NULL || file->fs->ops->read == NULL)
			return -1;
		ret = file->fs->ops->read(file->inode, buf,
			nbyte, file->offset);
	}
	file->offset += ret; 
	return ret;

}

off_t vfs_write(struct file *file, 
	void *buf, size_t nbyte)
{
	int ret = 0;
	if(file == NULL || buf == NULL)
		return -1;
	if((file->inode->mode & S_IFCHR) != 0){
		ret = char_device_write(file->inode->rdev, 
			buf, file->offset, nbyte);
	}else if((file->inode->mode & S_IFBLK) != 0){
	//	ret = block_device_readn(file->inode->rdev, 
	//		buf, 0, file->offset, nbyte);
		return -1;
	}else if((file->inode->mode & S_IFREG) != 0){
		if(file->fs == NULL || file->fs->ops->write == NULL)
			return -1;
		ret = file->fs->ops->write(file->inode, buf,
			nbyte, file->offset);
	}
	file->offset += ret; 
	return ret;
}

int  vfs_ioctl(struct file *file, 
	int request, ...)
{
	int ret = 0;
	if(file == NULL)
		return -1;
	if((file->inode->mode & S_IFCHR) != 0){
		ret = char_device_ioctl(file->inode->rdev, 
			request, NULL);
	}else if((file->inode->mode & S_IFBLK) != 0){
	//	ret = block_device_readn(file->inode->rdev, 
	//		buf, 0, file->offset, nbyte);
		return -1;
	}else if((file->inode->mode & S_IFREG) != 0){
	//	if(file->fs == NULL || file->fs->ops->write == NULL)
	//		return -1;
	//	ret = file->fs->ops->write(file->inode, buf,
	//		nbyte, file->offset);
		return -1;
	}
	return ret;
}

off_t vfs_seek(struct file *file, off_t offset, int whence)
{
	switch(whence)
	{
		case SEEK_SET:
			file->offset = offset;
			break;
		case SEEK_CUR:
			file->offset += offset;
			break;
		case SEEK_END:
			//file->offset = file->end + offset;
			//break;
		default:
			return 0;

	}

	return file->offset;
}

int vfs_chdir(const char *_path)
{
	struct file *file;
	int ret = 0;
	char *path = strdup(_path);
	file  = vfs_open(path);
	if(file != NULL)
	{
		vfs_close(thread_current()->cur_dir);
		thread_current()->cur_dir = file;
		ret = 1;
	}
	kfree(path);
	return ret;
}
