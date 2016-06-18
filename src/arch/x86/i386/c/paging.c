/*	ChickenOS - mm/paging.c
 *	Handles paging, and gdt
 *  Heap allocation is implemented with liballoc
 *  which uses pages from the page allocator
 *  Uses code borrowed from JamesM's kernel tutorial
 *
 */
#include <common.h>
#include <stdio.h>
#include <mm/vm.h>
#include <kernel/interrupt.h>
#include <kernel/memory.h>
#include "gdt.h"

//TODO: Cleanup, and add a pagedir_lookup function to see if an
//		address is valid
uint32_t _kernel_pd[1024] __attribute__((aligned (4096)));
pagedir_t kernel_pd = & _kernel_pd[0];
typedef uint32_t * page_table_t;

#define ASSERT_PAGE_ALIGNED(x) ASSERT(((uintptr_t)x & 0xFFF) == 0, "Page alignment failed!")

pagedir_t pagedir_new()
{
	pagedir_t new = palloc();

	kmemcpy(new, kernel_pd, PD_SIZE);

	return new;
}

//FIXME: doesn't do anything for now
void pagedir_delete(pagedir_t pd UNUSED)
{

}

void page_invalidate(uintptr_t page)
{
	asm volatile ("invlpg %0" :: "m" (page));
}

void pagedir_install(uint32_t *pd)
{
	asm volatile (
				"mov %0, %%eax\n"
				"mov %%eax, %%cr3\n"
				"mov %%cr0, %%eax\n"
				"orl $0x80000000, %%eax\n"
				"mov %%eax, %%cr0\n" :: "r"
				V2P(pd));
}

pagedir_t pagedir_get()
{
	pagedir_t pd;

	asm volatile(	"mov %%cr3, %%eax\n"
					"mov %%eax, %0\n":"=r" (pd));
	return (pagedir_t)P2V(pd);
}

void pagedir_insert_page_internel(pagedir_t pd, virt_addr_t kaddr,
	virt_addr_t uvirt,uint8_t flags, bool phys)
{
	page_table_t pde, pte;
	uint32_t pd_idx = uvirt >> PDE_SHIFT;
	uint32_t pt_idx = (uvirt & PTE_MASK) >> PTE_SHIFT;

	pde = &pd[pd_idx];

	if((*pde & PDE_P) == 0)
	{
		*pde = V2P(palloc()) | flags | PDE_P;
	}

	pte = (page_table_t)P2V(*pde & ~0xFFF) + pt_idx;

	if(!phys)
		kaddr = V2P(kaddr);

	*pte = kaddr | flags | PTE_P;
}

page_table_t pagetable_clone(page_table_t pt)
{
	page_table_t new = palloc();
	page_table_t p = NULL;
//	pt = (void*)((uintptr_t)pt & (PDE_MASK | PTE_MASK));
//	ASSERT_PAGE_ALIGNED(pt);
	p = (void *)pt;//P2V(pt);
	for(int i = 0; i < 1024; i++)
	{
		if(p[i] & PTE_P)
		{
			new[i] = (pt[i]);
		//	printf("P %p p %x pt %p\n", p, p[i], pt);
			/*
			new[i] = (uint32_t)palloc();
			kmemcpy((void*)new[i], (void*)P2V(*p & ~0x3ff), PAGE_SIZE);
			new[i] = V2P(new[i]) | (*p & 0x3ff);*/
		}
	}

	return (page_table_t)V2P(new);
}
//FIXME: Inefficient
//Once we have memory regions we can just
//Directly map what we need instead of scanning and copying
pagedir_t pagedir_clone(pagedir_t pd)
{
	pagedir_t new = palloc();
	uint32_t *new_pt;
	page_table_t cur;

	for(int i = 0; i < 1024; i++)
	{
		if((pd[i] & PTE_P) != 0)
		{
			new_pt = palloc();
			cur = (page_table_t)((uintptr_t)P2V(pd[i]) & ~0xfff);
			if(i < 768)
			{
				for(int i = 0; i < 1024; i++)
				{
					new_pt[i] = (uintptr_t)palloc();
					kmemcpy((void *)new_pt[i], (void *)P2V(cur[i] & ~0x3FF), 4096);
					new_pt[i] = V2P(new_pt[i]) | PTE_RW | PTE_USER | PDE_P;
				}
			}
			else
			{
					kmemcpy(new_pt, (void *)cur, 4096);
			}
			new[i] = V2P(new_pt) | PDE_RW | PDE_USER | PDE_P;
		}
	}

	return new;
}

