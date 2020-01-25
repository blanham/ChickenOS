#include <stdlib.h>
#include <string.h>
#include <chicken/common.h>
#include <chicken/fs/ext2/ext2.h>
#include <chicken/fs/vfs.h>
#include "ext2fs_defs.h"

//#define EXT2_DEBUG
#ifdef EXT2_DEBUG
#define DPRINTF(fmt, ...)\
	do { serial_printf("EXT2: "fmt,##__VA_ARGS__); } while (0)
#else
#define DPRINTF(fmt, ...) do { } while (0)
#endif 

#define DIV_ROUND_UP(x,y) (((x) + (y) + 1) / (y))

size_t ext2_read_block(ext2_fs_t *fs, void *buf, off_t block)
{
	return fs->dev->read(fs->dev, buf, fs->aux->block_size, block * fs->aux->block_size);
}

size_t ext2_write_block(ext2_fs_t *fs UNUSED, void *buf UNUSED, off_t block UNUSED)
{
	PANIC("NO EXT2 WRITES");
	return	-1;
}

int ext2_read_superblock(struct inode *device, vfs_fs_t *fs)
{
	ext2_superblock_t *sb = kcalloc(sizeof(ext2_superblock_t), 1);
	ext2_aux_t *ext2 = NULL;
	inode_t *root_directory = NULL;

	if (device == NULL || device->read == NULL) {
		printf("Device %p\n", device);
		PANIC("Invalid call of ext2_readsb with null(s)\n");
	}

	size_t count = device->read(device, (void *)sb, 1024, 1024);
	if (count != 1024) {
		DPRINTF("EXT2: Invalid count %i while trying to load superblock\n", count);
		goto fail;
	}

	if (sb->s_magic != EXT2_MAGIC) {
		DPRINTF("EXT2: bad magic number: %X\n", sb->s_magic);
		ext2_superblock_dump(sb);
		goto fail;
	}

	fs->dev = device;
	fs->superblock->sb = sb;

	ext2 = fs->aux = kcalloc(sizeof(*ext2), 1);
	ext2->block_size = 1024 << sb->s_log_block_size;
	ext2->gd_block = (sb->s_log_block_size == 0 ? 2 : 1);

	// XXX: This caches the group descriptor block, or at least tries to.
	//      Remove? Or just implement it properly with multiple block support?
	int num_groups = DIV_ROUND_UP(sb->s_blocks_count, sb->s_blocks_per_group);
	ext2->gd_table = kcalloc(sizeof(ext2_group_descriptor_t), num_groups);

	ext2_read_block((ext2_fs_t *)fs, ext2->gd_table, ext2->gd_block);
#ifdef EXT2_DEBUG
	ext2_gd_dump(ext2->gd_table);
#endif

	root_directory = ext2_load_inode((ext2_fs_t *)fs, EXT2_ROOT_INO);
	if (root_directory == NULL)
		goto fail;

#ifdef EXT2_DEBUG
	ext2_inode_dump(root_directory->storage);
#endif

	fs->superblock->root = root_directory;

	return 0;
fail:
	free(root_directory);
	free(sb);
	free(ext2->gd_table);
	free(ext2);
	return -1;
}

size_t ext2_read_block_section(ext2_fs_t *fs, void *buf, int block, int offset, int size)
{
	static uint8_t *bounce;
	if(bounce == NULL)
		bounce = kcalloc(fs->aux->block_size, 1);

	size_t ret = ext2_read_block(fs, bounce, block);
	if (ret != fs->aux->block_size)
		return -1;

	memcpy(buf, &bounce[offset], size);

	return size;
}

struct inode * ext2_load_inode(ext2_fs_t *fs, int inode)
{
	ext2_superblock_t *sb = fs->superblock->sb;

	div_t indexes = div(INODE(inode), sb->s_inodes_per_group);
	int group = indexes.quot;
	int table_index = indexes.rem;

	ext2_group_descriptor_t *gd = &fs->aux->gd_table[group];

#ifdef EXT2_DEBUG
	//ext2_gd_dump(gd);
#endif

	int block = gd->bg_inode_table + ((table_index * sb->s_inode_size) / fs->aux->block_size);
	int pos_in_block = (table_index % (fs->aux->block_size / sb->s_inode_size)) * sb->s_inode_size;

	//DPRINTF("Inode %i index %i Group %i Block %i pos %i\n", inode, table_index, group, block, pos_in_block);
	//DPRINTF("Inode size %x\n", sb->s_inode_size);

	ext2_inode_t *ext2_ino = kcalloc(sb->s_inode_size, 1);
	if (ext2_read_block_section(fs, ext2_ino, block, pos_in_block, sb->s_inode_size) != sb->s_inode_size) {
		DPRINTF("Error reading inode %i on device x:x\n", inode);
		ext2_inode_dump(ext2_ino);
		kfree(ext2_ino);
		return NULL;
	}

	// XXX: Move this allocation to ext2_inode_to_vfs?
	struct inode *read = kcalloc(sizeof(*read), 1);
	ext2_inode_to_vfs(fs, read, ext2_ino, inode);

	return read;
}

// XXX: This is a dirty hack lol
//struct inode * ext2_namei2(struct inode *dir, const char *file, size_t length)
//{
//	char *name = kcalloc(length + 1, 1);
//
//	memcpy(name, file, length);
//
//	//DPRINTF("NAMEI HACK %lli %s %s len: %i\n", dir->info.st_ino, name, file, length);
//
//	struct inode *ret = ext2_namei(dir, name);
//
//	ret->read = ret->fs->ops->read;
//	return ret;
//}

struct inode *ext2_namei(struct inode *dir, const char *file, size_t length)
{
	uint32_t block_size = dir->info.st_blksize;
	ext2_directory_t *ext2_dir = kcalloc(block_size, 1);
	struct inode *ret = NULL;

	ext2_inode_t *ext2_ino = dir->storage;
	bool dump = false;
	if (ext2_ino->i_flags & EXT2_INDEX_FL) {
		dump = true;
	}

	// EXT2 directory entries do not cross blocks
	// so we can implement this fairly simply:
	for (off_t i = 0; ret == NULL && i < dir->info.st_size; i += block_size) {
		if (dir->read(dir, (void *)ext2_dir, block_size, i) != block_size)
			goto finished;

		for (ext2_directory_t *p = ext2_dir; (void *)p < (void *)ext2_dir + block_size; p = (void *)p + p->rec_len) {
			if(p->rec_len == 0)
				goto finished;

			//if (dump)
			//	ext2_dir_entry_dump(p);

			if (length != p->name_len)
				continue;

			//if (!strncmp(p->name, file, p->name_len)) {
			if (!memcmp(p->name, file, length)) {
				ret = ext2_load_inode((ext2_fs_t *)dir->fs, p->inode);
				//serial_printf("Found an inode\n");
				goto finished; // XXX: FIXME: !!!!!
			}
		}	
	}

finished:
	kfree(ext2_dir);
	return ret;
}

//FIXME: better error detection, need to combine read and write and use
//		 the existing functions as wrappers to a functiont that has a
//		 rw boolean
size_t ext2_read_inode(struct inode *inode,	uint8_t *buf, size_t nbytes, off_t offset)
{
	ext2_fs_t *fs = (ext2_fs_t *)inode->fs;
	ext2_inode_t *ext2_ino = inode->storage;
	int block_size = fs->aux->block_size;

	uint8_t *bounce = kmalloc(block_size);

	int remaining = nbytes;
	size_t count = 0;
	int block, block_ofs, cur_block_size,
		to_end, till_end, cur_size;
	while(remaining > 0)
	{
		block = byte_to_block(fs, ext2_ino, offset);
		//if (offset == 0x115000)
			//DPRINTF("reading block %i %i\n",block, offset);
		block_ofs = offset % block_size;
		cur_block_size = block_size - block_ofs;
		to_end = ext2_ino->i_size - offset;
		till_end = (to_end < cur_block_size) ? to_end: cur_block_size;

		cur_size = remaining < till_end ? remaining : till_end;
		if(cur_size <= 0)
			break;

		if(block_ofs == 0 && cur_size == block_size)
		{
			if(ext2_read_block(fs, buf + count, block)  == 0)
			{
				count = -1;
				goto done;
			}
		}
		else
		{
			if(ext2_read_block(fs, bounce, block) == 0)
			{
				count = -1;
				goto done;
			}

			memcpy(buf + count, bounce + block_ofs,cur_size);
		}

		count  += cur_size;
		offset += cur_size;
		remaining   -= cur_size;
	}

done:
	if(bounce != NULL)
		kfree(bounce);

	return count;
}

size_t ext2_write_inode(struct inode *inode UNUSED, uint8_t *buf UNUSED, size_t nbytes UNUSED, off_t offset UNUSED)
{
	PANIC("No writes!\n");
	return -1;
}

vfs_ops_t ext2_ops = {
	.read = ext2_read_inode,
	.write = ext2_write_inode,
	.read_sb = ext2_read_superblock,
	.namei = ext2_namei,
	.readlink = ext2_readlink,
	.getdents = ext2_getdents,
	.bmap = ext2_bmap
};

int ext2_init()
{
	DPRINTF("Initializing EXT2 FS\n");
	return vfs_register_fs(vfs_alloc("ext2", &ext2_ops));
}