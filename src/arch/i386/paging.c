/*	ChickenOS - mm/paging.c
 *
 * TODO: Add more assertions?
 * 		 Maybe a bit more cleanup
 */

#include <string.h>
#include <chicken/common.h>
#include <chicken/mm/vm.h>
#include "i386_defs.h"

//Might make more sense to put this into the asm file
uint32_t kernel_pd[1024] __attribute__((aligned (4096)));

//XXX: Not sure if I want to bother with this here, should be somewhere else anyway
//		probably in assert.h
#define ASSERT_PAGE_ALIGNED(x) ASSERT(((uintptr_t)x & 0xFFF) == 0, "Page alignment failed!")

uint32_t *pagedir_alloc()
{
	uint32_t *new = palloc();
	memcpy(new, kernel_pd, PAGE_SIZE);
	return new;
}

uint32_t *pagetable_alloc()
{
	uint32_t *new = palloc();
	memset(new, 0, PAGE_SIZE);
	return new;
}

//NOTE: This should probably just iterate through the PD
//      quickly and verify that everything is clear, as
//      it would be much more efficient/cleaner to unmap
//      pages in the memory regions code, instead of having
//      to do a linear search
void pagedir_free(uint32_t *pd)
{
	palloc_free(pd);
}

//Implement these two in assembly
void page_invalidate(uintptr_t page)
{
	asm volatile ("invlpg %0" :: "m" (page));
}

void pagedir_activate(uint32_t *pd)
{
	asm volatile (
		"mov %0, %%eax\n"
		"mov %%eax, %%cr3\n":: "r"
		V2P(pd));
}

void pagedir_map(uint32_t *pd, uintptr_t kvaddr, uintptr_t vaddr, bool rw)
{
	uint32_t pt_idx = (vaddr & PTE_MASK) >> PTE_SHIFT;
	uint32_t *pde = &pd[vaddr >> PDE_SHIFT];

	if (!(*pde & PDE_P))
		*pde = (uintptr_t)V2P(palloc()) | PDE_RW | PDE_USER | PDE_P;

	uint32_t *pt = P2V(*pde & PAGE_MASK);
	pt[pt_idx] = (uintptr_t)V2P(kvaddr) | (rw ? PTE_RW : 0) | PTE_USER | PTE_P;
}

void pagedir_map2(uint32_t *pd, uintptr_t kvaddr, uintptr_t vaddr, bool rw, bool present)
{
	uint32_t pt_idx = (vaddr & PTE_MASK) >> PTE_SHIFT;
	uint32_t *pde = &pd[vaddr >> PDE_SHIFT];

	if (!(*pde & PDE_P))
		*pde = (uintptr_t)V2P(palloc()) | PDE_RW | PDE_USER | PDE_P;

	uint32_t *pt = P2V(*pde & PAGE_MASK);
	pt[pt_idx] = (uintptr_t)V2P(kvaddr) | (rw ? PTE_RW : 0) | PTE_USER | (present ? PTE_P : 0);
}

void pagedir_map_kernel(uintptr_t kvaddr, uintptr_t vaddr, bool rw)
{
	uint32_t pt_idx = (vaddr & PTE_MASK) >> PTE_SHIFT;
	uint32_t *pde = &kernel_pd[vaddr >> PDE_SHIFT];

	if (!(*pde & PDE_P))
		*pde = (uintptr_t)V2P(palloc()) | PDE_RW | PDE_P;

	uint32_t *pt = P2V(*pde & PAGE_MASK);
	pt[pt_idx] = (uintptr_t)V2P(kvaddr) | (rw ? PTE_RW : 0) | PTE_P;
}


void pagedir_unmap(uint32_t *pd, uintptr_t addr)
{
	uint32_t pt_idx = (addr & PTE_MASK) >> PTE_SHIFT;
	uint32_t pde = pd[addr >> PDE_SHIFT];

	if (!(pde & PDE_P)) {
		//TODO: Probably more of a warning than a panic
		PANIC("Trying to remove nonmapped page");
		return;
	}

	uint32_t *pt = P2V(pde & PAGE_MASK);
	pt[pt_idx] = 0;
}

uintptr_t pagedir_lookup(uint32_t *pd, uintptr_t addr)
{
	uint32_t pt_idx = (addr & PTE_MASK) >> PTE_SHIFT;
	uint32_t pde = pd[addr >> PDE_SHIFT];

	if (!(pde & PDE_P))
		return 0;

	uint32_t *pt = P2V(pde & PAGE_MASK);
	return pt[pt_idx] & PAGE_MASK;
}

//XXX: Don't use this, but commit it first so that I have a backup of the cleaned up version
//Not even sure I need this, it may be more efficient to remap + change flags
//If I remove this make a function pagedir_page_clone(old_pd, new_pd, virtual_addr)?
//Nah, probably still faster overall to only copy/mark the pages we need
//
//Basically, boils down to if I want to mess with the frame allocator here (as in increase
//the refcounts for physical pages)
uint32_t *pagedir_clone(uint32_t *pd)
{
	uint32_t *new = pagedir_alloc();

	for (int i = 0; i < KERNEL_PDE_START; i++) {
		uint32_t pde = pd[i];
		if (!(pde & PDE_P))
			continue;

			uint32_t *new_pt = pagetable_alloc();
			uint32_t *old_pt = P2V(pde & PAGE_MASK);

			for (int i = 0; i < 1024; i++) {
				if(!(old_pt[i] & PTE_P))
					continue;
				//XXX: This should also increase reference count
				//mark all pages as read only
				//old_pt[i] &= ~PTE_RW;
				void * new = palloc();
				memcpy(new, P2V(old_pt[i] &PAGE_MASK), PAGE_SIZE);
				new_pt[i] = (uintptr_t)V2P(new) | (old_pt[i] & ~PAGE_MASK) ;

			}
			new[i] = (uintptr_t)V2P(new_pt) | PDE_RW | PDE_USER | PDE_P;
	}

	return new;
}

void paging_init(uint32_t mem_size)
{
	memset(kernel_pd, 0, PAGE_SIZE);

	for (uintptr_t i = PHYS_BASE; i < PHYS_BASE + mem_size; i += PAGE_SIZE) {
		pagedir_map(kernel_pd, i, i, true);
	}

	pagedir_activate(kernel_pd);

	gdt_init();
}