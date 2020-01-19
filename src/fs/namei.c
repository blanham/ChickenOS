#include <common.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <chicken/thread.h>
#include <fs/dentry.h>
#include <fs/ext2/ext2.h>
//#include <fs/partition.h>
#include <fs/vfs.h>
#include <mm/vm.h>
#include <mm/liballoc.h>

#define DENTRY_CACHE_LEN 	1024
#define DENTRY_CACHE_MASK	(DENTRY_CACHE_LEN-1)

#define MULTIPLIER 37 //or 31

// TODO:
//  * hash by full path, but insert the other directories we find on the way, but with a 0 reference count
//  * might be worth doing something on getdents too
//  * how do we handle system calls that get a path from an fd?

dentry_t *root_dentry;

struct dentry *dentry_cache_storage[DENTRY_CACHE_LEN] = {0};
struct dcache {
	// XXX: Needs a lock
	dentry_t **entries;
} dcache = { .entries = dentry_cache_storage};

//From Kernighan and Pike:
unsigned int string_hash(const char *str)
{
	uint32_t hash = 0;
	for (const char *p = str; *p; p++)
		hash = MULTIPLIER * hash + (unsigned)*p;
	return hash;
}

dentry_t *dcache_lookup(const char *path)
{
	uint32_t hash = string_hash(path);

	dentry_t *d = dcache.entries[hash & DENTRY_CACHE_MASK];
	while (d) {
		// XXX: Will this work?
		if (d->hash == hash)
			return d;

		d = d->hash_next;
	}

	return NULL;
}

dentry_t *dcache_lookup_by_hash(uint32_t hash)
{
	for (dentry_t *d = dcache.entries[hash & DENTRY_CACHE_MASK]; d != NULL; d = d->hash_next)
		if (d->hash != 0 && d->hash == hash)
			return d;
	return NULL;
}

dentry_t *dcache_insert(struct inode *insert, uint32_t hash)
{
	dentry_t *new = kcalloc(sizeof(*new), 1);
	new->inode = insert;
	new->hash = hash;
	new->refs++;

	dentry_t *entry = dcache.entries[hash & DENTRY_CACHE_MASK];
	if (entry == NULL) {
		dcache.entries[hash & DENTRY_CACHE_MASK] = new;
	} else {
		while (entry->hash_next)
			entry = entry->hash_next;
		entry->hash_next = new;
	}

	new->refs++;
	return new;
}


// TODO: Add permission checking, mount checking, etc
int dcache_pathsearch(const char *path, dentry_t **lookup, dentry_t *override_parent, dentry_t **return_parent, int loops, bool follow_link)
{
	if (loops > 3)
		return -ELOOP;

	*lookup = dcache_lookup(path);
	if (*lookup != NULL) { // Full path was already cached
		// XXX: IS this the best plce to handle negative dentrys?
		if ((*lookup)->inode == NULL) {
			*lookup = NULL;
			return -ENOENT;
		}
		
		(*lookup)->refs++;
		return 0;
	}

	const char *p = path;
	dentry_t *cur = thread_current()->file_info->cur;
	if (*p == '/') {
		cur = thread_current()->file_info->root;
		p++;
	} else if (override_parent != NULL) {
		cur = override_parent;
	}

	uint32_t hash = 0;
	const char *current_name = p;
	dentry_t *parent = cur;
	while (*current_name) {
//	for (const char *p = path; *p; p++) {
		
		if (*p != '/' && *p != '\0') {
			hash = hash * MULTIPLIER + *p++;
			continue;
		}

		size_t name_length = p - current_name;

		//printf("Current_name: %s\n", current_name);

		// XXX: predictably, this broke on names that started with '.', could be better
		if (name_length < 3 && current_name[0] == '.') {
			if(current_name[1] == '.') 
				cur = parent;
			goto skip_to_next_elem;
		}

		parent = cur; // keep this around for the next go around
		if (return_parent)
			*return_parent = parent; // Shouldn't we finish here? (or should we return the length....?)

		cur = dcache_lookup_by_hash(hash);
		if (cur == NULL) {
			struct inode *r = parent->inode->fs->ops->namei(parent->inode, current_name, name_length);

			//char * p2 = kcalloc(name_length +1, 1);
			//memcpy(p2, current_name, name_length);
			//printf("Name: %s\n", p2);


			cur = dcache_insert(r, hash);

			// TODO: I think it makes more sense to store the full path
			//		 and have this pointer point to the name in that string
			// calloc + memcpy, as path isn't null terminated
			size_t path_length = p - path;
			cur->path = kcalloc(path_length + 1, 1);
			memcpy(cur->path, path, path_length);
			cur->name = cur->path + path_length - name_length;
		}

		if (cur->inode == NULL) // NOTE: Treat dentry's with inode=NULL as negative dentries for now
			return -ENOENT;

		if (S_ISLNK(cur->inode->info.st_mode) && follow_link) {
			size_t size = cur->inode->info.st_size;

			char *link_path = kcalloc(size + 1, 1);
			if (link_path == NULL)
				return -ENOMEM;

			size_t rl_ret = cur->inode->fs->ops->readlink(cur->inode, link_path, size);
			if (rl_ret != size) {
				kfree(link_path);
				return -EIO; // FIXME: figure out correct error code for this
			}

			dentry_t *link_dentry = NULL;
			int rl_ps_ret = dcache_pathsearch(link_path, &link_dentry, parent, NULL, loops+1, true); // XXX: pass through or always false?
			if (link_dentry == NULL) {
				// I'm pretty sure this breaks for symlinks with multiple '..''s in the path
				serial_printf("SYMLINK ERROR: %i %s\n", rl_ps_ret, link_path);
				//PANIC("ERROR READING SYMLINK");

				kfree(link_path);
				return -ENOENT;
			}

			kfree(link_path);

			cur = link_dentry;
		}

skip_to_next_elem:
		while (*p == '/') p++;

		if (*p == '\0')
			break;

		if (!S_ISDIR(cur->inode->info.st_mode)) {
			return -ENOTDIR;
		}

		current_name = p;
	}

	*lookup = cur;
	if (cur)
		(*lookup)->refs++;

	return 0;
}

