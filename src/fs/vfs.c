/*	ChickenOS - fs/vfs.c - virtual file system
 *
 */
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chicken/common.h>
#include <chicken/fs/ext2/ext2.h>
#include <chicken/fs/icache.h>
#include <chicken/fs/vfs.h>
#include <chicken/mm/vm.h>
#include <chicken/thread.h>

//FIXME: Probably shouldn't be a table
vfs_fs_t * filesystems[10];

struct file *vfs_file_new(dentry_t *dentry)
{
	struct file *new = kcalloc(sizeof(*new),1);
	new->dentry = dentry;
	new->inode = dentry->inode;
	new->fs = dentry->inode->fs;
	return new;
}

struct file *vfs_file_get(int fd)
{
	thread_t *cur = thread_current();

	struct file *ret = NULL;
	if(fd >= 0)// && fd <= cur->file_info->files_open) // XXX: This should probably check a new variable named max_fd
		ret = cur->files->files[fd];

	return ret;
}

void vfs_file_free(struct file *file UNUSED)
{
	//struct inode *inode = file->inode;
	//kfree(file);
	//icache_put(inode);
}

vfs_fs_t *vfs_alloc(const char *name, vfs_ops_t *ops)
{
	vfs_fs_t *new = kcalloc(sizeof(*new),1);
	new->superblock = kcalloc(sizeof(vfs_sb_t),1);
	new->ops = ops;
	new->name = strdup(name);
	return new;
}

// TODO: Improve this
int vfs_register_fs(vfs_fs_t *fs)
{
	//return error if name blank
	if(fs->name[0] == '\0')
		return -1;

	for (int i = 0; i < 10; i++) {
		if (filesystems[i] == NULL) {
			filesystems[i] = fs;
			return 1;
		}
	}

	return 0;
}

//FIXME: use linked list or hash
vfs_fs_t * vfs_find_fs(const char *type)
{
	for (int i = 0; i < 10; i++) {
		if (!strncmp(type, filesystems[i]->name, 10)) {
			return filesystems[i];
		}
	}

	return NULL;
}

size_t vfs_read2(struct inode *inode, uint8_t *buf, size_t count, off_t offset)
{
	size_t blocksize = inode->info.st_blksize;
	off_t blockmask = (blocksize - 1);
	uint8_t *block_buf = kcalloc(1024, 1);

	off_t leader = offset & blockmask;
	if (leader) {
		uint64_t block = inode->fs->ops->bmap(inode, offset);
		(void)block;

		memcpy(buf, &block_buf[leader], blocksize - leader);

		count -= blocksize - leader;
	}

	while (count > blocksize) {
		uint64_t block = inode->fs->ops->bmap(inode, offset);
		uint8_t *block_buf = kcalloc(1024, 1);
		(void)block;

		memcpy(buf, &block_buf, blocksize);

		count -= blocksize;
	}

	if (count) {
		uint64_t block = inode->fs->ops->bmap(inode, offset);
		uint8_t *block_buf = kcalloc(1024, 1);
		(void)block;

		memcpy(buf, &block_buf, count);

		count = 0;
	}

	kfree(block_buf);

	return 0;
}

extern void chardevs_init();
void vfs_init()
{
	printf("Initialzing VFS\n");
	chardevs_init();
	icache_init();
	if(ext2_init() < 0)
		PANIC("failed to init ext2 fs\n");
}