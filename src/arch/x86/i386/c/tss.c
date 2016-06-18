#include <common.h>
#include "tss.h"
#include <mm/vm.h>
#include <kernel/thread.h>
#include <memory.h>
tss_t tss;
extern void tss_flush();
void tss_init(void)
{
	kmemset(&tss, 0, 104);
	tss.ss0  = 0x10;
	tss.io_bmap = sizeof(tss);
//	uint32_t esp;
//	asm volatile ("mov %0, %%esp"::"m"(esp));
//	tss.esp0 = esp;
	tss.cs = 0xb;
	tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x13;
//	asm volatile(
//				"mov $0x2B, %ax\n"//0x28->tss is sixth entry in GDT
//				"ltr %ax\n"
//				);
//	tss_flush();
}

void
tss_update(uint32_t esp)
{
	//if(esp == 0)
	//	esp = (uintptr_t)((uintptr_t)thread_current() + 4096);
	tss.esp0 = esp;
}
