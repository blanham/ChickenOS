#include <stdint.h>
#include "vfs.h"
/* inodes are indexed starting at 1 */
#define INODE(x) (x-1)
#define BLOCK(x) (1024*(x))
#define UNUSED(X) X = X

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

#define EXT2_BAD_INO 			1
#define EXT2_ROOT_INO			2
#define EXT2_ACL_IDX_INO		3
#define EXT2_ACL_DATA_INO		4
#define EXT2_BOOT_LOADER_INO	5
#define EXT2_UNDEL_DIR_INO		6

typedef struct ext2_aux {
	uint32_t size;
	ext2_group_descriptor_t *gd_table; 
	uint32_t *block_bitmap;
	uint32_t *inode_bitmap;
} ext2_aux_t;

typedef struct ext2_fs {
	ext2_superblock_t *superblock;
	uint16_t type;
	ext2_aux_t *aux;
	vfs_ops_t *ops;
} ext2_fs_t;

