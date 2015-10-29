/*
 *
 */
#include <common.h>
#include <mm/vm.h>
#include <stdio.h>

//FIXME Taken from linux, only applicable to i386?
#define TASK_UNMAPPED_BASE (PHYS_BASE/3)

void *mmap_base = (void *)0x5000000;
//Move this to the region code?
void *sys_mmap2(void *addr, size_t length, int prot, int flags, int fd, off_t pgoffset)
{
	(void)addr;
	(void)length;
	(void)prot;
	(void)flags;
	(void)fd;
	(void)pgoffset;
	PANIC("FUCKING THING SUCKS\n");
	printf("Addr %p, length %x prot %x flags %x fd %i pgoffset %i\n",
					addr, length, prot, flags, fd, pgoffset);

	if(addr == NULL)
	{
		//Starting with mmap base
		//look through threads memregions
		//if
		addr = mmap_base;

	}
//	void *new = palloc(length / PAGE_SIZE);
//	pagedir_t pd = thread_current()->pd;
//	pagedir_insert_pagen(pd, (uintptr_t)new, (uintptr_t)mmap_base, 0x7, length/PAGE_SIZE);

	addr = mmap_base;
	mmap_base += length;

	return addr;//(void*)-1;//NULL;
}

//sys_munmap - If file mapping writes dirty pages to disk
//sys_msync - Syncs dirty pages to disk
