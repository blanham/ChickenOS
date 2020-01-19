#ifndef C_OS_FS_DENTRY_H
#define C_OS_FS_DENTRY_H
#include <fs/vfs.h>
#include <dirent.h>

#define DCACHE_PRESENT	0x01
#define DCACHE_MOUNT	0x10

// How do we keep track of the full path?
typedef struct dentry {
	char *name; // this point to the name with path, don't free() it
	char *path;
	uint32_t hash;

	uint8_t flags;

	dev_t dev;	
	ino_t ino;
	dev_t parent_dev;
	ino_t parent_ino;

	struct inode *inode, *parent;

	int refs; // Reference count

	struct dentry *hash_prev, *hash_next;
	struct dentry *lru_next, *lru_prev;
} dentry_t;

int dcache_pathsearch(const char *path, dentry_t **lookup, dentry_t *override_parent, dentry_t **return_parent, int loops, bool follow_link);
int vfs_pathsearch(const char *path, dentry_t **lookup, dentry_t **return_parent);
int vfs_pathsearch_nofollow(const char *path, dentry_t **lookup, dentry_t **return_parent);
void vfs_mount_root(uint16_t dev, char *type);
#endif