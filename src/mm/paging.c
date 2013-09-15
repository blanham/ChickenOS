/*	ChickenOS - mm/paging.c
 *	Handles paging, and gdt
 *  Heap allocation is implemented with liballoc
 *  which uses pages from the page allocator
 *  Uses code borrowed from JamesM's kernel tutorial
 *
 */
#include <common.h>
#include <mm/paging.h>
#include <kernel/interrupt.h>
#include <kernel/memory.h>
#include <stdio.h>


//TODO: Cleanup, and add a pagedir_lookup function to see if an
//		address is valid
uint32_t _kernel_pd[4096] __attribute__((aligned (4096)));
pagedir_t kernel_pd = & _kernel_pd[0];
typedef uint32_t * page_table_t;

#define ASSERT_PAGE_ALIGNED(x) ASSERT(((uintptr_t)x & 0xFFF) == 0, "Page alignment failed!")
page_table_t pagetable_clone(pagedir_t pd UNUSED, uint32_t index UNUSED)
{
	//allocates new page for each mapped page and copies shit into it
	return NULL;
}




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

pagedir_t pagedir_clone(pagedir_t pd UNUSED)
{
	pagedir_t clone = pagedir_new();

	for(int i = 0; i < KENERL_PDE_START; i++)
	{
		//if present
		//pagetable_clone(pd, i)

	}

	return clone;
}


void pagedir_install(uint32_t *pd)
{
	asm volatile (	"mov %0, %%eax\n"
				"mov %%eax, %%cr3\n"
				"mov %%cr0, %%eax\n"
				"orl $0x80000000, %%eax\n"
				"mov %%eax, %%cr0\n" :: "r" V2P(pd));
}

pagedir_t pagedir_get()
{
	pagedir_t pd;

	asm volatile(	"mov %%cr3, %%eax\n"
					"mov %%eax, %0\n":"=r" (pd));
	return (pagedir_t)P2V(pd);
}

void pagedir_insert_page(pagedir_t pd, virt_addr_t kvirt, 
	virt_addr_t uvirt,uint8_t flags)
{
	(void)pd;
	(void)kvirt;
	(void)uvirt;
	(void)flags;
}

void pagedir_insert_page_physical(pagedir_t pd, phys_addr_t kphys, 
	virt_addr_t uvirt,uint8_t flags)
{
	(void)pd;
	(void)kphys;
	(void)uvirt;
	(void)flags;

	//pagedir_install(pd);
}

void pagedir_insert_pagen(pagedir_t pd, virt_addr_t kvirt, 
	virt_addr_t uvirt,uint8_t flags, int n)
{
	for(int i = 0; i < n; i++)
		pagedir_insert_page(pd, kvirt + PAGE_SIZE*i, uvirt + PAGE_SIZE*i, flags);
}

void pagedir_insert_pagen_physical(pagedir_t pd, phys_addr_t kphys, 
	virt_addr_t uvirt,uint8_t flags, int n)
{
	for(int i = 0; i < n; i++)
		pagedir_insert_page_physical(pd, kphys + PAGE_SIZE*i, uvirt + PAGE_SIZE*i, flags);
}

static phys_addr_t pagetable_init(phys_addr_t pt, uint8_t flags)
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

//FIXME: should take into account RAM size
void paging_init(uint32_t mem_size UNUSED)
{
	
	kmemset(kernel_pd, 0, PD_SIZE);
 
	for(int i = KERNEL_PDE_START; i < 880; i++)
		kernel_pd[i] = pagetable_init(i - KERNEL_PDE_START, PTE_RW | PTE_P);
	 
	pagedir_install(kernel_pd);
	
	gdt_install();
}

