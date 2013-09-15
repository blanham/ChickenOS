/*	ChickenOS - mm/vm.c
 *	Handles paging, gdt, and page allocation
 *  Heap allocation is implemented with liballoc
 *  which uses pages from the page allocator
 *  Uses code borrowed from JamesM's kernel tutorial
 *
 */
#include <common.h>
#include <mm/vm.h>
#include <mm/paging.h>
#include <kernel/hw.h>
#include <kernel/interrupt.h>
#include <kernel/thread.h>
#include <stdio.h>

extern uint32_t end;
uint32_t mem_size;
void vm_init(struct multiboot_info *mb)
{
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

