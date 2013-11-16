#include <common.h>
#include <stdint.h>
#include <kernel/thread.h>
#include <kernel/memory.h>
#include <kernel/interrupt.h>
#include <thread/tss.h>
#include <kernel/interrupt.h>

bool
in_kernel(void)
{
	uint16_t ss;	
	asm volatile ("mov %%ss, %0\n" : "=r"(ss));
	if((ss & 3) == 0)
		return true;

	return false;
}

void arch_thread_init()
{
	tss_init();
}

void thread_yield()
{
	asm volatile("int $32");
}

void thread_reschedule(registers_t *regs, thread_t *cur, thread_t *next)
{
	extern void pic_send_end(int irq);
	uint32_t _esp = 0;
	cur->sp = (uint8_t *)regs->ESP;
	_esp = (uint32_t)next->sp;

	tss_update((uintptr_t)next + STACK_SIZE);
	pagedir_install(next->pd);
//	printf("dfaddfafds\n");
//	dump_regs((void *)_esp + 4);

	//have to reset timer interrupt here
	pic_send_end(0);
	
	asm volatile(
					"mov %0,%%esp\n"
					"jmp intr_return"
					:: "r"(_esp)
				);


}

//Heh, we don't need this anymore
void thread_usermode(void)
{
	uint32_t cur_esp,new_esp;
	thread_t *cur;
	void *userstack;

	interrupt_disable();

	stackpointer_get(cur_esp);

	cur = thread_current();

	userstack = pallocn(STACK_PAGES);
	kmemcpy(userstack, cur, STACK_SIZE);

	pagedir_insert_pagen(cur->pd, (uintptr_t)userstack, 
			(uintptr_t)PHYS_BASE - STACK_SIZE, 0x7, STACK_PAGES);
	extern uintptr_t main_loc;


	printf("Main %p\n", main_loc);

	//puts new kernel stack in tss
	//FIXME? Since this is the kernel thread
	//we have important stuff on the stack
	//without the -8 offset we have a race condition
	//where, if an interrupt happens between the iret
	//below and returning, the stack is trashed
	void *temp_kern = pallocn(STACK_PAGES);
	kmemcpy(temp_kern, cur, STACK_SIZE);
		//tss_update((uintptr_t)temp_kern + STACK_SIZE);
	tss_update((uintptr_t)cur_esp - 8);
	
	printf("Entering user mode\n");
	//console_set_color(BLACK,WHITE);
	
	//TODO: 7/8/13 It appears the changes in this funtion
	//have made this call obsolete
	//triple faults in BOCHS, and pagefaults at 0x1000 below
	//PHYS_BASE without this pagedir_install
	pagedir_install(cur->pd);
	
	//use the previous offset we had before, but on the new userstack
	new_esp = (uintptr_t)(cur_esp & 0xfff) + (PHYS_BASE - 0x1000);
	
	asm volatile(
				"cli\n"
				"mov $0x23, %%ax\n"
				"mov %%ax, %%ds\n" 
				"mov %%ax, %%es\n" 
				"mov %%ax, %%fs\n" 
				"mov %%ax, %%gs\n" 
				"mov %0, %%eax\n"
				"push $0x23\n"
				"pushl %%eax\n"
				"push $0x200\n"
				"pushl $0x1b\n"
				"push $1f\n"
				"iret\n"
				"1:"	
				::
				"m"(new_esp)
				);
}