//FIXME: 
/*
page_table_t pagetable_clone(page_table_t pt)
{
	page_table_t new = palloc();
	ASSERT_PAGE_ALIGNED(pt);
	
	kmemcpy(new, (void *)P2V(pt), 4096);

	for(int i = 0; i < 1024; i++)
	{
		//new[i] |= new[i];// & ~PTE_RW; 
	}	
	
	return (page_table_t)V2P(new);
}



pagedir_t pagedir_new()
{
	pagedir_t new = palloc();
	uint8_t *new_pt;
	uint32_t cur;
	
	for(int i = 0; i < 1024; i++)
	{
		if((kernel_pd[i] & PTE_P) != 0)
		{
			new_pt = palloc();
			cur = kernel_pd[i] & ~0xfff;
			kmemcpy(new_pt, (void *)P2V(cur), 4096);
			new[i] = V2P(new_pt) | PDE_USER | PDE_RW | PDE_P; 
		}
	}

	return new;
}


void pagedir_delete(pagedir_t pd)
{
	PANIC("Function broken\n");
	for(int i = 0; i < 1024; i ++)
	{
		if((pd[i] & PTE_P) != 0)
		{
			palloc_free((void *)P2V(pd[i]));
		} 
	}
}

pagedir_t pagedir_clone(pagedir_t pd)
{
	pagedir_t new = palloc();
	uint8_t *new_pt;
	page_table_t cur;	

	for(int i = 0; i < 1024; i++)
	{
		if((pd[i] & PTE_P) != 0)
		{
			new_pt = palloc();
		//	new[i] = (uintptr_t)pagetable_clone((page_table_t)pd[i]);
			cur = (page_table_t)((uintptr_t)P2V(pd[i]) & ~0xfff);
			kmemcpy(new_pt, (void *)cur, 4096);
			new[i] = V2P(new_pt) | PDE_RW | PDE_USER | PDE_P;
		} 
	}

	return new;	
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



void pagedir_insert_page(pagedir_t pd, virt_addr_t kvirt, 
	virt_addr_t uvirt,uint8_t flags)
{
	uint32_t pde_index, pte_index;
	uint32_t *pde_entry = pd;
	uint32_t *pte_entry;
	
	pde_index = uvirt >> PDE_SHIFT;
	pte_index = (uvirt & PTE_MASK) >> PTE_SHIFT;

	pde_entry +=pde_index;
	if((*pde_entry & 0x1) == 0)
	{
		*pde_entry = (uint32_t)V2P(palloc()) | flags;	
	}

	pte_entry = (uint32_t *)P2V(((uintptr_t)(*pde_entry) & ~0xfff));
	pte_entry += pte_index;
	
	*pte_entry = V2P(kvirt) | flags;
}

void pagedir_insert_page_physical(pagedir_t pd, phys_addr_t kphys, 
	virt_addr_t uvirt,uint8_t flags)
{
	uint32_t pde_index, pte_index;
	uint32_t *pde_entry = pd;
	uint32_t *pte_entry;
	
	pde_index = uvirt >> PDE_SHIFT;
	pte_index = (uvirt & PTE_MASK) >> PTE_SHIFT;


	pde_entry +=pde_index;
	if((*pde_entry & 0x1) == 0)
	{
		*pde_entry = (uint32_t)V2P(palloc()) | flags;	
	}

	pte_entry = (uint32_t *)P2V(((uintptr_t)(*pde_entry) & ~0xfff));
	pte_entry += pte_index;
	
	*pte_entry = (kphys) | flags;
	
	pagedir_install(pd);
}

void pagedir_insert_pagen(pagedir_t pd, virt_addr_t kvirt, 
	virt_addr_t uvirt,uint8_t flags, int n)
{
	for(int i = 0; i < n; i++)
		pagedir_insert_page(pd, kvirt + PAGE_SIZE*i, uvirt + PAGE_SIZE*i, flags);
}

void pagedir_insert_pagen_physical(pagedir_t pd, phys_addr_t kphys, 
	virt_addr_t uvirt,uint8_t flags, int n)
{
	for(int i = 0; i < n; i++)
		pagedir_insert_page_physical(pd, kphys + PAGE_SIZE*i, uvirt + PAGE_SIZE*i, flags);
}

static phys_addr_t pagetable_init(phys_addr_t offset, uint8_t flags)
{
	page_table_t new = palloc();
	phys_addr_t ret = 0;
	
	kmemset(new, 0, PD_SIZE); 

	offset = (PTE_COUNT*PTE_SIZE)*(offset - KERNEL_PDE_START);
	
	for (int k = 0; k < 1024; k++)
	{
		new[k] = offset | (k * 4096) | flags;
	}

	ret = V2P(new) | flags;
	
	return ret;
}
*/

