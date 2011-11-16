#include <kernel/vm.h>
#include <stdio.h>
extern unsigned int end;
// Declare the page directory and a page table, both 4kb-aligned
unsigned long kernelpagedir[1024] __attribute__ ((aligned (4096)));
unsigned long lowpagetable[1024] __attribute__ ((aligned (4096)));
 
void paging_init()
{
// Pointers to the page directory and the page table
	void *kernelpagedirPtr = 0;
	void *lowpagetablePtr = 0;
	int k = 0;
 
	kernelpagedirPtr = (char *)kernelpagedir + 0x40000000;	// Translate the page directory from
								// virtual address to physical address
	lowpagetablePtr = (char *)lowpagetable + 0x40000000;	// Same for the page table
 
	// Counts from 0 to 1023 to...
	for (k = 0; k < 1024; k++)
	{
		lowpagetable[k] = (k * 4096) | 0x3;	// ...map the first 4MB of memory into the page table...
		kernelpagedir[k] = 0;			// ...and clear the page directory entries
	}
 
	// Fills the addresses 0...4MB and 3072MB...3076MB of the page directory
	// with the same page table
 
	kernelpagedir[0] = (unsigned long)lowpagetablePtr | 0x3;
	kernelpagedir[768] = (unsigned long)lowpagetablePtr | 0x3;
	// Copies the address of the page directory into the CR3 register and, finally, enables paging!
 
	asm volatile (	"mov %0, %%eax\n"
			"mov %%eax, %%cr3\n"
			"mov %%cr0, %%eax\n"
			"orl $0x80000000, %%eax\n"
			"mov %%eax, %%cr0\n" :: "m" (kernelpagedirPtr));
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
	gp.limit = (sizeof(struct gdt_entry) * 6) - 1;
	gp.base = (unsigned int)&gdt;
 
	gdt_set_gate(0, 0, 0, 0, 0);
	gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
	gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
 
	gdt_flush();
}
void vm_init()
{
	printf("Kernel end %x\n", &end);
}
