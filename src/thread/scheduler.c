//*	ChickenOS - thread/scheduler.c
 /*	Very basic scheduler, called on every timer interrupt  
 */
#include <common.h>
#include <kernel/list.h>
#include <kernel/thread.h>
#include <thread/tss.h> 
#include <kernel/console.h>
#include <kernel/interrupt.h>
#include <kernel/vm.h>
#include <stdio.h>
void thread_scheduler(struct registers *regs)
{

	uint32_t _esp = 0xfeedface;
	asm volatile("xchg %bx, %bx");	
	thread_t *cur = thread_current();
	thread_t *next = list_entry(cur->list.next, thread_t, list);

	cur->sp = (uint8_t *)regs->ESP;
	_esp = (uint32_t)next->sp;

	tss_update((uintptr_t)next + 4096);	
	pagedir_install(next->pd);
	asm volatile(
					"mov %0,%%esp\n"
					"jmp intr_return"
					:: "r"(_esp)
				);
}

void new_scheduler()
{







}
