#include <common.h>
#include <thread/tss.h>
#include <kernel/vm.h>
#include <kernel/thread.h>
#include <memory.h>
tss_t tss;
void tss_init(void)
{
	kmemset(&tss, 0, 104);
	tss.ss0  = 0x10;
	tss.io_bmap = sizeof(tss);	

//	tss.cs = 0xb;
//	tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x13;
//	asm volatile( 
//				"mov $0x2B, %ax\n"//0x28->tss is sixth entry in GDT
//				"ltr %ax\n"
//				);	
}

void
tss_update(uint32_t esp)
{
	if(esp == 0)
		esp = (uintptr_t)((uintptr_t)thread_current() + 4096);
	tss.esp0 = esp;
}
