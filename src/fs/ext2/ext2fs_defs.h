#ifndef C_OS_FS_EXT2FS_DEFS_H
#define C_OS_FS_EXT2FS_DEFS_H
#include <fs/vfs.h>

// inodes are indexed starting at 1
#define INODE(x) (x-1)
#define BLOCK(x) (1024*(x))
#define EXT2_MAGIC 0xef53

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

typedef struct ext2_group_descriptor {
	uint32_t bg_block_bitmap;		// Address of block containing the block bitmap for this group
	uint32_t bg_inode_bitmap;		// Address of block containing the inode bitmap for this group
	uint32_t bg_inode_table;		// Address of the block containing the inode table for this group
	uint16_t bg_free_blocks_count;	// Count of free blocks in group
	uint16_t bg_free_inodes_count;	// Count of free inodes in group
	uint16_t bg_used_dirs_count;	// Number of inodes in this group that are directories
	uint16_t bg_pad;
	uint32_t bg_reserved[3];
} __attribute__((packed)) ext2_group_descriptor_t;


struct ext2_inode {
	uint16_t i_mode;		// File mode
	uint16_t i_uid;			// Owner UID
	uint32_t i_size;		// Size in bytes
	uint32_t i_atime;		// Access time
	uint32_t i_ctime;		// Creation time
	uint32_t i_mtime;		// Modification time
	uint32_t i_dtime;		// Deletion time
	uint16_t i_gid;			// Group ID
	uint16_t i_links_count;	// Link count
	uint32_t i_blocks;		// Block count
	uint32_t i_flags;		// File flags
	uint32_t i_reserved1;
	uint32_t i_block[15];	// Block pointers/other data
	uint32_t i_version;
	uint32_t i_file_acl;	// File ACL
	uint32_t i_dir_acl;		// Directory acl
	uint8_t i_faddr;		// Fragment address
	uint8_t i_fsize;		// Fragment size
	uint16_t i_pad1;
	uint32_t i_reserved2[3];
}__attribute__((packed));
typedef struct ext2_inode ext2_inode_t;

typedef struct ext2_directory {
	uint32_t inode;
	uint16_t rec_len;
	uint8_t name_len;
	uint8_t file_type;
	char name[255];
} ext2_directory_t;

#define EXT2_BAD_INO 			1
#define EXT2_ROOT_INO			2
#define EXT2_ACL_IDX_INO		3
#define EXT2_ACL_DATA_INO		4
#define EXT2_BOOT_LOADER_INO	5
#define EXT2_UNDEL_DIR_INO		6

typedef struct ext2_aux {
	uint32_t size;
	uint32_t gd_block;
	uint32_t block_size;
	ext2_group_descriptor_t *gd_table; 
	// XXX: I wanted to cache these, but instead with the block cache these should be replace with block #'s
	//may cache these
	uint32_t *block_bitmap;
	uint32_t *inode_bitmap;
} ext2_aux_t;
/*
struct vfs_fs {
	vfs_sb_t *superblock;
	char name[10];
	void *fs;
	vfs_ops_t *ops;
};
*/
typedef struct ext2_fs {
	vfs_sb_t *superblock;
	char name[10];
	ext2_aux_t *aux;
	// If we make this an inode we can call read/write directly
	uint16_t dev;
	vfs_ops_t *ops;
} ext2_fs_t;

int ext2_read_superblock(vfs_fs_t *fs, uint16_t dev);
struct inode * ext2_load_inode(ext2_fs_t *fs, int ino);
struct inode * ext2_namei(struct inode *dir, char *file);
int ext2_getdents(struct inode *dir, struct dirent *dirp, int count, off_t *off);
size_t ext2_read_inode(struct inode *inode, void *_buf, size_t length, off_t offset);
size_t ext2_write_inode(struct inode *inode, void *_buf, size_t length, off_t offset);

#endif
