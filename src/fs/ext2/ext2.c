#include <kernel/common.h>
#include <kernel/memory.h>
#include <kernel/fs/vfs.h>
#include <mm/liballoc.h>
#include "ext2fs_defs.h"
#include "ext2fs.h"
#include <stdio.h>
#include <string.h>

#define BLOCK_SIZE 512
#define EXT2_SB_SIZE 1024
#define EXT2_SB_BLOCK 2

//moved debug functions to seperate file
#include "ext2_debug.c"

//FIXME
vfs_ops_t ext2_ops = {
	ext2_read_inode,//vfs_read_inode_t read;
	ext2_write_inode,//vfs_write_inode_t write;
	ext2_read_superblock,//vfs_read_sb_t read_sb;
	ext2_namei//vfs_namei_t namei;
};



void ext2_inode_to_vfs(ext2_fs_t *fs,struct inode *vfs,ext2_inode_t *ext2,uint32_t inode)
{
	vfs->size = ext2->i_size;
	vfs->inode_num = inode;
	vfs->mode = ext2->i_mode;
	vfs->size = ext2->i_size;
	//fix time reading
	vfs->time = ext2->i_ctime;
	if(ext2->i_mode & S_IFCHR || ext2->i_mode & S_IFBLK)
	{
		vfs->rdev = ext2->i_block[0];
	}
	//TODO:if part of mount point,keep in cache
	vfs->flags = 0;
	vfs->storage = ext2;
	vfs->fs = (void *)fs;

}
size_t ext2_read_block(ext2_fs_t *fs, void *buf, int block)
{
	int block_size = fs->aux->block_size;
	return	read_block(fs->dev, buf, block, block_size);
}
size_t ext2_write_block(ext2_fs_t *fs, void *buf, int block)
{
	int block_size = fs->aux->block_size;
	return	write_block(fs->dev, buf, block, block_size);
}

int ext2_read_superblock(vfs_fs_t *fs, uint16_t dev)
{
	struct inode *root;
	int count;
	ext2_fs_t *ext2 = (ext2_fs_t *)fs;
	ext2_superblock_t *sb;
	ext2_aux_t *aux = fs->aux; 
	fs->dev = dev;

	fs->superblock->sb = kmalloc(sizeof(ext2_superblock_t));
	sb = fs->superblock->sb;
		
	count = read_block_at(fs->dev, sb, 1,1024, 0, 1024);

	if(count != 1024)
	{
		printf("reading superblock failed\n");
		goto fail;
	}
	
	if(sb->s_magic != EXT2_MAGIC)
	{
		printf("bad magic number\n");
		goto fail;
	}

	ext2->aux->gd_block = (sb->s_log_block_size == 0 ? 3 : 2);

	int num_groups = (sb->s_blocks_count - 1)/sb->s_blocks_per_group + 1;

	
	int size = sizeof(ext2_group_descriptor_t)*num_groups;
	
//	printf("EXT2 groups %i blocks %i gd_size %i\n", 
//		num_groups,sb->s_blocks_count, size);
	
	ext2->aux->gd_table = 
		(ext2_group_descriptor_t *)kmalloc(size);

	ext2->aux->block_size = 1024 << sb->s_log_block_size;
	
	ext2_read_block(ext2, aux->gd_table, aux->gd_block - 1);
	//now that we can read inodes, we cache the root inode(2)
	//in the vfs superblock struct
	if((root  = kcalloc(sizeof(*root), 1)) == NULL)
		goto fail;

	if((root = ext2_load_inode((ext2_fs_t *)fs, EXT2_ROOT_INO)) == NULL)
		goto fail;
	
	fs->superblock->root = root;
	
	return 0;
fail:
	if(root != NULL)
		kfree(root);	
	return -1;
}




//read_superblock
//read_inode
struct inode * ext2_load_inode(ext2_fs_t *fs, int inode)
{
	struct inode *read = kcalloc(sizeof(*read), 1);
	ext2_inode_t *ext2_ino = kcalloc(sizeof(*ext2_ino),1);
	ext2_inode_t *inode_block = kcalloc(sizeof(*ext2_ino),8);
	ext2_superblock_t *sb = fs->superblock->sb;
	int ind;
	int group = ((inode - 1)/sb->s_inodes_per_group);
	