pagedir_t pagedir_copy(pagedir_t pd)
{
	pagedir_t new = palloc();
	uint32_t *new_pt;
	page_table_t cur;

	for(int i = 0; i < 1024; i++)
	{
		if((pd[i] & PTE_P) != 0)
		{
			new_pt = palloc();
			cur = (page_table_t)((uintptr_t)P2V(pd[i]) & ~0xfff);
			if(i < 768)
			{
				for(int i = 0; i < 1024; i++)
				{
					if((cur[i] & PTE_P) == 0)
						continue;
					//printf("cur %x\n", cur[i]);
					cur[i] &= ~PTE_RW;
					new_pt[i] = cur[i];

				}
			}
			else
			{
					kmemcpy(new_pt, (void *)cur, 4096);
			}
			new[i] = V2P(new_pt) | PDE_RW | PDE_USER | PDE_P;
		}
	}

	return new;
}

void pagedir_insert_page(pagedir_t pd, virt_addr_t kvirt,
	virt_addr_t uvirt,uint8_t flags)
{
	pagedir_insert_page_internel(pd, kvirt, uvirt,flags, false);
}

void pagedir_insert_page_physical(pagedir_t pd, phys_addr_t kphys,
	virt_addr_t uvirt,uint8_t flags)
{
	pagedir_insert_page_internel(pd, kphys, uvirt,flags, true);
}

void pagedir_insert_pagen(pagedir_t pd, virt_addr_t kvirt,
	virt_addr_t uvirt,uint8_t flags, int n)
{
	for(int i = 0; i < n; i++)
		pagedir_insert_page(pd, kvirt + PAGE_SIZE*i, uvirt + PAGE_SIZE*i, flags);
}

void pagedir_insert_pagen_physical(pagedir_t pd, phys_addr_t kphys,
	virt_addr_t uvirt, uint8_t flags, int n)
{
	for(int i = 0; i < n; i++)
		pagedir_insert_page_physical(pd, kphys + PAGE_SIZE*i, uvirt + PAGE_SIZE*i, flags);
}

static phys_addr_t pagetable_init(int pt, uint8_t flags)
{
	page_table_t new = palloc();
	phys_addr_t offset = 0;

	kmemset(new, 0, PTE_SIZE);

	offset = (PTE_COUNT*PTE_SIZE) * pt;

	for(int i = 0; i < PTE_COUNT; i++)
	{
		new[i] = offset | (i * PAGE_SIZE) | flags;
	}

	return V2P(new) | flags;
}

void pagedir_remove_page(pagedir_t pd, virt_addr_t virtual)
{
	page_table_t pde, pte;
	uint32_t pd_idx = virtual >> PDE_SHIFT;
	uint32_t pt_idx = (virtual & PTE_MASK) >> PTE_SHIFT;

	pde = &pd[pd_idx];

	if((*pde & PDE_P) == 0)
	{
		PANIC("Trying to remove nonmapped page");
	}

	pte = (page_table_t)P2V(*pde & ~0xFFF) + pt_idx;


	*pte = 0;
}

phys_addr_t pagedir_lookup(pagedir_t pd, virt_addr_t virtual)
{
	page_table_t pde, pte;
	uint32_t pd_idx = virtual >> PDE_SHIFT;
	uint32_t pt_idx = (virtual & PTE_MASK) >> PTE_SHIFT;

	pde = &pd[pd_idx];

	if((*pde & PDE_P) == 0)
	{
		return 0;
	}

	pte = (page_table_t)P2V(*pde & ~0xFFF) + pt_idx;


	return *pte & PAGE_MASK;
}

void pagedir_remove_pages(pagedir_t pd, virt_addr_t virtual, int n)
{
	virt_addr_t end = virtual + (n * PAGE_SIZE);
	for(; virtual < end; virtual += PAGE_SIZE)
		pagedir_remove_page(pd, virtual);
}

void paging_init(uint32_t mem_size UNUSED)
{
	kmemset(kernel_pd, 0, PD_SIZE);

	//FIXME: instead of 880, i believe it should be mem_size / (1024*1024)
	for(int i = KERNEL_PDE_START; i < 880; i++)
		kernel_pd[i] = pagetable_init(i - KERNEL_PDE_START, PTE_USER | PTE_RW | PTE_P);

	pagedir_install(kernel_pd);

	gdt_install();
}

