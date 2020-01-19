#include <stdio.h>
#include <string.h>
#include "ext2fs_defs.h"

void ext2_superblock_dump(ext2_superblock_t *sb)
{
	printf("Inode count: %i block count: %i\n", sb->s_inodes_count, sb->s_blocks_count);
	printf("Block size %x\n", 1024 << sb->s_log_block_size);
}
/*
typedef struct ext2_superblock {
	uint32_t s_inodes_count;		// Count of inodes in fs
	uint32_t s_blocks_count;		// Count of blocks in fs
	uint32_t s_r_blocks_count;		// Count of # of reserved blocks
	uint32_t s_free_blocks_count;	// Count of # of free blocksw
	uint32_t s_free_inodes_count;	// Count of # of free inodes
	uint32_t s_first_data_block;	// First block that contains data
	uint32_t s_log_block_size;		// Indicator of block size
	int32_t  s_log_frag_size;		// Indicator of the size of fragments
	uint32_t s_blocks_per_group;	// Count of # of blocks in each block group
	uint32_t s_frags_per_group;		// Count of # of fragments in each block group
	uint32_t s_inodes_per_group;	// Count of # of inodes in each blcok group
	uint32_t s_mtime;				// Time filesystem was last mounted
	uint32_t s_wtime;				// Time filesystem was last written to
	uint16_t s_mnt_count;			// Number of times the file system has been mounted
	int16_t  s_max_mnt_count;		// Number of times the file system can be mounted
	uint16_t s_magic;				// EXT2 Magic number
	uint16_t s_state;				// Flags indicating current state of filesystem
	uint16_t s_errors;				// Flags indicating errors
	uint16_t s_pad;					// Padding
	uint32_t s_lastcheck;			// Time the fs was last checked
	uint32_t s_checkinterval;		// Maximum time between checks
	uint32_t s_creator_os;			// Indicator of which OS created 
	uint32_t s_rev_level;			// EXT2 revision level
	uint32_t s_reserved[236];		// Padding to 1024 bytesOS
} __attribute__((packed)) ext2_superblock_t;
*/
void ext2_inode_dump(ext2_inode_t *inode)
{
	if (inode == NULL) {
		serial_printf("BAD INODE\n");
	}

	serial_printf("\n=========inode========\n");
	serial_printf("inode mode %o\n",	inode->i_mode);
	serial_printf("inode size %i\n",	inode->i_size);
	serial_printf("inode atime %i\n",	inode->i_atime);
	serial_printf("inode mtime %i\n",	inode->i_mtime);
	serial_printf("inode ctime %i\n\n",inode->i_ctime);
	serial_printf("inode block0 %x\n",	inode->i_block[0]);
	serial_printf("inode block1 %i\n",	inode->i_block[1]);
	serial_printf("inode block2 %i\n\n",inode->i_block[2]);
	serial_printf("inode IND %i\n\n",inode->i_block[EXT2_INO_BLK_IND]);
	serial_printf("inode DBL %i\n\n",inode->i_block[EXT2_INO_BLK_DBL]);
	if(inode->i_size == 0 && inode->i_block[0] != 0)
	{
		uint32_t dev = inode->i_block[0];
		unsigned major = (dev & 0xfff00) >> 8;
		unsigned minor = (dev & 0xff) | ((dev >> 12) & 0xfff00);
		major = (dev >> 8 ) & 255;
		minor = dev & 255;
		printf("major %i minor %i\n",major,minor);
	}
}

void ext2_gd_dump(ext2_group_descriptor_t *gd)
{
	serial_printf("\n========EXT2=GD========\n");
	serial_printf("block bitmap\t%i\n", gd->bg_block_bitmap);
	serial_printf("inode bitmap\t%i\n", gd->bg_inode_bitmap);
	serial_printf("inode table\t%i\n", gd->bg_inode_table);
	serial_printf("free blocks\t%i\n", gd->bg_free_blocks_count);
	serial_printf("free inodes\t%i\n", gd->bg_free_inodes_count);
	serial_printf("dir inodes\t%i\n\n", gd->bg_used_dirs_count);
}

void ext2_dir_entry_dump(ext2_directory_t *dir)
{
	char buf[256];
	memset(buf, 0, 256);
	strncpy(buf, dir[0].name, dir[0].name_len);
	printf("inode: %i file_type %x rec_len %i name_lin %i name: %s\n", dir->inode, dir->file_type, dir->rec_len, dir->name_len, buf);
}