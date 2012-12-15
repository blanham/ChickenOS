#include <common.h>
#include <memory.h>
#include <kernel/vm.h>
#include <thread/tss.h>
struct segment_descriptor {
	uint16_t limit0;
	uint16_t base0;
	uint8_t base16;
	uint8_t access;
	uint8_t flags;
	uint8_t base24;	

} __attribute__((packed)) gdt_entries[6];

struct gdt_descriptor
{
	uint16_t size;
	uint32_t location;
} __attribute__((packed)) gdt_desc;

#define GDTA_P		 0x80	//present
#define GDTA_RING0 	 0x00	
#define GDTA_RING3 	 0x60
#define GDTA_ALWAYS	 0x10	//always 1
#define GDTA_EXE	 0x08
#define GDTA_CANJUMP 0x04	//can jump to lower ring levels
#define GDTA_RW		 0x02
#define GDTA_ACCESS  0x01
#define GDTF_32BIT 	 0x40
#define GDTF_4KB 	 0x80 

#define GDTA_KERNEL 	 (GDTA_P | GDTA_RING0 | GDTA_ALWAYS | GDTA_EXE  | GDTA_RW)
#define GDTA_KERNEL_DATA (GDTA_P | GDTA_RING0 | GDTA_ALWAYS | GDTA_RW)
#define GDTA_USER 		 (GDTA_P | GDTA_RING3 | GDTA_ALWAYS | GDTA_EXE |  GDTA_RW)
#define GDTA_USER_DATA 	 (GDTA_P | GDTA_RING3 | GDTA_ALWAYS | GDTA_RW)
#define GDTA_TSS 	 	 (GDTA_P | GDTA_RING3 | GDTA_EXE | GDTA_ACCESS)
#define GDTF_BOTH 		 (GDTF_32BIT | GDTF_4KB)

static void
gdt_fill(struct segment_descriptor *sd, uint32_t base, 
	uint32_t limit, uint8_t flags, uint8_t access)
{
	sd->base0 	= (base & 0xFFFF);
	sd->base16	= (base >> 16) & 0xFF;
	sd->base24	= (base >> 24) & 0xFF;
	sd->limit0 	= (limit & 0xFFFF);
	sd->flags 	= (flags & 0xf0) | ((limit >> 16) & 0xF);
	sd->access 	= access;
}
extern void gdt_flush(uintptr_t ptr);
void
gdt_install(void)
{
	gdt_desc.size 	  = (sizeof(struct segment_descriptor)*6) - 1;
	gdt_desc.location = (uintptr_t)&gdt_entries;

	gdt_fill(&gdt_entries[0], 0, 0, 0, 0);
	gdt_fill(&gdt_entries[1], 0, 0xFFFFF, GDTF_BOTH, GDTA_KERNEL);
	gdt_fill(&gdt_entries[2], 0, 0xFFFFF, GDTF_BOTH, GDTA_KERNEL_DATA);
	gdt_fill(&gdt_entries[3], 0, 0xFFFFF, GDTF_BOTH, 0xFA);
	gdt_fill(&gdt_entries[4], 0, 0xFFFFF, GDTF_BOTH, 0xF2);

kmemset(&tss, 0, 104);
	tss.ss0  = 0x10;
	tss.io_bmap = sizeof(tss);	
//	uint32_t esp;
//	asm volatile ("mov %0, %%esp"::"m"(esp));
//	tss.esp0 = esp;	
	tss.cs = 0xb;
	tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x13;


	gdt_fill(&gdt_entries[5], (uintptr_t)&tss, 0x67, 0xcf, GDTA_TSS);
	gdt_flush((uintptr_t)&gdt_desc);
	/*asm volatile(
			//	"jmp $0x8, $test\n"
			//	"test:\n"
				"lgdt 		(%0)\n"
				"mov   %1,	%%ax\n"
				"mov %%ax, %%ds\n" 
				"mov %%ax, %%es\n" 
				"mov %%ax, %%fs\n" 
				"mov %%ax, %%gs\n" 
			   	"mov %%ax, %%ss\n"
			//	"mov $0x28, %%ax\n"
			//	"ltr %%ax\n"
				::
				"r"(&gdt_desc),
				"K"(KERNEL_SEG)//,
			//	"q"(0x28)
				);*/
}