	ext2_group_descriptor_t * gd = 
		&fs->aux->gd_table[group];
//	gd_print(*gd);
	int table_index = ((inode - 1 ) % sb->s_inodes_per_group) ;
//	printf("inode %i table_index = %i\n",inode, table_index);
	int block = ((table_index * 128) / 1024 )+ gd->bg_inode_table;
//	printf("BLOCK %i FUCK %i\n",block,((table_index * 128) % 1024) / 128);
	read_block(fs->dev, inode_block, block, 1024);
	ind = table_index % (1024 /128);
//	printf("IND %i\n",ind);
	kmemcpy(ext2_ino, &inode_block[ind], sizeof(*ext2_ino));
//	inode_print(*ext2_ino);
	kfree(inode_block);
	ext2_inode_to_vfs(fs, read,ext2_ino,inode);
	return read;
}
//read_dir

typedef uint32_t ino_t;
struct inode * ext2_namei(struct inode *dir, char *file)
{
	ext2_fs_t *fs = (ext2_fs_t *)dir->fs;
	struct inode *inode = ext2_load_inode((ext2_fs_t *)dir->fs, dir->inode_num);
	ext2_inode_t * in = (ext2_inode_t *)(inode->storage);
	int len = 0;	
	ext2_directory_t *ext2_dir = kmalloc(inode->size);
	int count =inode->size;
	void *fdir = ext2_dir;
	
//	inode_print(*in);
	ext2_read_block(fs, ext2_dir, in->i_block[0]);

//	ext2_read(&test, dir, 4096, 0);
	while(count)
	{
	//	print_dir_entry(ext2_dir);
	//	printf("%s %s %i\n", file, ext2_dir->name, ext2_dir->name_len);
		if(ext2_dir->name_len  == strlen(file) && !strncmp(ext2_dir->name,file,strlen(file)-1))
		{
			kfree(fdir);
		//	printf("loading inode %i\n",ext2_dir->inode);
			return ext2_load_inode(fs,ext2_dir->inode); 
		}
		if((len = ext2_dir[0].rec_len) == 0)
			break;	
		ext2_dir = (ext2_directory_t *)(uint32_t)((uint32_t)ext2_dir + (uint32_t)ext2_dir->rec_len);
		count -= len;

	}
	return 0;
}

#define EXT2_NDIR_BLOCKS 12
#define EXT2_DIR_BLOCKS 12

//FIXME: There are some magic numbers in here that should be gotten rid of
int byte_to_block(ext2_fs_t *fs, ext2_inode_t *inode, size_t offset)
{
	uint32_t block_size = fs->aux->block_size;
	off_t block_offset = offset / block_size;
	uint32_t *indirect = NULL;	
	uint32_t *dbl_indirect;
	int block = 0;
	uint32_t temp = 0;
	if(offset > inode->i_size)
		return -1;
	
	if(offset < EXT2_NDIR_BLOCKS*block_size)
	{
	//	printf("offset %i temp %i\n",offset, block_offset);
		block = inode->i_block[block_offset];

	}
	else if((unsigned)offset < ((EXT2_NDIR_BLOCKS + (block_size/4))* (block_size)))
	{
		indirect = kmalloc(block_size*sizeof(uint32_t));
		if(ext2_read_block(fs, indirect, inode->i_block[12]) != block_size)
			return -1;
		block_offset = (offset - EXT2_NDIR_BLOCKS*block_size)/block_size;	
	//	printf("offset %i temp %i\n",offset, block_offset);
		block = indirect[block_offset];

	}
	else if((unsigned)offset < ((EXT2_NDIR_BLOCKS + (block_size/4)) + (block_size/4)*(block_size/4)) * block_size)
	{
		dbl_indirect = kmalloc(block_size*sizeof(uint32_t));
		if(ext2_read_block(fs, dbl_indirect, inode->i_block[13]) != block_size)
			return -1;
	//	printf("check %u\n", offset - 274432);
		temp = offset - ((EXT2_NDIR_BLOCKS*block_size) + ((block_size / sizeof(uint32_t)*block_size)));
		block_offset = temp / (256*block_size);	
//	block_offset = (offset - (EXT2_NDIR_BLOCKS + (block_size / 4))*block_size)/block_size;	
		block = dbl_indirect[block_offset];
	//	printf("offset %i b_offset %i block %u temp %u temp2 %u\n",offset, block_offset, block, temp, temp2);
		if(block_offset > 256){
			printf("temp %u\n", temp);
			printf("value %u\n",((EXT2_NDIR_BLOCKS + (block_size/4))* (block_size)));	
			PANIC("SHIT'S FUCKED");
		}
	//	printf("offset %i b_offset %i block %u temp %u\n",offset, block_offset, block, temp);
		temp = temp - (block_offset*262144);//(block*((block_size/4) * block_size));
		indirect = kmalloc(block_size*sizeof(uint32_t));
		if(ext2_read_block(fs, indirect, block) != block_size)
			return -1;
		block_offset = (temp) / block_size;	
		block = indirect[block_offset];

	//	printf("offset %i b_offset %i block %u temp %u\n",offset, block_offset, block, temp);
	//	PANIC("Double indirect blocks in ext2.c not implemented yet");
		//return -1;
	}
	else
	{

		PANIC("Triple indirect blocks in ext2.c not implemented yet");

	}
	
	if(indirect != NULL)
		kfree(indirect);


	return block;
}

