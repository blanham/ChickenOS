/*	ChickenOS - mm/palloc.c
 *	Page Allocator 
 *	Similar in design to the allocator in Pintos
 *	FIXME: Seems to deadlock after allocating 225MB worth
 *	of pages
 */
#include <kernel/common.h>
#include <mm/vm.h>
#include <kernel/interrupt.h>
#include <kernel/bitmap.h>
#include <kernel/memory.h>
#include <string.h>
#include <stdio.h>

phys_addr_t palloc_start = 0;
bitmap_t page_bitmap;

//if i seperate user/kernel pages, might build a struct:
/*struct page_arena{
	phys_addr_t arena_start;
	bitmap_t page_bitmap;
};
*/
void palloc_internal_free(void *addr, int pages)
{
	uint32_t index = ((virt_addr_t)addr - palloc_start) / PAGE_SIZE;
	bitmap_clear_multiple(&page_bitmap, index, pages);
	//Inspired by Pintos, sets pages to a magic number
	//so if we accidentally keep using a freed page
	//we'll know
	kmemset(addr, 0xBC, PAGE_SIZE * pages);
}

void palloc_free(void *addr)
{
	palloc_internal_free(addr,1);
}

int pallocn_free(void *addr, int pages)
{
	palloc_internal_free(addr,pages);
	return 0;
}


void *palloc_internal(phys_addr_t *phys, int pages)
{
	int32_t first;
	virt_addr_t virt = 0xdeadbeef;
	if((first = bitmap_find_multiple(&page_bitmap, pages)) == BITMAP_ERROR)
		PANIC("NO MORE PAGES AVAILABLE");

	bitmap_set_multiple(&page_bitmap, first,pages);
	virt = palloc_start + (first * PAGE_SIZE);
	if(phys != 0)
		*phys = (phys_addr_t)(virt - PHYS_BASE);	

	return (void *)virt;
}

void *palloc()
{
	return palloc_internal(0, 1);
}

void *pallocn(uint32_t count)
{
	return palloc_internal(0, count);
}

void palloc_init(uint32_t page_count, uint32_t placement)
{
	void *start = 0;
	uint32_t bitmap_ptr = placement;
	uint32_t bitmap_length = (page_count/32) * sizeof(uint32_t);
	
	start = (void *)(placement + bitmap_length);

	if(((uint32_t)start & ~PAGE_MASK) != 0)
		start = (void *)(((uint32_t)start & PAGE_MASK) + PAGE_SIZE);
	
	bitmap_init_phys(&page_bitmap, page_count, (uint32_t *)bitmap_ptr);
	
	palloc_start = (phys_addr_t)start;
}

