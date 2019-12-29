#include <common.h>
#include <errno.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <mm/liballoc.h>
#include <fs/vfs.h>
#include "ext2fs_defs.h"
#include "ext2fs.h"

extern size_t ext2_read_block(ext2_fs_t *fs, void *buf, int block);
extern int byte_to_block(ext2_fs_t *fs, ext2_inode_t *inode, size_t offset);

// TODO: Clean this up
// FIXME: Can entries cross blocks? 
int ext2_getdents(struct inode *dir, struct dirent *dirp, int count, off_t *off)
{
	ext2_fs_t *fs = (ext2_fs_t *)dir->fs;
	ext2_inode_t *ext2_ino = (ext2_inode_t *)dir->storage;
	ext2_directory_t *entry, *ext2_dir = kmalloc(fs->aux->block_size);
	int read = fs->aux->block_size, ret = 0;

	int block = byte_to_block(fs, ext2_ino, *off);
	ext2_read_block(fs, ext2_dir, block);
	entry = ext2_dir;

	if (*off >= ext2_ino->i_size) {
		goto end;
	}

	if ((offsetof(struct dirent, d_name) + entry->name_len + 1) > (unsigned)count) {
		ret = -EINVAL;
		goto end;
	}

	while (count > ret && read > 0) {
		memcpy(dirp->d_name, entry->name, entry->name_len);
		*(dirp->d_name + entry->name_len + 1) = '\0';

		dirp->d_reclen = offsetof(struct dirent, d_name) + entry->name_len + 1;
		dirp->d_ino = entry->inode;
		dirp->d_off = *off;
		dirp->d_type = 1 << entry->file_type;

		count -= dirp->d_reclen;
		ret += dirp->d_reclen;
		dirp = (void *)dirp + dirp->d_reclen;

		*off += entry->rec_len;
		read -= entry->rec_len;
		entry = (void *)entry + entry->rec_len;
	}

end:
	kfree(ext2_dir);
	return ret;
}