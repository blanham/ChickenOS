#include <kernel/vm.h>
#include <kernel/interrupt.h>
#include <kernel/bitmap.h>
#include <kernel/memory.h>
#include <string.h>
#include <stdio.h>
#include "debug.h"
#define PAGE_SIZE 4096
#define PAGE_MASK 0xFFFFF000
#define PD_SIZE 4096
extern unsigned int end;

unsigned int placement = 0;
phys_addr_t palloc_start = 0;
virt_addr_t heap_start;
bitmap_t page_bitmap;

//uint32_t placement;
// Declare the page directory and a page table, both 4kb-aligned

uint32_t kernelpagedir[1024] __attribute__ ((aligned (4096)));
uint32_t lowpagetable[1024] __attribute__ ((aligned (4096)));
uint32_t * kernel_pd;

#define PAGE_VIOLATION 0x01
#define PAGE_WRITE	   0x02
#define PAGE_USER	   0x04

uint32_t palloc_bitmap_storage[1024] __attribute__ ((aligned (4096)));
uint32_t *palloc_bitmap = palloc_bitmap_storage;
uint32_t *PD;
void gpf(struct registers *regs)
{
	uint32_t error_code = regs->err_code;
	printf("Error %x\n",error_code);
	dump_regs(regs);
	//triggers debugger in BOCHS
	asm volatile("xchg %bx, %bx");
	PANIC("GENERAL PROTECTION FAULT!");
}
void page_fault(struct registers * regs)
{
	void *faulting_addr;
	uint32_t error_code = regs->err_code;

	bool is_user, is_write, not_present;	
		
	is_user = ((PAGE_USER & error_code) ? TRUE : FALSE);
	is_write = ((PAGE_WRITE & error_code) ? TRUE : FALSE);
	not_present = ((PAGE_VIOLATION & error_code) ? FALSE : TRUE);
	if(is_user)
		printf("user space\n");
	else
		printf("kernel space\n");

	if(is_write)
		printf("write\n");
	else
		printf("read\n");
	
	if(not_present)
		printf("page not present\n");
	else
		printf("protection violation\n");

	asm volatile ("mov %%cr2, %0"
				  : "=r" (faulting_addr)

				 );
	printf("PAGE FAULT! @ %x\n",faulting_addr);

	if(!is_user)
	{
		dump_regs(regs);
		PANIC("Page fault in kernel space");
	}else {
		//kill process

	}
}

void palloc_internal_free(void *addr, int pages)
{
	uint32_t index = ((virt_addr_t)addr - palloc_start) / PAGE_SIZE;
	bitmap_clear_multiple(&page_bitmap, index, pages);
	//might copy pintos here and set entire page(s) to some magic number	
}

void palloc_free(void *addr)
{
	palloc_internal_free(addr,1);
}

void pallocn_free(void *addr, int pages)
{
	palloc_internal_free(addr,pages);
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
	uint32_t bitmap_length = (page_count/32);
	start = (void *)(placement + bitmap_length);

	if(((uint32_t)start & PAGE_MASK) != 0)
		start = (void *)(((uint32_t)start & PAGE_MASK) + PAGE_SIZE);
	
	bitmap_init_phys(&page_bitmap, page_count, (uint32_t *)bitmap_ptr);
	
	palloc_start = (phys_addr_t)start;
}

void heap_init()
{
/*	void *test = pallocn(9);
	void *test2 = palloc();
	printf("test %x test2 %x\n", test, test2);
	pallocn_free(test, 9);
	test = pallocn(21);
	void *test3 = palloc(); 

	printf("test %x test2 %x test3 %x\n", test, test2, test3);*/
}

void vm_init(struct multiboot_info *mb)
{
	
	//return;
	uint32_t size = mb->mem_upper;
	
	if(mb->mods_count > 0 )
	{
		placement =(uint32_t) P2V(*(void **)P2V(mb->mods_addr + 4));	
	}
	if(placement == 0)
		placement = (unsigned)&end;
	placement = (placement & PAGE_MASK) + PAGE_SIZE;
	uint32_t temp = (placement - PHYS_BASE)/PAGE_SIZE;
	//size = 1024 * 1024; //needed for machines with large amounts of memory at the moemnt
						//becuase we only have 4MB, and the bitmap overflows it 
	uint32_t page_count = (size/4) - temp;
	
//	printf("%iMB installed %i\n", (size)/1024 + 2);
//	printf("PC %i PL %X %x\n",page_count, placement, temp);	
	palloc_init(page_count, placement);
	PD = pallocn(4);
	paging_init();	
	interrupt_register(13, &gpf);
	interrupt_register(14, &page_fault);


	heap_init();
}

pagedir_t pagedir_new()
{
	pagedir_t new = palloc();

//	memcpy(new, kernel_pd, PD_SIZE);	

	return new;
}

void install_pagedir(uint32_t *pd)
{
	uint32_t pdn = V2P((uint32_t)pd);
	asm volatile (	"mov %0, %%eax\n"
				"mov %%eax, %%cr3\n"
				"mov %%cr0, %%eax\n"
				"orl $0x80000000, %%eax\n"
				"mov %%eax, %%cr0\n" :: "m" (pdn));
}






void paging_init()
{
	void *lowpagetablePtr = (char *)lowpagetable - PHYS_BASE;	

	kernel_pd = palloc();
	kmemsetl(kernel_pd, 0, 4*1024); 
	
	for (int k = 0; k < 1024; k++)
	{
		lowpagetable[k] = (k * 4096) | 0x3;	// ...map the first 4MB of memory into the page table...
	}
 
 
	kernel_pd[768] = (unsigned long)lowpagetablePtr | 0x3;

	install_pagedir(kernel_pd);
	gdt_install();

}

struct gdt_entry
{
	unsigned short limit_low;
	unsigned short base_low;
	unsigned char base_middle;
	unsigned char access;
	unsigned char granularity;
	unsigned char base_high;
} __attribute__((packed));
 
struct gdt_ptr
{
	unsigned short limit;
	unsigned int base;
} __attribute__((packed));
 
// We'll need at least 3 entries in our GDT...
 
struct gdt_entry gdt[3];
struct gdt_ptr gp;
 
// Extern assembler function
void gdt_flush();
 
// Very simple: fills a GDT entry using the parameters
void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran)
{
	gdt[num].base_low = (base & 0xFFFF);
	gdt[num].base_middle = (base >> 16) & 0xFF;
	gdt[num].base_high = (base >> 24) & 0xFF;
 
	gdt[num].limit_low = (limit & 0xFFFF);
	gdt[num].granularity = ((limit >> 16) & 0x0F);
 
	gdt[num].granularity |= (gran & 0xF0);
	gdt[num].access = access;
}
 
// Sets our 3 gates and installs the real GDT through the assembler function
void gdt_install()
{
	gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
	gp.base = (unsigned int)&gdt;
 
	gdt_set_gate(0, 0, 0, 0, 0);
	gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
	gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
 
	gdt_flush();
}

