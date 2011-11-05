//#include "include/kernel/types.h"
#include <stdint.h>
#include <stdlib.h>
typedef struct ext2_superblock {
	uint32_t s_inodes_count; //Count of inodes in fs
	uint32_t s_blocks_count; //Count of blocks in fs
	uint32_t s_r_blocks_count; //Count of # of reserved blocks
	uint32_t s_free_blocks_count; //Count of # of free blocksw
	uint32_t s_free_inodes_count; //Count of # of free inodes
	uint32_t s_first_data_block; //First block that contains data
	uint32_t s_log_block_size; //Indicator of block size
	int32_t  s_log_frag_size; //Indicator of the size of fragments
	uint32_t s_blocks_per_group;//Count of # of blocks in each block group
	uint32_t s_frags_per_group;//Count of # of fragments in each block group
	uint32_t s_inodes_per_group;//Count of # of inodes in each blcok group
	uint32_t s_mtime; //time filesystem was last mounted
	uint32_t s_wtime;//time filesystem was last written to
	uint16_t s_mnt_count;//number of times the file system has been mounted
	int16_t  s_max_mnt_count;//number of times the file system can be mounted
	uint16_t s_magic;//EXT2 Magic number
	uint16_t s_state;//flags indicating current state of filesystem
	uint16_t s_errors;//flags indicating errors
	uint16_t s_pad;//padding
	uint32_t s_lastcheck;//time the fs was last checked
	uint32_t s_checkinterval;//maximum time between checks
	uint32_t s_creator_os;//indicator of which OS created 
	uint32_t s_rev_level;//EXT2 revision level
	uint32_t s_reserved[236];//padding to 1024 bytesOS
} ext2_superblock_t;

typedef struct ext2_group_descriptor {
	uint32_t bg_block_bitmap;//address of block containing the block bitmap for this group
	uint32_t bg_inode_bitmap;//address of block containing the inode bitmap for this group
	uint32_t bg_inode_table;//address of the block containing the inode table for this group
	uint16_t bg_free_blocks_count;//count of free blocks in group
	uint16_t bg_free_inodes_count;//count of free inodes in group
	uint16_t bg_used_dirs_count; //number of inodes in this group that are directories
	uint16_t bg_pad;
	uint32_t bg_reserved[3];
} ext2_group_descriptor_t;

struct ext2_inode {
	uint16_t i_mode;//File mode
	uint16_t i_uid;//Owner UID
	uint32_t i_size;//size in bytes
	uint32_t i_atime;//access time
	uint32_t i_ctime;//creation time
	uint32_t i_mtime;//modification time
	uint32_t i_dtime;//deletion time
	uint16_t i_gid;//Group ID
	uint16_t i_links_count;//links count
	uint32_t i_blocks;//blocks count
	uint32_t i_flags;//file flags
	uint32_t i_reserved1;
	uint32_t i_block[15];//pointers to blocks
	uint32_t i_version;
	uint32_t i_file_acl;//file ACL
	uint32_t i_dir_acl;//directory acl
	uint8_t i_faddr;//fragment address
	uint8_t i_fsize;//fragment size
	uint16_t i_pad1;
	uint32_t i_reserved2[3];
}__attribute__((packed));
typedef struct ext2_inode ext2_inode_t;

typedef struct ext2_directory {
	uint32_t inode;
	uint16_t rec_len;
	uint8_t name_len;
	uint8_t file_type;
	uint8_t name[255];
} ext2_directory_t;

typedef struct inode_cached {
	



} inode_cached_t;
#define S_IFMT 		0xF000 	format mask
#define S_IFSOCK 	0xA000 	socket
#define S_IFLNK 	0xC000 	symbolic link
#define S_IFREG 	0x8000 	regular file
#define S_IFBLK 	0x6000 	block device
#define S_IFDIR 	0x4000 	directory
#define S_IFCHR 	0x2000 	character device
#define S_IFIFO 	0x1000 	fifo
#define S_ISUID 	0x0800 	SUID
#define S_ISGID 	0x0400 	SGID
#define S_ISVTX 	0x0200 	sticky bit
#define S_IRWXU 	0x01C0 	user mask
#define S_IRUSR 	0x0100 	read
#define S_IWUSR 	0x0080 	write
#define S_IXUSR 	0x0040 	execute
#define S_IRWXG 	0x0038 	group mask
#define S_IRGRP 	0x0020 	read
#define S_IWGRP 	0x0010 	write
#define S_IXGRP 	0x0008 	execute
#define S_IRWXO 	0x0007 	other mask
#define S_IROTH 	0x0004 	read
#define S_IWOTH 	0x0002 	write
#define S_IXOTH 	0x0001 	execute
/* inodes are indexed starting at 1 */
#define INODE(x) (x-1)
#define BLOCK(x) (1024*x)

