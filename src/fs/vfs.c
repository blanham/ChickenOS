/*	ChickenOS - fs/vfs.c - virtual file system main
 *	Patterned after the Linux 2.4 vfs
 *	
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <kernel/common.h>
#include <kernel/memory.h>
#include <kernel/thread.h>
#include <mm/vm.h>
#include <fs/vfs.h>
#include <fs/ext2/ext2.h>
#include <mm/liballoc.h>
#include <fcntl.h>
#include <errno.h>
//FIXME: Probably shouldn't be a table
vfs_fs_t * filesystems[10];
vfs_fs_t *root_fs;

//FIXME: This is incorrect, should he hashed or something
struct inode *open_inodes[100];
struct file *root;


struct file *vfs_file_new(struct inode *inode, char *name)
{
	struct file *new = kcalloc(sizeof(*new),1);
	new->inode = inode;
	new->fs = inode->fs;
	strcpy(new->name, name);
	return new;
}
//FIXME: Not implemented
int vfs_file_free(struct file *file)
{
	struct inode *inode = file->inode;
	kfree(file);
	//decrement reference count
	//if 0 take out of cache
	(void)inode;
	return 0;
}



//FIXME: Traversing mounts, '.' and '..', and non-directory inodes broken
//		 Some kind of cache would probably be a good idea
struct inode * vfs_namei(struct inode *dir, char *file)
{
	if(dir->flags & I_MOUNT && dir != root->inode)
	{
		PANIC("traversing mounts not yet implemented");
	} 

	if((dir->info.st_mode & S_IFDIR) == 0)
	{
		//TODO: return is not directory error
	//	PANIC("calling vfs_namei on a non-directory inode");
		return NULL;
	}
	
	return dir->fs->ops->namei(dir, file);
}

struct inode * vfs_pathsearch(struct file *dir, char *_path)
{
	char *saveptr, *tok, *filename, *path;
	struct inode *res = dir->inode;

	path = strdup(_path);

	filename = kcalloc(255, 1);;


//Remnant of idea for network based fs
//	if(memcmp(_path, "cfs://",6) == 0)
//	{
	//	return NULL;
//	}
	
	if(path[0] == '/')
		res = thread_current()->file_info->root;
	else
		res = thread_current()->file_info->cur;
	
	if((tok = (char *)strtok_r(path, "/", &saveptr)) == NULL)
		return NULL;

	if((res = vfs_namei(res, tok)) == NULL)
	{
		//FIXME	leaving this here for now
		//		but i've probably fixed the main bug
		//		causing this
		//printf("dir search or insert broken. path %s\n",path);
		
		return NULL;
	}

	strcpy(filename, tok);
	while((tok = (char *)strtok_r(NULL, "/", &saveptr)) != NULL)
	{
		//printf("TOk %s\n", tok);
		strcpy(filename, tok);
		if((res = vfs_namei(res, tok)) == NULL)
		{
		//	printf("file not found?\n");
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
	printf("Device %x\n",file->inode->info.st_rdev);

}

void vfs_mount_root(uint16_t dev, char *type)
{
	vfs_fs_t *fs;
	int ret = 0;
	dev = 0x301;
	if((fs = vfs_find_fs(type)) == NULL)
		goto error;
	ret++;
	
	if(fs->ops->read_sb == NULL)
		goto error;	
	ret++;
	
	if(fs->ops->read_sb(fs, dev) < 0)
		goto error;
	
	//FIXME: needs to be a list	
	open_inodes[0] = fs->superblock->root;
	
	//root->inode = fs->superblock->root;

	//FIXME: use vfs_new_file instead:
	root = vfs_file_new(fs->superblock->root, "/");
//	strcpy(root->name, "/");
//	root->dev = dev;
//	root->offset = 0;
//	root->fs = fs;

	thread_current()->file_info->cur = root->inode;
	thread_current()->file_info->root = root->inode;
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

//TODO: Copy the Linux module setup
//		Wherein we put everything in a 
//		null-terminated array of function 
//		pointers that we iterate over
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
extern void chardevs_init();
void vfs_init()
{
	//root = (struct file *)kcalloc(sizeof(struct file),1);

	printf("Initialzing VFS\n");
	chardevs_init();
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

struct file *vfs_open(char *path, int oflags, mode_t mode)
{
	(void)mode;
	struct inode *cur = thread_current()->file_info->cur;
	struct file *tmp = kcalloc(sizeof(*tmp), 1);
	tmp->inode = cur;	
	struct inode *lookup = vfs_pathsearch(tmp, path);

	if(lookup == NULL)
	{
		if((oflags & O_CREAT) != 0)
		{
	//		printf("need to create file\n");
		}
		return NULL;
	}
	kfree(tmp);
	struct file *new = vfs_file_new(lookup, path);
	return new;
}

int vfs_close(struct file *file)
{
	if(file == NULL)
		return -1;
	
	//vfs_file_free(file);

	return 0;
}

size_t vfs_read(struct file *file, void *buf, size_t nbyte)
{
	int ret = 0;
	if(file == NULL || buf == NULL)
		return -1;
	if((file->inode->info.st_mode & S_IFCHR) != 0){
		ret = char_device_read(file->inode->info.st_rdev, 
			buf, file->offset, nbyte);
	}else if((file->inode->info.st_mode & S_IFBLK) != 0){
		ret = block_device_readn(file->inode->info.st_rdev, 
			buf, 0, file->offset, nbyte);
	}else if((file->inode->info.st_mode & S_IFREG) != 0){
		//printf("READ OFF %x\n", file->offset);
		if(file->fs == NULL || file->fs->ops == NULL || file->fs->ops->read == NULL)
			return -1;
		ret = file->fs->ops->read(file->inode, buf,
			nbyte, file->offset);
	}
	file->offset += ret;
	return ret;

}

off_t vfs_write(struct file *file, void *buf, size_t nbyte)
{
	int ret = 0;
	if(nbyte == 0)
		return 0;
	if(file == NULL || buf == NULL)
		return -1;
	if((file->inode->info.st_mode & S_IFCHR) != 0){
		ret = char_device_write(file->inode->info.st_rdev, 
			buf, file->offset, nbyte);
	}else if((file->inode->info.st_mode & S_IFBLK) != 0){
//FIXME: This returns -1 so we don't fuxxor our disk image accidentally
	//	ret = block_device_readn(file->inode->rdev, 
	//		buf, 0, file->offset, nbyte);
		printf("Writing it currently disabled\n");
		return -1;
	}else if((file->inode->info.st_mode & S_IFREG) != 0){
		if(file->fs == NULL || file->fs->ops->write == NULL)
			return -1;
		ret = file->fs->ops->write(file->inode, buf,
			nbyte, file->offset);
	}
	file->offset += ret; 
	return ret;
}

int  vfs_ioctl(struct file *file, int request, char * args)
{
	int ret = 0;
	if(file == NULL)
		return -1;
	if((file->inode->info.st_mode & S_IFCHR) != 0){
		ret = char_device_ioctl(file->inode->info.st_rdev, 
			request, args);
	}else if((file->inode->info.st_mode & S_IFBLK) != 0){
	//	ret = block_device_readn(file->inode->rdev, 
	//		buf, 0, file->offset, nbyte);
		return -1;
	}else if((file->inode->info.st_mode & S_IFREG) != 0){
	//	if(file->fs == NULL || file->fs->ops->write == NULL)
	//		return -1;
	//	ret = file->fs->ops->write(file->inode, buf,
	//		nbyte, file->offset);
		return -1;
	}
	return ret;
}



//FIXME: Error catching
//		check if fd is open and return EBADF
//		check if file offset is negative and return EINVAL
//		check if offset overflows off_t and return EOVERFLOW
//		check if pipe and return ESPIPE
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
			//EINVAL?
			return 0;

	}

	return file->offset;
}
//FIXME: I'm not sure if the logic here checks out
//FIXME: Add various errors, most importantly
//		EBADF, EACCES, ENOTDIR and EFAULT
int vfs_chdir(const char *_path)
{
	struct file *file;
	int ret = -1;
	char *path = strdup(_path);
	file  = vfs_open(path, 0, 0);
	if(file != NULL)
	{
		//vfs_close(thread_current()->cur_dir);
		thread_current()->file_info->cur = file->inode;
		ret = 0;
	}
	kfree(path);
	return ret;
}

int vfs_stat(const char *path, struct stat *buf)
{
	(void)path;
	(void)buf;

	return 0;
}

int vfs_stat64(const char *path, struct stat64 *buf)
{
	struct inode *cur = thread_current()->file_info->cur;
	struct file *tmp = kcalloc(sizeof(*tmp), 1);
	tmp->inode = cur;
	struct inode *lookup = vfs_pathsearch(tmp, (char *)path);
	
	if(lookup == NULL)
		return -(ENOENT);
	kfree(tmp);	
	memcpy(buf, &lookup->info, sizeof(struct stat));
	
	return 0;
}/*
struct inode {
	uint32_t inode_num;
	uint16_t mode;
	uint16_t pad;
	uint32_t size;
	uint32_t atime;
	uint32_t ctime;
	uint32_t dtime;
	uint32_t mtime;
	uint32_t time;
	uint16_t gid;
	uint16_t uid;
	uint16_t links_count;
	uint16_t rdev;
	//if part of mount point,keep in cache
	uint32_t flags;
	void *storage;
	//may need parent
	vfs_fs_t *fs;	
};

struct stat64
{
	dev_t st_dev;
	int __st_dev_padding;
	long __st_ino_truncated;
	mode_t st_mode;
	nlink_t st_nlink;
	uid_t st_uid;
	gid_t st_gid;
	dev_t st_rdev;
	int __st_rdev_padding;
	off64_t st_size;
	blksize_t st_blksize;
	blkcnt64_t st_blocks;
	struct timespec st_atim;
	struct timespec st_mtim;
	struct timespec st_ctim;
	ino64_t st_ino;
};:*/

