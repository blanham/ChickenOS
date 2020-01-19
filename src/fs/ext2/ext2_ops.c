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

// TODO: Clean this up
// FIXME: Can entries cross blocks? They can't! :)
int ext2_getdents(struct inode *dir, struct dirent *dirp, int count, off_t *off)
{
	ext2_fs_t *fs = (ext2_fs_t *)dir->fs;
	ext2_inode_t *ext2_ino = (ext2_inode_t *)dir->storage;

	if (*off >= ext2_ino->i_size) {
		return 0;
	}

	size_t block_size = fs->aux->block_size;
	//off_t offset = *off;


	// EXT2 directory entries do not cross blocks
	// so we can implement this fairly simply:
	ext2_directory_t *ext2_dir = kcalloc(block_size, 1);
	off_t i;
	size_t count2 = count;
	size_t count3 = 0;
	for (i = *off & ~(block_size-1); i < dir->info.st_size; i += block_size) {
		if (dir->read(dir, (void *)ext2_dir, block_size, i) != block_size)
			printf("hmm\n");//goto finished;

		off_t inner_offset = *off & (block_size-1);
		for (ext2_directory_t *p = (void *)ext2_dir + inner_offset; (void *)p < (void *)ext2_dir + block_size; p = (void *)p + p->rec_len) {
			if(p->rec_len == 0)
				break;

			size_t next_rec_len = (offsetof(struct dirent, d_name) + p->name_len + 1 + 7) & ~0x7;
			if (next_rec_len > count2) {
				goto finished;
			}

			dirp->d_ino = p->inode;
			dirp->d_off = next_rec_len;
			dirp->d_reclen = next_rec_len;
			dirp->d_type = ext2_dir_ft_to_dirent(p->file_type);
			memcpy(&dirp->d_name, p->name, p->name_len);
			dirp->d_name[p->name_len] = '\0';

			//ext2_dir_entry_dump(p);
			//printf("d_ino=%lli, d_off=%lli d_reclen=%i d_type=%x, d_name=\"%s\"\n", dirp->d_ino, dirp->d_off, dirp->d_reclen, dirp->d_type, dirp->d_name);
			//printf("Value: %i count3 %i\n", next_rec_len, count3);

			count2 -= dirp->d_reclen;
			count3 += dirp->d_reclen;

			*off += p->rec_len;
			dirp = (void *)dirp + dirp->d_reclen;
		}	
	}


finished:
	kfree(ext2_dir);
	return count3;
}

int ext2_getdents_old(struct inode *dir, struct dirent *dirp, int count, off_t *off)
{
	ext2_fs_t *fs = (ext2_fs_t *)dir->fs;
	ext2_inode_t *ext2_ino = (ext2_inode_t *)dir->storage;
	ext2_directory_t *entry, *ext2_dir = kmalloc(fs->aux->block_size);
	int read = fs->aux->block_size, ret = 0;

	int block = byte_to_block(fs, ext2_ino, *off);
	ext2_read_block(fs, ext2_dir, block);
	entry = ext2_dir;

	if((offsetof(struct dirent, d_name) + entry->name_len + 1) > (unsigned)count)
	{
		ret = -EINVAL;
		goto end;
	}
	while(count > ret && read > 0)
	{
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

uint64_t ext2_bmap(struct inode *inode, off_t offset)
{
	ext2_fs_t *fs = (ext2_fs_t *)inode->fs;
	ext2_inode_t *ext2_ino = inode->storage;
	return byte_to_block(fs, ext2_ino, offset);
}

// TODO: review, but I think this is right
ssize_t ext2_readlink(struct inode *inode, char *path, size_t length)
{
	ext2_inode_t *ext2_ino = inode->storage;

	off_t size = inode->info.st_size;
	if (size <= 60) {
		if (size > length)
			size = length;
		memcpy(path, ext2_ino->symlink, size);
		return size;
	}

	if (size > length)
		size = length;

	return inode->read(inode, (uint8_t *)path, size, 0);
}