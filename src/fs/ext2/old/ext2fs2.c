#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include "../vfs.h"
//#include "initrd.h"
#include "ext2fs_defs.h"
typedef int vfs_fs_t;

struct vfs_virtual_sb {




};
void gd_print2(ext2_group_descriptor_t gdt)
{
	printf("\n=========GDT========\n");
	printf("block bitmap\t%i\n", gdt.bg_block_bitmap);
	printf("inode bitmap\t%i\n", gdt.bg_inode_bitmap);
	printf("inode table\t%i\n", gdt.bg_inode_table);
	printf("free blocks\t%i\n", gdt.bg_free_blocks_count);
	printf("free inodes\t%i\n", gdt.bg_free_inodes_count);
	printf("dir inodes\t%i\n\n", gdt.bg_used_dirs_count);
}

void inode_print2(ext2_inode_t inode)
{

	printf("\n=========inode========\n");
	printf("inode mode %i\n",inode.i_mode);
	printf("inode size %i\n",inode.i_size);
	printf("inode atime %i\n\n",inode.i_atime);
	printf("inode ctime %i\n\n",inode.i_ctime);

}



#define INODE2GROUP(x) (x / ext2->superblock->s_inodes_per_group)

int ext2_read_inode(ext2_fs_t *ext2, uint32_t inode_num, ext2_inode_t *inode)
{
//	int inode_block = ext2->aux->gd_table[INODE2GROUP(inode_num)];
	inode = inode;
	 
//	uint32_t inode_blk = gd->bg_inode_table;
	ext2 = ext2;
	inode_num = inode_num;
	UNUSED(inode);
	return -1;
/*
	int inode_start = BLOCK(gd[0].bg_inode_table);
	printf("%i %i\n", inode_start, inode_blk);
//	printf("inode_start %x\n",inode_start);

	ext2_inode_t *inode_table = (ext2_inode_t *)malloc(ext2->superblock->s_inodes_per_group * sizeof(ext2_inode_t));//   //&ext2->fs[inode_start];
//	ext2_read_block(ext2, inode_blk, inode_table);
//	ext2_read_raw(ext2, inode_table, BLOCK(inode_blk), ext2->superblock->s_inodes_per_group*sizeof(ext2_inode_t));

//	for(i = 0; i < 16; i++)
		inode_print(inode_table[15]);

	printf("%i\n", gd->bg_free_blocks_count);
	free(gd);*/
//	return -1;

}


ext2_fs_t *ext2_new()
{
	ext2_fs_t *ext2 = malloc(sizeof(ext2_fs_t));
	ext2->aux = malloc(sizeof(ext2_aux_t));
	ext2->superblock = malloc(sizeof(ext2_superblock_t));
	ext2->aux->gd_table = malloc(sizeof(ext2_group_descriptor_t)*1024);

	return ext2;
}



int ext2_init(ext2_fs_t *fs, char *filename)
{
	ext2_fs_t *ext2 = ext2_new();	
	int fd = open(filename, 0);

	
	lseek(fd,512*2,SEEK_SET);
	read(fd, ext2->superblock, sizeof(ext2_superblock_t));	
	
	if(ext2->superblock->s_magic != 0xef53)
	{
		printf("bad magic number!\n");
		return -1;
	} else {
		printf("Magic good\n");
	}	
	
	uint32_t gd_block = 0;
	/* if 1024 block size, gd table is in block 3 else block 2 */
	if(ext2->superblock->s_log_block_size == 0)
	{
		gd_block = 3;
	}else {
		gd_block = 2;
	}	

	/* need to check if inode is allocated or not */	
	uint32_t group = INODE2GROUP(2);
	printf("gd %u\n", gd_block);
	printf("group %u\n", group);
	lseek(fd,BLOCK(INODE(gd_block)), SEEK_SET);
	read(fd,ext2->aux->gd_table, sizeof(ext2_group_descriptor_t)*1024);
	//initrd_read(ext2->aux->gd_table, 2+2, (sizeof(ext2_group_descriptor_t)*1024)/512);
	gd_print2(ext2->aux->gd_table[0]);


	return 0;
}

int main(int argc, char **argv)
{

	if(argc != 2)
		return -1;

	ext2_fs_t fs;
	ext2_init(&fs, argv[1]);



}