int vfs_pathsearch(const char *path, dentry_t **lookup, dentry_t **return_parent)
{
	return dcache_pathsearch(path, lookup, NULL, return_parent, 0, true);
}

int vfs_pathsearch_nofollow(const char *path, dentry_t **lookup, dentry_t **return_parent)
{
	return dcache_pathsearch(path, lookup, NULL, return_parent, 0, false);
}

void vfs_mount_root(uint16_t dev, char *type)
{
	vfs_fs_t *fs = vfs_find_fs(type);
	if(fs == NULL)
		goto error;

	if(fs->ops->read_sb == NULL)
		goto error;

    // FIXME: 
	struct inode *inode = kcalloc(sizeof(*inode), 1);
	struct device *device = get_device(S_IFBLK, dev);
	inode->read = device->read;
	// FIXME: this is wrong?
	inode->info.st_dev = dev;

	printf("Trying to mount\n");

	if(fs->ops->read_sb(inode, fs) < 0)
		goto error;

	root_dentry = kcalloc(sizeof(*root_dentry), 1);
	root_dentry->inode = fs->superblock->root;
	root_dentry->refs++;
	root_dentry->path = strdup("/");
	root_dentry->name = root_dentry->path;


	thread_t *cur = thread_current();
	cur->file_info->root = root_dentry;
	cur->file_info->cur = root_dentry;

	printf("Mounted %s fs @ dev %i:%i as root\n",type, MAJOR(dev),MINOR(dev));
	return;

error:
	PANIC("mounting root filesystem failed");
}

// TODO:
int sys_mount(const char *src, const char *mount, const char *fstype, uint64_t flags, const void *aux)
{
	if(verify_pointer(src, -1, VP_READ) | verify_pointer(mount, -1, VP_READ) | verify_pointer(fstype, -1, VP_READ))
		return -EFAULT;
	
	// XXX: This is nonsense, implement correctly later
	if (fstype)
		verify_pointer(aux, -1, VP_READ);

	vfs_fs_t *fs = vfs_find_fs(fstype);
	if (fs == NULL)
		return -ENODEV;

	serial_printf("Mount was called with: %s %s %s %llx %p\n", src, mount, fstype, flags, aux);

	// TODO: Implement
	return -ENOSYS;
}

// TODO:
int sys_umount2(const char *mount, int flags)
{
	if (verify_pointer(mount, -1, VP_READ))
		return -EFAULT;

	(void)flags;

	return -ENOSYS;
}

// TODO: Might it be worth pre-caching here?
// XXX: isn't this actually getdents64?
int sys_getdents(int fd, struct dirent *dirp, unsigned int count)
{
	if(verify_pointer(dirp, sizeof *dirp, VP_WRITE))
		return -EFAULT;

    struct file *fp = vfs_file_get(fd);
	if(fp == NULL)
		return -EBADFD;

	if (!S_ISDIR(fp->inode->info.st_mode))
		return -ENOTDIR;

	//TODO: Add this case
	//if inode was deleted after we opened it, but we still have a reference)
	// Apparently can trigger a corner case in GNU Make:
	// https://github.com/blanham/musl/commit/b9f7f2e8762922a1a24d41358164ebe9ae437e31
	//if(fp->inode->deleted)?
	//	return -ENOENT;

	return fp->inode->fs->ops->getdents(fp->inode, dirp, count, &fp->offset);
}