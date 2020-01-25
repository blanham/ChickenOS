/*	ChickenOS - mm/palloc.c
 *	Page Allocator
 *	Similar in design to the allocator in Pintos
 *	FIXME: Seems to deadlock after allocating 225MB worth
 *	of pages
 */
#include <stdio.h>
#include <string.h>
#include <chicken/common.h>
#include <chicken/bitmap.h>
#include <chicken/boot.h>
#include <chicken/interrupt.h>
#include <chicken/mm/vm.h>


phys_addr_t palloc_start = 0;
bitmap_t page_bitmap;


//New page allocator:
//	Placement<->16MB - Bitmap
//		Needs to be able to allocate contiguous pages
//	16MB->PCI - Linked List
//   		Each free page contains pointer to next free page
//	PCI Space
//		Also bitmap
//	struct free_page {
//		uintptr_t next_page;
//	};
//
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
	memset(addr, 0xBC, PAGE_SIZE * pages);
}

void palloc_free(void *addr UNUSED)
{
	//palloc_internal_free(addr,1);
}

int pallocn_free(void *addr UNUSED, int pages UNUSED)
{
	//palloc_internal_free(addr,pages);
	return 0;
}

void *palloc_internal(int pages)
{
	int32_t first;
	virt_addr_t virt = 0xdeadbeef;
	if((first = bitmap_find_multiple(&page_bitmap, pages)) == BITMAP_ERROR)
		PANIC("NO MORE PAGES AVAILABLE");

	bitmap_set_multiple(&page_bitmap, first,pages);
	virt = palloc_start + (first * PAGE_SIZE);
	//XXX: This is just a test
	//serial_printf("VIRT %x\n", virt);
	memset((void *)virt, 0, PAGE_SIZE * pages);
	//serial_printf("BALLS\n");


	return (void *)virt;
}

void *palloc()
{
	return palloc_internal(1);
}

void *pallocn(uint32_t count)
{
	return palloc_internal(count);
}

void *palloc_len(size_t len UNUSED)
{
	return pallocn(PAGE_COUNT(len));
}

//XXX: WTF?
void palloc_init(struct kernel_boot_info *info)
{
	uintptr_t bitmap_ptr = (uintptr_t)info->placement;
	uint32_t page_count = (info->mem_size)/PAGE_SIZE;
	uint32_t bitmap_length = (page_count/32) * sizeof(uint32_t);

	uintptr_t start = bitmap_ptr + bitmap_length;

	// Ensure that we're aligned to a 64KB boundary
	uint32_t masker = PAGE_SIZE * 16;
	if(start & (masker-1))
		start = (start & ~(masker-1)) + masker;

	bitmap_init_phys(&page_bitmap, page_count, (uint32_t *)bitmap_ptr);

	palloc_start = (phys_addr_t)start;

	serial_printf("PALLOC START %p bitmap %p\n", palloc_start, bitmap_ptr);
}