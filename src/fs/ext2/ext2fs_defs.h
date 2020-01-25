#ifndef C_OS_FS_EXT2FS_DEFS_H
#define C_OS_FS_EXT2FS_DEFS_H

// inodes are indexed starting at 1
#define INODE(x) (x-1)
#define BLOCK(x) (1024*(x))
#define EXT2_MAGIC 0xef53

// TODO: There might be a few more fields that should be added
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
	uint16_t s_def_resuid;			// Default uid for reserved blocks (default 0)
	uint16_t s_def_resgid;			// Default gid for reserved blocks (default 0)
	uint32_t s_fist_ino;			// First available standard ino
	uint32_t s_inode_size;			// Inode size
	uint32_t s_reserved[233];		// Padding to 1024 bytesOS
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

#define EXT2_INDEX_FL 0x1000

typedef struct ext2_inode {
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
	union {
		uint32_t i_block[15];	// Block pointers/other data
		char	 symlink[30];	// Fast symlink data
	};
	uint32_t i_version;
	uint32_t i_file_acl;	// File ACL
	uint32_t i_dir_acl;		// Directory acl
	uint8_t i_faddr;		// Fragment address (fragments are not implemented anywhere though)
	uint8_t i_fsize;		// Fragment size
	uint16_t i_pad1;
	uint32_t i_reserved2[3];
} __attribute__((packed)) ext2_inode_t;

typedef struct ext2_directory {
	uint32_t inode;
	uint16_t rec_len;
	uint8_t name_len;
	uint8_t file_type;
	char name[255];
} __attribute__((packed)) ext2_directory_t;

#define EXT2_FT_UNKNOWN		0
#define EXT2_FT_REG_FILE	1
#define EXT2_FT_DIR			2
#define EXT2_FT_CHRDEV		3
#define EXT2_FT_BLKDEV		4
#define EXT2_FT_FIFO		5
#define EXT2_FT_SOCK		6
#define EXT2_FT_SYMLINK		7

#define EXT2_BAD_INO 			1
#define EXT2_ROOT_INO			2
#define EXT2_ACL_IDX_INO		3
#define EXT2_ACL_DATA_INO		4
#define EXT2_BOOT_LOADER_INO	5
#define EXT2_UNDEL_DIR_INO		6
//#define BLOCK_SIZE			512
#define EXT2_SB_SIZE		1024
#define EXT2_SB_BLOCK		2
#define EXT2_INO_BLK_DEV	0
#define EXT2_INO_BLK_IND	12
#define EXT2_INO_BLK_DBL	13
#define EXT2_INO_BLK_TPL	14

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
	const char *name;
	ext2_aux_t *aux;
	// If we make this an inode we can call read/write directly
	struct inode *dev;
	vfs_ops_t *ops;
} ext2_fs_t;

/* ext2.c */
size_t			ext2_read_block(ext2_fs_t *fs, void *buf, blkcnt_t block);
int				ext2_read_superblock(struct inode *inode, vfs_fs_t *fs);
struct inode *	ext2_load_inode(ext2_fs_t *fs, int ino);
struct inode *	ext2_namei_old(struct inode *dir, const char *file);
struct inode *	ext2_namei(struct inode *dir, const char *file, size_t length);
ssize_t			ext2_readlink(struct inode *inode, char *path, size_t length);
int				ext2_getdents(struct inode *dir, struct dirent *dirp, int count, off_t *off);
size_t			ext2_read_inode(struct inode *inode, uint8_t *buf, size_t length, off_t offset);
size_t			ext2_write_inode(struct inode *inode, uint8_t *buf, size_t length, off_t offset);

/* ext2_debug.c */
void ext2_superblock_dump(ext2_superblock_t *sb);
void ext2_gd_dump(ext2_group_descriptor_t *gd);
void ext2_inode_dump(ext2_inode_t *inode);
void ext2_dir_entry_dump(ext2_directory_t *dir);

/* ext2_ops.c */
uint64_t ext2_bmap(struct inode *, off_t);

/* ext2_util.c */

void ext2_inode_to_vfs(ext2_fs_t *fs,struct inode *vfs,ext2_inode_t *ext2,uint32_t inode);
int byte_to_block(ext2_fs_t *fs, ext2_inode_t *inode, off_t offset);
uint8_t ext2_dir_ft_to_dirent(uint8_t file_type);

#endif