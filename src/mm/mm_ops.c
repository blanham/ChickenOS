/*	ChickenOS - mm/mm_ops.c
 */
#include <errno.h>
#include <stdio.h>
#include <chicken/common.h>
#include <chicken/mm/vm.h>
#include <chicken/mm/regions.h>
#include <chicken/thread.h>

//Put sys_mmap, and any other system calls that affect memory mapping
int sys_mprotect(void *addr UNUSED, size_t len UNUSED, int prot UNUSED)
{
    serial_printf("mprotect not implemented yet\n");
    return 0;
}

//FIXME Taken from linux, only applicable to i386?
#define TASK_UNMAPPED_BASE (PHYS_BASE/3)

//Move this to the region code?
void *sys_mmap2(void *addr, size_t length, int prot, int flags, int fd, off_t pgoffset)
{
	printf("MMAP! Addr %p, length %x prot %x flags %x fd %i pgoffset %i\n", addr, length, prot, flags, fd, pgoffset);

	if ((flags & MAP_FIXED) == 0) {
		thread_t *cur = thread_current();
		addr += (uintptr_t)cur->mm->mmap_base;
		cur->mm->mmap_base += length;
	}

	if ((flags & MAP_ANONYMOUS)) {
		thread_t *cur = thread_current();
		memregion_map_data(cur->mm, (uintptr_t)addr, length, prot, MAP_FIXED, NULL);
		return addr;
	}

	struct file *file = vfs_file_get(fd);
	if (file == NULL)
		return (void *)-EBADF; // 

	thread_t *cur = thread_current();
	memregion_map_file(cur->mm, (uintptr_t)addr&PAGE_MASK, length, prot, MAP_FILE, file->dentry, pgoffset*4096, file->inode->info.st_size);

	return (void *) (((uintptr_t)addr + PAGE_SIZE - 1) & PAGE_MASK);
}

int sys_munmap(void *addr, size_t length)
{
	printf("MUNMAP: %p %x\n", addr, length);

	return -ENOSYS;
}

//XXX: This and sbrk should be using the memregion interface
int sys_brk(void *_addr)
{
	thread_t *cur = thread_current();
	uintptr_t addr = (uintptr_t)_addr;
	serial_printf("BRK %X\n", addr);
	if(addr == 0)
	{
		return (int)cur->mm->brk;
	}

	cur->mm->brk = _addr;
//	printf("addr %x cur %x\n", addr, cur->brk);
//	cur->brk = (void *)((uintptr_t)0x8000000 + (uintptr_t)addr);
	return (int)addr;
}

//FIXME: Needs more error/bounds checking
void *sys_sbrk(intptr_t ptr)
{
	printf("SBRK\n");
	PANIC("SRBK");
	void * old;
	thread_t *cur = thread_current();

	if (ptr == 0) {
		return cur->mm->brk;
	} else {
		old = cur->mm->brk;
		cur->mm->brk = cur->mm->brk + ptr;
		return old;
	}
}