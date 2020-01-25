#ifndef C_OS_CHICKEN_MM_REGIONS_H
#define C_OS_CHICKEN_MM_REGIONS_H
#include <stdatomic.h>
#include <stdint.h>
#include <chicken/fs/dentry.h>

#define MM_PRESENT	0x0001
#define MM_WRITE	0x0002
#define MM_COW		0x0008

struct memregion {
	uintptr_t addr_start, addr_end;
	uintptr_t requested_start, requested_end;
	int pages;
	size_t len;
	atomic_int ref_count;

	// XXX: These should be a bitfield
	int present;
	int cow;
	int flags;
	int prot;

	// XXX: This should be a struct
	//struct file *file;
	struct inode *inode;
	dentry_t *dentry;
	off_t file_offset;
	size_t file_size;
	
	// NOTE: Maybe we should represent all mapped memory as files?
	// say, /dev/zero for zero pages
	// could greatly simplify code paths
	// literally use file offset and len of /dev/mem loaded as a struct inode?
	// would make swapping easier too....

	struct memregion *next;
	struct memregion *prev;
};


struct memregion *region_clone(struct memregion *original);
//int memregion_add(struct mm *mm, uintptr_t address, size_t len, int prot,
//						int flags, struct inode *inode, off_t offset, size_t file_len, void *data);
int memregion_fault(struct mm *mm, uintptr_t address, int prot);
int memregion_map_data(struct mm *mm, uintptr_t address, size_t len, int prot, int flags,
		void *data);
int memregion_map_file(struct mm *mm, uintptr_t address, size_t len, int prot,
		int flags, dentry_t *dentry, off_t offset, size_t size);
//struct memregion *memregion_new();
void mm_clear(struct mm *mm);


#endif