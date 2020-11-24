#ifndef C_OS_MM_VM_H
#define C_OS_MM_VM_H
#include <stdint.h>
#include <stddef.h>
#include <stdatomic.h>
#include <sys/mman.h>
#include <chicken/boot.h>
#include <chicken/common.h>
#include <chicken/interrupt.h>
#include <chicken/fs/dentry.h>
#include <chicken/fs/vfs.h>
#include <chicken/mm/paging.h>
#include <chicken/lib/rbtree.h>

typedef uintptr_t phys_addr_t;
typedef uintptr_t virt_addr_t;

#define PAGE_SIZE 4096
#define PAGE_MASK 0xFFFFF000
#define PAGE_OFFSET (~PAGE_MASK)
#define PAGE_SHIFT 12

#define PAGE_COUNT(len) (((len) / PAGE_SIZE) + ((len) & PAGE_OFFSET ? 1 : 0))

#define PD_SIZE   4096

#define PAGE_VIOLATION 0x01
#define PAGE_WRITE	   0x02
#define PAGE_USER	   0x04
#define PAGE_ERR_MASK (PAGE_VIOLATION|PAGE_WRITE|PAGE_USER)

#define PHYS_BASE 0xC0000000
#define MMAP_BASE 0x5000000
#define HEAP_BASE 0x09000000
#define V2P(p) ((void *)((char *)(p) - PHYS_BASE))
#define P2V(p) ((void *)((char *)(p) + PHYS_BASE))

struct mm {
	void *pd;
	rbtree_t *tree;
	struct memregion *regions;
	//Tree here
	void * brk;
	void *mmap_base;
	uintptr_t sbrk;
	int ref;
};

typedef struct memregion address_region_t;

typedef struct {
	uint32_t *page_directory;
	rbtree_t *tree;
	address_region_t *heap;
	address_region_t *stack;
	address_region_t *mmap;


	atomic_int ref;
} address_space_t;

/* mm/vm.c */
void vm_init(struct kernel_boot_info *info);
void vm_page_fault(registers_t *regs, uintptr_t addr, int flags);
struct mm *mm_alloc();
struct mm *mm_clone(struct mm *old);
void mm_init(struct mm *mm);

// XXX: Hmm, should this just use the MM_* constants from region.h?
// NOTE: This is in region.c, but left here because of the huge mess of includes it would affect
#define VP_READ  0
#define VP_WRITE 1
int verify_pointer(const void *ptr, size_t len, int rw); // XXX: For now, rw = 1 means write

/* mm/palloc.c */
void	palloc_init(struct kernel_boot_info *info);
void *	pallocn(uint32_t count);
void *	palloc();
void *	palloc_len(size_t len);
void	palloc_free(void *addr);
int		pallocn_free(void *addr, int pages);

/* mm/mm_ops.c */
int		sys_mprotect(void *addr, size_t len, int prot);
void *	sys_mmap2(void *addr, size_t length, int prot, int flags, int fd, off_t pgoffset);
int		sys_munmap(void *addr, size_t length);

#endif