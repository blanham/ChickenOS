#include "vfs.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#define UNUSED(X) X = X

struct fat16_bootblock {
	uint8_t jmp[3];
	uint8_t desc[8];
	uint16_t bytes_per_block;
	uint8_t  blocks_per_alloc;
	uint16_t reserved_blocks;
	uint8_t	 num_fats;
	uint16_t root_directories;
	uint16_t disk_blocks;//0 if overflow
	uint8_t  media_desc;
	uint16_t fat_blocks;
	uint16_t blocks_per_track;
	uint16_t heads;
	uint32_t hidden_blocks;
	uint32_t sectors_per_partition;
	
	uint8_t logical_drive;
	uint8_t unused;
	uint8_t ext_sig;
	uint32_t volume_serial;
	uint8_t volume_label[11];
	uint8_t  file_system[8];
	uint8_t  boot[448];
	uint16_t signature; 
} __attribute__ ((packed));
struct fat_directory_entry {
	uint8_t filename[11];
	uint8_t attrib;
	uint8_t reserved;
	uint8_t tenths;
	uint16_t creation_time;
	uint16_t creation_date;
	uint16_t accesse_date;
	uint16_t cluster_high;
	uint16_t modified_time;
	uint16_t modified_date;
	uint16_t cluster_low;
	uint32_t file_size;
} __attribute__ ((packed));
typedef struct fat_directory_entry directory_t;
typedef struct fat16_bootblock fat16_bootblock_t;
typedef struct fat16_bootblock fat12_bootblock_t;

struct fat32_bootblock {
	uint8_t jmp[3];
	uint8_t desc[8];
	uint16_t bytes_per_block;
	uint8_t  blocks_per_alloc;
	uint16_t reserved_blocks;
	uint8_t	 num_fats;
	uint16_t root_directories;
	uint16_t disk_blocks;//0 if overflow
	uint8_t  media_desc;
	uint16_t fat_blocks;
	uint16_t blocks_per_track;
	uint16_t heads;
	uint32_t hidden_blocks;
	uint32_t sectors_per_partition;
	uint32_t sectors_per_fat;
	uint16_t flags;
	uint16_t fatver;
	uint32_t root_cluster;
	uint16_t fs_info_sector;
	uint16_t backup_boot_sector;
	uint8_t reserved[12];
	uint8_t logical_drive;
	uint8_t unused;
	uint8_t ext_sig;
	uint32_t volume_serial;
	uint8_t volume_label[11];
	uint8_t  file_system[8];
	uint8_t  boot[420];
	uint16_t signature; 
} __attribute__ ((packed));

typedef struct fat32_bootblock fat32_bootblock_t;

struct partition_entry {
	uint8_t bootflag;
	uint8_t chs_begin[3];
	uint8_t typecode;
	uint8_t chs_end[3];
	uint32_t lba_begin;
	uint32_t num_sectors;
} __attribute__ ((packed));
typedef struct partition_entry partition_t;

struct mbr_struct {
	uint8_t bootcode[446];
	partition_t parts[4];
	uint16_t signature;
} __attribute__ ((packed));

typedef struct mbr_struct mbr_t;
typedef struct fat_filesystem {
	//fat_bootblock_t *bootblock;
	int type;


} fat_filesystem_t;
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

/*struct fat_directory_entry {
	uint8_t filename[11];
	uint8_t attributes;
	uint8_t reserved;
	uint8_t tenths;
	uint16_t creation_time;
	uint16_t creation_date;
	uint16_t accesse_date;
	uint16_t cluster_high;
	uint16_t modified_time;
	uint16_t modified_date;
	uint16_t cluster_low;
	uint32_t file_size;
} __attribute__ ((packed));*/




void print_dir_entry(directory_t dir)
{
	print_chars(dir.filename, 11,false);
	printf("  cluster %.2x%.2x \tsize %8x attrib %x\n",
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




int main2(int argc, char **argv)
{
	UNUSED(argc);
	UNUSED(argv);
	if(argc != 2)
		return -1;

	FILE *fp = fopen(argv[1], "rb");
	if(fp == NULL)
		return 1;

	fread(&bootblock, sizeof(fat12_bootblock_t),1,fp);
	//print_mbr(&MBR);
	//printf("VOL s\n", (char *)&bootblock.volume_label);
	print_chars((char *)bootblock.volume_label,11, true);
	printf("%X %lu\n", bootblock.signature, sizeof(fat12_bootblock_t));
	printf("Cluster size %i sectors per fat%x\n", bootblock.bytes_per_block, bootblock.fat_blocks*bootblock.bytes_per_block*bootblock.num_fats);	
	directory_t root[100];
//	fseek(fp, 0x100, SEEK_CUR);
	fseek(fp, bootblock.fat_blocks*bootblock.bytes_per_block*bootblock.num_fats, SEEK_CUR);
	fread(&root, sizeof(directory_t), 100, fp);
	ls_dir((directory_t *)&root);
	directory_t test[100];
//	fseek(fp,512, SEEK_CUR);
	fseek(fp, bootblock.fat_blocks*bootblock.bytes_per_block*bootblock.num_dats + 512, SEEK_SET);
	fseek(fp, 0x200, SEEK_CUR);	
	fread(&test, sizeof(directory_t), 100, fp);
	ls_dir(&test);
//	int i = 0;
//	for(;i < 244; i++)
//		print_dir_entry(root[i]);	
	fclose(fp);
//

}
