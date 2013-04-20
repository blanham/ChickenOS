//#include "vfs.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "fat.h"
#define UNUSED(X) X = X

typedef struct vfs_fs vfs_fs_t;
struct inode;
typedef size_t (*vfs_read_inode_t) (struct inode *, void *_buf, size_t length, off_t offset);
typedef size_t (*vfs_write_inode_t) (struct inode *, void *_buf, size_t length, off_t offset);
typedef struct inode * (*vfs_namei_t) (struct inode *dir, char *path);
typedef int (*vfs_read_sb_t) (vfs_fs_t *fs, uint16_t dev);
typedef int (*vfs_mount_t) (uint16_t dev);
typedef struct vfs_superblock_ops {
	vfs_read_inode_t read;
	vfs_write_inode_t write;
	//vfs_creat_t creat;
	//vfs_mkdir_t mkdir;
	vfs_read_sb_t read_sb;
	vfs_namei_t namei;
} vfs_ops_t;

typedef struct vfs_sb {
	void *sb;
	struct inode *root;

} vfs_sb_t;
struct vfs_fs {
	vfs_sb_t *superblock;
	char name[10];
	void *aux;
	uint16_t dev;
	vfs_ops_t *ops;
};


struct inode {
	uint32_t inode_num;
	uint16_t mode;
	uint16_t pad;
	uint32_t size;
	uint32_t time;
	uint16_t rdev;
	//if part of mount point,keep in cache
	uint32_t flags;
	void *storage;
	//may need parent
	vfs_fs_t *fs;	
};

void print_mbr(mbr_t *mbr)
{
	int i;
	for(i = 0; i < 4; i++)
	{
		printf("Boot %x\n",mbr->parts[i].bootflag);
		printf("Type %x\n",mbr->parts[i].typecode);
		printf("LBA %x\n",mbr->parts[i].lba_begin);
	}
}
fat12_bootblock_t bootblock;
mbr_t MBR;
void print_chars(char* array, int size, bool newline)
{
	int i;
	for(i = 0; i < size; i++)
		putchar(array[i]);
	if(newline)
		putchar('\n');

}




void print_dir_entry(directory_t dir)
{
//	print_chars(dir.filename, 11,false);
	printf("%11s",dir.filename);
	printf("  cluster %.4x%.4x \tsize %8u attrib %x\n",
		dir.cluster_high,dir.cluster_low, dir.file_size,dir.attrib);
}
void ls_dir(directory_t *dir)
{
	int i = 0;
	for(;i < 256; i++)
	{	
		if(((dir[i].attrib)== 0xf) || (dir[i].filename[0] == 0xe5))
			continue;
		if(dir[i].filename[0] == 0)
			break;
		print_dir_entry(dir[i]);



	}
}
struct buffer_element {
	uint16_t device;
	uint16_t size;
	uint8_t *buf;
	//struct list_head list;
};


//uint8_t * fat_read_block(




int main(int argc, char **argv)
{
	if(argc != 2)
		return -1;

	FILE *fp = fopen(argv[1], "rb");
	if(fp == NULL)
		return -1;

	fread(&bootblock, sizeof(fat12_bootblock_t),1,fp);
	//print_mbr(&MBR);
	//printf("VOL s\n", (char *)&bootblock.volume_label);
	print_chars((char *)bootblock.volume_label,11, true);
	printf("%X %lu\n", bootblock.signature, sizeof(fat12_bootblock_t));
	printf("Cluster size %i sectors per fat%x reserved blocks: %u\n", bootblock.bytes_per_block, bootblock.fat_blocks*bootblock.bytes_per_block*bootblock.num_fats, bootblock.reserved_blocks);	
//	directory_t root[100];
	directory_t *root;
	root = calloc(sizeof(directory_t),100);
//	fseek(fp, 0x100, SEEK_CUR);
	fseek(fp, bootblock.fat_blocks*bootblock.bytes_per_block*bootblock.num_fats, SEEK_CUR);
	fread(root, sizeof(directory_t), 100, fp);
	ls_dir(root);
	printf("ass mcballs\n");
	directory_t *test;
	test = calloc(sizeof(directory_t),100);

//	fseek(fp,512, SEEK_CUR);
	fseek(fp, bootblock.fat_blocks*bootblock.bytes_per_block*bootblock.num_fats + 512, SEEK_SET);
	fseek(fp, 0x200, SEEK_CUR);	
	fread(test, sizeof(directory_t), 100, fp);
	ls_dir(test);
//	int i = 0;
//	for(;i < 244; i++)
//		print_dir_entry(root[i]);	
	free(root);
	free(test);
	fclose(fp);
//

}