#define EXT2_BAD_INO 			1
#define EXT2_ROOT_INO			2
#define EXT2_ACL_IDX_INO		3
#define EXT2_ACL_DATA_INO		4
#define EXT2_BOOT_LOADER_INO	5
#define EXT2_UNDEL_DIR_INO		6

typedef struct ext2_filesystem {
	ext2_superblock_t *superblock;
	ext2_group_descriptor_t *gd_table; 
	uint32_t *block_bitmap;
	uint32_t *inode_bitmap;
	

} ext2_filesystem_t;

#include <stdio.h>
int bitmap_search(uint32_t *bitmap, uint32_t position, uint32_t length)
{
	uint32_t word = position / 32;
	uint32_t bit = position % 32;
	if((bitmap[word] & (1 << bit)) != 0)
	{
		return 1;
	}
	else
		return 0;

}


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
ext2_directory_t *open_root(uint8_t *fs, ext2_inode_t *inode_table)
{
	ext2_directory_t *root;
	uint32_t block = inode_table[INODE(EXT2_ROOT_INO)].i_block[0];
	printf("dir block %i\n", block);
	root = (ext2_directory_t *)&fs[BLOCK(block)];

	return root;

}
/*god damnit inode numbering starts with 1, not 0*/
void print_dir_entry(ext2_directory_t *dir)
{
	int i;
	for(i = 0; i < dir[0].name_len; i++)
		printf("%c",dir[0].name[i]);
	printf("\n");

	printf("%x\n\n",&dir);

}
void list_files(ext2_directory_t *dir)
{
	printf("dir size %i\n", dir[0].name_len);
	uint8_t *ptr = (uint8_t *)dir;	
	int i;	
	int count;

	print_dir_entry((ext2_directory_t *)ptr);

	count = dir[0].name_len;
	count %= count;
	int corr =0;
	if(count != 0)
		corr = 4;
	else
		corr = 0;
	count = 8 + (dir[0].name_len - count) + corr;
	printf("count %i\n",count);
	uint8_t *dir2 = dir;	
//	ext2_directory_t *dir2 = (dir + 16);
	dir = (ext2_directory_t *)(dir2 + count);
	printf("%x %x\n",&dir);
			
	print_dir_entry(dir);
//	ext2_directory_t *dir3 = (ext2_directory_t *)dir2;
//	printf("dir size %i\n", dir3[0].name_len);
//	for(i = 0; i < dir3[0].name_len; i++)
//		printf("%c ",dir3[0].name[i]);// + i));




}
int main(int argc, char **argv)
{
	FILE *fp;
	ext2_superblock_t *superblock = (ext2_superblock_t *)malloc(sizeof(ext2_superblock_t));
	if(argc != 2){
		printf("incorrect arguments!\n");
		return -1;
	}	
	if((fp = fopen(argv[1],"rb")) == NULL)
	{
		printf("file NULL!\n");
		return -1;
	}	
	fseek(fp, 1024, SEEK_SET);
	fread(superblock, 1024,1, fp);

	//if(superblock->s_magic 
	printf("sizei %x\n",1024 << superblock->s_log_block_size);
	uint32_t blockstart;
	
	if(superblock->s_log_block_size == 0)//1KB blocks
	{
		blockstart = 0x800;
	}else{
		blockstart = 0x1000;
	}

	int32_t groups = (1024/sizeof(ext2_group_descriptor_t));//superblock->s_blocks_count/superblock->s_blocks_per_group + 1; 
//	uint32_t total_groups = superblock->s_blocks_count / superblock->s_blocks_per_group;
//	uint32_t total_inodes = superblock->s_inodes_count / superblock->s_inodes_per_group;

//	printf("inodes %i groups %i\n",total_inodes,total_groups);


	ext2_group_descriptor_t *gd_table = malloc(sizeof(ext2_group_descriptor_t)*groups);
	fread(gd_table, 1024,1, fp);
	
	int i;
//	for(i = 0; i < groups; i++)
	gd_print(gd_table[0]);
	fseek(fp, 0, SEEK_SET);
	
	uint32_t fs_size = superblock->s_blocks_count * 1024;
	char *fs = malloc(fs_size);
	fread(fs, fs_size,1, fp);
	

	int inode_start = BLOCK(gd_table[0].bg_inode_table);
	printf("inode_start %x\n",inode_start);
	ext2_inode_t *inode_table = (ext2_inode_t *)&fs[inode_start];
	
//	for(i = 0; i < 16; i++)
		inode_print(inode_table[21]);
	
	ext2_directory_t *root = open_root(fs, inode_table);
	list_files(root);
/*
//		printf("\nsize %i\n",sizeof(ext2_inode_t));
	printf("groups %i\n",groups);
	printf("magic %x\n",superblock->s_magic);*/
	fclose(fp);
	return 0;
}