size_t ext2_read_inode(struct inode *inode,void *_buf, 
	size_t nbytes, off_t offset)
{
	ext2_fs_t *fs = (ext2_fs_t *)inode->fs;
	ext2_inode_t *ext2_ino = inode->storage;
	int block_size = fs->aux->block_size;

	uint8_t *buf = _buf;
	uint8_t *bounce = kmalloc(block_size);

	int remaining = nbytes;
	size_t count = 0;
	int block, block_ofs, cur_block_size, 
		to_end, till_end, cur_size;
	while(remaining > 0)
	{
		block = byte_to_block(fs, ext2_ino,offset);
		//printf("reading block %i %i\n",block, offset);
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

			kmemcpy(buf + count, bounce + block_ofs,cur_size);
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
size_t ext2_write_inode(struct inode *inode,void *_buf, 
	size_t nbytes, off_t offset)
{
	ext2_fs_t *fs = (ext2_fs_t *)inode->fs;
	ext2_inode_t *ext2_ino = inode->storage;
	int block_size = fs->aux->block_size;

	uint8_t *buf = _buf;
	uint8_t *bounce = kmalloc(block_size);

	int remaining = nbytes;
	size_t count = 0;
	int block, block_ofs, cur_block_size, 
		to_end, till_end, cur_size;
	if(nbytes + offset > ext2_ino->i_size)
	{
		printf("extensible files not yet supported\n");
		return 0;
	}
	while(remaining > 0)
	{
		block = byte_to_block(fs, ext2_ino,offset);
		block_ofs = offset % block_size;
		cur_block_size = block_size - block_ofs;
		to_end = ext2_ino->i_size - offset;
		till_end = (to_end < cur_block_size) ? to_end: cur_block_size;

		cur_size = remaining < till_end ? remaining : till_end;
		if(cur_size <= 0)
			break;
		
		if(block_ofs == 0 && cur_size == block_size)
		{
			if(ext2_write_block(fs, buf + count, block)  == 0)
			{
				count = -1;
				goto done;
			}
		}
		else
		{
			if(block_ofs > 0 || cur_size < cur_block_size)
			{
				ext2_read_block(fs, bounce, block);
			}
			kmemcpy(bounce + block_ofs,buf + count, cur_size);
			if(ext2_write_block(fs, bounce, block) == 0)
			{
				count = -1;
				goto done;
			}

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


int ext2_init()
{
	printf("Initializing EXT2 FS\n");
	ext2_fs_t * new = (ext2_fs_t *)vfs_alloc();
	new->aux = kmalloc(sizeof(ext2_aux_t));
	new->ops = &ext2_ops;
	//FIXME:should use strncpy
	strcpy(new->name, "ext2");
	return vfs_register_fs((vfs_fs_t *)new);	
}

