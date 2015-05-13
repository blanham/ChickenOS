#include <common.h>
#include <memory.h>
#include <mm/vm.h>
#include <thread/tss.h>
#include <arch/i386/gdt.h>

gdt_seg_t gdt_entries[6];
struct gdt_descriptor gdt_desc;

static void
gdt_fill(gdt_seg_t *sd, uint32_t base, uint32_t limit, uint8_t flags, uint8_t access)
{
	sd->base0 	= (base & 0xFFFF);
	sd->base16	= (base >> 16) & 0xFF;
	sd->base24	= (base >> 24) & 0xFF;
	sd->limit0 	= (limit & 0xFFFF);
	sd->flags 	= (flags & 0xf0) | ((limit >> 16) & 0xF);
	sd->access 	= access;
}

void gdt_install(void)
{
	gdt_desc.size 	  = (sizeof(struct segment_descriptor)*6) - 1;
	gdt_desc.location = (uintptr_t)&gdt_entries;

	gdt_fill(&gdt_entries[0], 0, 0, 0, 0);
	gdt_fill(&gdt_entries[1], 0, 0xFFFFFFF, GDTF_BOTH, GDTA_KERNEL);
	gdt_fill(&gdt_entries[2], 0, 0xFFFFFFF, GDTF_BOTH, GDTA_KERNEL_DATA);
	gdt_fill(&gdt_entries[3], 0, 0xFFFFFFF, GDTF_BOTH, 0xFA);
	gdt_fill(&gdt_entries[4], 0, 0xFFFFFFF, GDTF_BOTH, 0xF2);

	kmemset(&tss, 0, 104);
	tss.ss0  = 0x10;
	tss.io_bmap = sizeof(tss);	
	tss.cs = 0xb;
	tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x13;

	gdt_fill(&gdt_entries[5], (uintptr_t)&tss, (uint32_t)&tss + sizeof(tss), 0xcf, 0x89);

	asm volatile ("":::"memory");
	gdt_flush(&gdt_desc);
}
