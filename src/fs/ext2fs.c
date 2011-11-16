#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "ext2fs_defs.h"
//#include "ext2fs.h"
/* inodes are indexed starting at 1 */
#define INODE(x) (x-1)
#define BLOCK(x) (1024*(x))
#define UNUSED(X) X = X
typedef struct ext2_filesystem {
	ext2_superblock_t *superblock;
	char *fs;
	uint32_t size;
//	ext2_group_descriptor_t *gd_table; 
//	uint32_t *block_bitmap;
//	uint32_t *inode_bitmap;
	

} ext2_filesystem_t;

//char *fs; //just have a pointer to global buffer for now, will also work for ram disk




void gd_print(ext2_group_descriptor_t gdt)
{
	printf("\n=========GDT========\n");
	printf("block bitmap\t%i\n", gdt.bg_block_bitmap);
	printf("inode bitmap\t%i\n", gdt.bg_inode_bitmap);
	printf("inode table\t%i\n", gdt.bg_inode_table);
	printf("free blocks\t%i\n", gdt.bg_free_blocks_count);
	printf("free inodes\t%i\n", gdt.bg_free_inodes_count);
	printf("dir inodes\t%i\n\n", gdt.bg_used_dirs_count);
}

void inode_print(ext2_inode_t inode)
{

	printf("\n=========inode========\n");
	printf("inode mode %i\n",inode.i_mode);
	printf("inode size %i\n",inode.i_size);
	printf("inode atime %i\n\n",inode.i_atime);
	printf("inode ctime %i\n\n",inode.i_ctime);

}



/*god damnit inode numbering starts with 1, not 0*/
void print_dir_entry(ext2_directory_t *dir)
{
	char buf[256];
	memset(buf, 0, 256);
	memcpy(buf, dir[0].name, dir[0].name_len);	
//	for(int i = 0; i < dir[0].name_len; i++)
//		printf("%c",dir[0].name[i]);
	printf("%-20s \t\tinode %i\n", buf, dir[0].inode);
}

int ext2_read_raw(ext2_filesystem_t *ext2, void *buf, uint32_t offset, uint32_t size)
{
	uint8_t *tmp = buf;
	for(uint32_t i = 0; i < size; i++)
	{
		*tmp++ = ext2->fs[offset +i];
	}
	return 0;
}

int ext2_read_blockon(ext2_filesystem_t *ext2, uint32_t block, void *buf, size_t offset, size_t length)
{
	uint32_t blk_size = 1024;
	
	ext2_superblock_t *sb = ext2->superblock;
	blk_size <<= sb->s_log_block_size;
	
	ext2_read_raw(ext2, buf, blk_size*(block - 1) + offset, length); 
	return 0;
}
int ext2_read_block(ext2_filesystem_t *ext2, uint32_t block, void *buf)
{
	uint32_t blk_size = 1024 << ext2->superblock->s_log_block_size;

	return ext2_read_blockon(ext2, block, buf, 0, blk_size);
}
/* returns block where inode is at */
/*uint32_t ext2_read_group(ext2_filesystem_t *ext2, uint32_t inode_num)
{





}*/
int ext2_read_inode(ext2_filesystem_t *ext2, uint32_t inode_num, ext2_inode_t *inode)
{
	ext2_group_descriptor_t *gd = malloc(sizeof(ext2_group_descriptor_t)*1024);
	uint32_t gd_block = 0;
	/* if 1024 block size, gd table is in block 3 else block 2 */
	if(ext2->superblock->s_log_block_size == 0)
	{
		gd_block = 3;
	}else {
		gd_block = 2;
	}	

	/* need to check if inode is allocated or not */	
	uint32_t group = (inode_num / ext2->superblock->s_inodes_per_group);
	printf("gd %u\n", gd_block);
	printf("group %u\n", group);
		
	ext2_read_block(ext2, gd_block, gd);
	gd_print(gd[0]);

	uint32_t inode_blk = gd->bg_inode_table;

	UNUSED(inode);

	int inode_start = BLOCK(gd[0].bg_inode_table);
	printf("%i %i\n", inode_start, inode_blk);
//	printf("inode_start %x\n",inode_start);

	ext2_inode_t *inode_table = (ext2_inode_t *)malloc(ext2->superblock->s_inodes_per_group * sizeof(ext2_inode_t));//   //&ext2->fs[inode_start];
	ext2_read_block(ext2, inode_blk, inode_table);
	ext2_read_raw(ext2, inode_table, BLOCK(inode_blk), ext2->superblock->s_inodes_per_group*sizeof(ext2_inode_t));

//	for(i = 0; i < 16; i++)
		inode_print(inode_table[15]);

	printf("%i\n", gd->bg_free_blocks_count);
	free(gd);
	return -1;

}

int lookup_file(ext2_directory_t *dir, uint32_t len, char *filename)
{
	int ret = -1;
	while(len > 0)
	{
		if(dir[0].inode == 0)
		{
			len -= dir[0].rec_len;
			dir = (ext2_directory_t *)((uint8_t *)dir + dir[0].rec_len);
			
			continue;
		}		

		//print_dir_entry((ext2_directory_t *)dir);
		ret = strncmp((const char *)filename, (const char *)dir[0].name, dir[0].name_len);
		if(ret == 0)
			return dir[0].inode;
		len -= dir[0].rec_len;
		dir = (ext2_directory_t *)((uint8_t *)dir + dir[0].rec_len);
	}
	return 0;
}

