/*	ChickenOS - mm/vm.c
 *	Handles paging, gdt, and page allocation
 *  Heap allocation is implemented with liballoc
 *  which uses pages from the page allocator
 */
#include <common.h>
#include <mm/vm.h>
#include <mm/paging.h>
#include <kernel/hw.h>
#include <kernel/interrupt.h>
#include <kernel/thread.h>
#include <stdio.h>
#include <multiboot.h>

extern uint32_t end;
uint32_t mem_size;
//TODO: Pagefaults should really be handled here, at least the
//		platform independent parts
//TODO: Generic page insertion here?
void vm_init(struct kernel_boot_info *info)
{
	//FIXME: Finish multiboot parser
	struct multiboot_info *mb = (void *)info->aux;

	virt_addr_t placement = 0;
	mem_size = mb->mem_upper;
	
	if(mb->mods_count > 0 )
	{
		//TODO: doublecheck if this is correct (needs to skip over all modules
		placement =(uint32_t) P2V(*(void **)P2V(mb->mods_addr + 4));	
	}else{	
		placement = (unsigned)&end;
	}	

	//round up placement to nearest page	
	placement = (placement & PAGE_MASK) + PAGE_SIZE;
	
	//FIXME - 	might be fixable by splitting page allocator initilization into kernel and
	//			user pages
//	size = 1024 * 1024; //needed for machines with large amounts of memory at the moment
						//because we only have 4MB, and the bitmap overflows it 
	uint32_t page_count = (mem_size/4) - (V2P(placement)/PAGE_SIZE);
	
//	printf("%iMB installed %i\n", (size)/1024 + 2);
	mem_size += ((uintptr_t)&end - 0xC0010000)/1024;
	palloc_init(page_count, placement);
	
	paging_init(mem_size);	
}

void *mmap_base = (void *)0x5000000;
void *sys_mmap2(void *addr, size_t length, int prot, int flags, int fd, off_t pgoffset)
{
	(void)addr;
	(void)length;
	(void)prot;
	(void)flags;
	(void)fd;
	(void)pgoffset;
	printf("Addr %p, length %x prot %x flags %x fd %i pgoffset %i\n",
					addr, length, prot, flags, fd, pgoffset);
	if(addr == 0)
		addr = mmap_base;
	void *new = palloc(length / PAGE_SIZE);
	pagedir_t pd = thread_current()->pd;
	pagedir_insert_pagen(pd, (uintptr_t)new, (uintptr_t)mmap_base, 0x7, length/PAGE_SIZE);
	
	addr = mmap_base;
	mmap_base += length;

	
	
	
	return addr;//(void*)-1;//NULL;
}
