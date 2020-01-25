#include <string.h>
#include <chicken/common.h>
#include "i386_defs.h"

#define GDT_CNT 9
gdt_seg_t gdt_entries[GDT_CNT];
struct gdt_descriptor gdt_desc;
tss_t tss;

// XXX: Does this need to also reload the tss with ltr?
void tss_update(uint32_t esp)
{
	tss.esp0 = esp;
}

void gdt_set_entry(int entry, uint32_t base, uint32_t limit, uint8_t flags, uint8_t access)
{
    gdt_seg_t *sd = &gdt_entries[entry];
	sd->base0 	= (base & 0xFFFF);
	sd->base16	= (base >> 16) & 0xFF;
	sd->base24	= (base >> 24) & 0xFF;
	sd->limit0 	= (limit & 0xFFFF);
	sd->flags 	= (flags & 0xf0) | ((limit >> 16) & 0xF);
	sd->access 	= access;

}

// FIXME: Remove all magic numbers, enable GS use for TLS
void gdt_init(void)
{
	gdt_desc.size 	  = (sizeof(struct segment_descriptor)*GDT_CNT) - 1;
	gdt_desc.location = (uintptr_t)&gdt_entries;

	gdt_set_entry(0, 0, 0, 0, 0);
	gdt_set_entry(1, 0, 0xFFFFFFF, GDTF_BOTH, GDTA_KERNEL);
	gdt_set_entry(2, 0, 0xFFFFFFF, GDTF_BOTH, GDTA_KERNEL_DATA);
	gdt_set_entry(3, 0, 0xFFFFFFF, GDTF_BOTH, GDTA_USER);
	gdt_set_entry(4, 0, 0xFFFFFFF, GDTF_BOTH, GDTA_USER_DATA);
	gdt_set_entry(5, (uintptr_t)&tss, (uint32_t)&tss + sizeof(tss), 0xcf, 0x89);
	//gdt_set_entry(6, 0x0825B2BC, 0xFFFFF, GDTF_BOTH, GDTA_USER_DATA);
	//gdt_set_entry(6, 0x80E0A2C , 0xFFFFFF, GDTF_BOTH, GDTA_USER_DATA);
	//gdt_set_entry(6, 0xE434C, 0xFFFFFF, GDTF_BOTH, GDTA_USER_DATA);
	gdt_set_entry(6, 0, 0xFFFFFFF, GDTF_BOTH, GDTA_USER_DATA);
	gdt_set_entry(7, 0, 0, 0, 0);
	gdt_set_entry(8, 0, 0, 0, 0);

	memset(&tss, 0, 104);
	tss.ss0  = 0x10;
	tss.cs = 0xb;
	tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x13;
	tss.io_bmap = sizeof(tss);

	//XXX: This should be a macro, since it is cross platform
	asm volatile ("":::"memory");
	gdt_flush(&gdt_desc);
}

void gdt_floosh() {
	asm volatile ("":::"memory");
	gdt_flush(&gdt_desc);
}