void list_files(ext2_directory_t *dir, uint32_t len)
{
	while(len > 0)
	{
		if(dir[0].inode == 0)
		{
			len -= dir[0].rec_len;
			dir = (ext2_directory_t *)((uint8_t *)dir + dir[0].rec_len);
			
			continue;
		}		

		print_dir_entry((ext2_directory_t *)dir);

		len -= dir[0].rec_len;
		dir = (ext2_directory_t *)((uint8_t *)dir + dir[0].rec_len);
	}

}
ext2_directory_t *open_root(uint8_t *fs, ext2_inode_t *inode_table)
{
	ext2_directory_t *root;
	uint32_t block = inode_table[INODE(EXT2_ROOT_INO)].i_block[0];
//	printf("dir block %i\n", block);
	root = (ext2_directory_t *)&fs[BLOCK(block)];

	return root;

}
ext2_directory_t *open_dir(uint8_t *fs, ext2_inode_t *inode_table, uint32_t inode)
{
	ext2_directory_t *root;
	uint32_t block = inode_table[INODE(inode)].i_block[0];
//	printf("dir block %i\n", block);
	root = (ext2_directory_t *)&fs[BLOCK(block)];

	return root;

}
ext2_directory_t *ext2_open_dir()
{
	
	ext2_directory_t *dir = malloc(sizeof(ext2_directory_t));
//	uint32_t block = inode_table[INODE(inode)].i_block[0];
//	printf("dir block %i\n", block);
//	root = (ext2_directory_t *)&fs[BLOCK(block)];

	return dir;

}

ext2_filesystem_t *ext2_load(char *filename)
{
	FILE *fp;
 	filename = filename;		
	ext2_filesystem_t *ext2 = (ext2_filesystem_t *)malloc(sizeof(ext2_filesystem_t));
	ext2->superblock = (ext2_superblock_t *)malloc(sizeof(ext2_superblock_t));
	if((fp = fopen(filename,"rb")) == NULL)
	{
		printf("file NULL!\n");
		return NULL;
	}	
	fseek(fp, 1024, SEEK_SET);
	printf("sizeopf %lui\n", sizeof(ext2_superblock_t));
	fread(ext2->superblock, sizeof(ext2_superblock_t),1, fp);
	fseek(fp, 0, SEEK_SET);
	if(ext2->superblock->s_magic != 0xef53)
	{
		printf("bad magic number!\n");
		return NULL;
	}	

	ext2->size = ext2->superblock->s_blocks_count * 1024;
	ext2->fs = malloc(ext2->size);
	fread(ext2->fs, ext2->size,1, fp);
	fclose(fp);
	return ext2;


}

void lookup(ext2_filesystem_t *ext2, char *name)
{
	char *save_ptr;
	char *directory;
	char *a;
	char *b;
	a = b = calloc(256, 1);
	strcpy(b, name);
	while((directory = strtok_r(b, "/", &save_ptr)) != NULL)
	{
		b = NULL;
		printf("%s\n", directory);
		

	}

	free(a);


}

int ext2(char *filename)
{
	ext2_filesystem_t *ext2 = ext2_load(filename);

	ext2_inode_t *inode_test = malloc(sizeof(ext2_inode_t));
	ext2_read_inode(ext2, 16, inode_test);


	uint32_t groups = (1024/sizeof(ext2_group_descriptor_t));//superblock->s_blocks_count/superblock->s_blocks_per_group + 1; 


	ext2_group_descriptor_t *gd_table = malloc(sizeof(ext2_group_descriptor_t)*groups);
	ext2_read_raw(ext2, gd_table, 2048,sizeof(ext2_group_descriptor_t)*groups);
	ext2_read_block(ext2, 3, gd_table);



	lookup(ext2, "/boot/map");

//	int i;
//	for(i = 0; i < groups; i++)
	gd_print(gd_table[0]);
	
	
	int inode_start = BLOCK(gd_table[0].bg_inode_table);
	printf("inode_start %i\n", gd_table[0].bg_inode_table);
	printf("inode_start %x\n",inode_start);
	ext2_inode_t *inode_table = (ext2_inode_t *)malloc(ext2->superblock->s_inodes_per_group * sizeof(ext2_inode_t));
	ext2_read_raw(ext2, inode_table, inode_start, ext2->superblock->s_inodes_per_group*sizeof(ext2_inode_t));
//	for(i = 0; i < 16; i++)
		inode_print(inode_table[15]);
	
	printf("sdafasdfasdsdaffas");
//	return 0;
	ext2_directory_t *root = open_root((uint8_t *)ext2->fs, inode_table);
	list_files(root, inode_table[1].i_size);
	printf("\n\n");
	ext2_directory_t *dir = open_dir((uint8_t *)ext2->fs, inode_table, 12);
	list_files(dir, inode_table[11].i_size);

int test =	lookup_file(dir, inode_table[11].i_size, "map");
	if(test)
		printf("awesome\n");

//	*/
/*
//		printf("\nsize %i\n",sizeof(ext2_inode_t));
	printf("groups %i\n",groups);
	printf("magic %x\n",superblock->s_magic);*/
//	fclose(fp);

	return 0;

}
