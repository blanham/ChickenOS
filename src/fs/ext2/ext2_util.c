#include <common.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <fs/vfs.h>
#include <mm/liballoc.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "ext2fs_defs.h"
#include "ext2fs.h"

void ext2_inode_to_vfs(ext2_fs_t *fs,struct inode *vfs,ext2_inode_t *ext2,uint32_t inode)
{
	struct stat64 *st = &vfs->info;
	st->st_dev = fs->dev->info.st_dev;
	st->st_ino = inode;
	st->st_mode = ext2->i_mode;
	st->st_nlink = ext2->i_links_count;
	st->st_uid = ext2->i_uid;
	st->st_gid = ext2->i_gid;

	if (ext2->i_mode & S_IFCHR || ext2->i_mode & S_IFBLK) {
		st->st_rdev = ext2->i_block[0];
	}

	st->st_size = ext2->i_size;
	st->st_atime = ext2->i_atime;
	st->st_ctime = ext2->i_ctime;
//	st->st_dtime = ext2->i_dtime;
	st->st_mtime = ext2->i_mtime;
	st->st_blksize = fs->aux->block_size;
	st->st_blocks = ext2->i_size / st->st_blksize;
	if(ext2->i_size % st->st_blksize)
		st->st_blocks++;


	vfs->flags = 0;
	vfs->storage = ext2;
	vfs->fs = (vfs_fs_t *)fs;
	vfs->read = fs->ops->read;
}


#include <mm/vm.h>
#define EXT2_NDIR_BLOCKS 12
#define EXT2_DIR_BLOCKS 12
int byte_to_block(ext2_fs_t *fs, ext2_inode_t *inode, off_t offset)
{
	ext2_superblock_t *sb = fs->superblock->sb;
	uint32_t block_size = fs->aux->block_size;
	uint32_t shift = 10 + sb->s_log_block_size;

	if (offset < EXT2_DIR_BLOCKS*block_size) {
		return inode->i_block[offset >> shift];
	}

	uint32_t *buf = kcalloc(block_size, 1);

	uint32_t indirect_block = inode->i_block[EXT2_INO_BLK_IND];
	uint32_t double_block   = inode->i_block[EXT2_INO_BLK_DBL];
	uint32_t triple_block   = inode->i_block[EXT2_INO_BLK_TPL];

	offset =  (offset >> shift) - EXT2_DIR_BLOCKS;

	uint16_t block_mask = (block_size - 1) >> 2;
	uint16_t block_shift = shift - 2;

	uint16_t ind = offset & block_mask;
	offset >>= block_shift;
	uint16_t dbl = offset & block_mask;
	offset >>= block_shift;
	uint16_t tpl = offset & block_mask;

	if (tpl) {
		if (ext2_read_block(fs, buf, triple_block) != block_size)
			return -1;
		double_block = buf[tpl - 1];
	}

	if (dbl) {
		if (ext2_read_block(fs, buf, double_block) != block_size)
			return -1;
		indirect_block = buf[dbl - 1];
	}


	if (ext2_read_block(fs, buf, indirect_block) != block_size)
		return -1;

	uint32_t block = buf[ind];

	kfree(buf);

	return block;
}

#define _GNU_SOURCE
#include <dirent.h>

uint8_t ext2_dir_ft_to_dirent(uint8_t file_type)
{
	switch (file_type & 0x7) {
		case EXT2_FT_UNKNOWN:
			return DT_UNKNOWN;
		case EXT2_FT_REG_FILE:
			return DT_REG;
		case EXT2_FT_DIR:
			return DT_DIR;
		case EXT2_FT_CHRDEV:		
			return DT_CHR;
		case EXT2_FT_BLKDEV:	
			return DT_BLK;
		case EXT2_FT_FIFO:
			return DT_FIFO;
		case EXT2_FT_SOCK:
			return DT_SOCK;
		case EXT2_FT_SYMLINK:
			return DT_LNK;
	}

	printf("TYPE %i\n", file_type);
	PANIC("BAD");
	return 0;
}