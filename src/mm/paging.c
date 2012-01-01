/*	ChickenOS - mm/paging.c
 *	Handles paging, and gdt
 *  Heap allocation is implemented with liballoc
 *  which uses pages from the page allocator
 *  Uses code borrowed from JamesM's kernel tutorial
 *
 */
#include <common.h>
#include <kernel/vm.h>
#include <kernel/interrupt.h>
#include <kernel/memory.h>
#include <stdio.h>

#define PDE_MASK	0xffc00000
#define PTE_MASK	0x003ff000
#define PTE_P		0x00000001
#define PDE_SHIFT 22
#define PTE_SHIFT 12


pagedir_t kernel_pd;

pagedir_t pagedir_new()
{
	pagedir_t new = palloc();
	uint8_t *new_pt;
	uint32_t cur;
	
	for(int i = 768; i < 1024; i++)
	{
		if((kernel_pd[i] & PTE_PRESENT) != 0)
		{
			new_pt = palloc();
			cur = kernel_pd[i] & ~0xfff;
			kmemcpy(new_pt, (void *)P2V(cur), 4096);
			new[i] = V2P(new_pt) | 4 | 2 |1; 
		}
	}

	return new;
}

void pagedir_delete(pagedir_t pd)
{
	for(int i = 0; i < 1024; i ++)
	{
		if((pd[i] & PTE_PRESENT) != 0)
		{
			palloc_free((void *)pd[i]);
		} 
	}
}

void pagedir_install(uint32_t *pd)
{
	virt_addr_t pdn = V2P(pd);
	asm volatile (	"mov %0, %%eax\n"
				"mov %%eax, %%cr3\n"
				"mov %%cr0, %%eax\n"
				"orl $0x80000000, %%eax\n"
				"mov %%eax, %%cr0\n" :: "m" (pdn));
}

typedef uint32_t * page_table_t;
static phys_addr_t pagetable_init(phys_addr_t offset, uint8_t flags)
{
	page_table_t new = palloc();
	kmemsetl(new, 0, PD_SIZE); 
	phys_addr_t ret = 0;
	offset = (1024*4096)*(offset - 768);
	for (int k = 0; k < 1024; k++)
	{
		new[k] = offset | (k * 4096) | flags;
	}

	ret = V2P(new) | flags;
	return ret;
}

void paging_init()
{
	kernel_pd = palloc();
	kmemsetl(kernel_pd, 0, PD_SIZE); 
	
	for(int i = 768; i < 1024; i++)
		kernel_pd[i] = pagetable_init(i, 4 | 2 |1);
	 
	pagedir_install(kernel_pd);
	gdt_install();

}

void pagedir_insert_page(pagedir_t pd, virt_addr_t kvirt, 
	virt_addr_t uvirt,uint8_t flags)
{
	uint32_t pde_index, pte_index;
	uint32_t *pde_entry = pd;
	uint32_t *pte_entry;
	flags= flags;	
	pde_index = uvirt >> PDE_SHIFT;
	pte_index = (uvirt & PTE_MASK) >> PTE_SHIFT;

	pde_entry +=pde_index;
	
	if((*pde_entry & 0x1) == 0)
	{
		*pde_entry = (uint32_t)V2P(palloc()) | 0x7;	
	}

	pte_entry = (uint32_t *)P2V(((uintptr_t)(*pde_entry) & ~0xfff));
	pte_entry += pte_index;
	
	*pte_entry = V2P(kvirt) | 0x7;
}
