//*	ChickenOS - thread/scheduler.c
 /*	Very basic scheduler, called on every timer interrupt  
 */
#include <common.h>
#include <kernel/thread.h>
#include <thread/tss.h> 
#include <kernel/memory.h>
#include <kernel/console.h>
#include <kernel/interrupt.h>
#include <kernel/vm.h>
#include <thread/syscall.h>
#include <stdio.h>
extern void hex_dump(void *ptr, int n);
extern struct list_head all_list;
void dummy_signal(int test)
{
	static int i = 0;
	uint32_t esp;
	asm volatile("mov %%esp, %0":"=m"(esp));
	printf("dummy signal %i %X %X\n",test, i++, esp);
}
void return_from_signal()
{
	thread_current()->signal_pending = -1;
	SYSCALL_0N(200);
}

void signal(registers_t *regs, thread_t *next)
{
	if(next->signal_pending > 0 && next->pid != 0)
	{
		uint8_t *new_useresp = (uint8_t *)next->regs->useresp;
		next->usersp = (void *)new_useresp;
		new_useresp -= sizeof(*regs);

		kmemcpy(new_useresp, (void *)next->regs, sizeof(*regs));
		uint32_t *test = (uint32_t *)new_useresp;
		
		*test-- = next->signal_pending;
		*test =(uintptr_t)return_from_signal;
		
		next->regs->eip = (uintptr_t)dummy_signal;
		next->regs->useresp = (uintptr_t)test;
		next->signal_pending = -2;
	}else if(next->signal_pending == -2)
	{
		registers_t *r = (void *)next->usersp - 4;
		r--;
		kmemcpy((void *)next->sp, r, sizeof(*regs));
		
		next->regs->useresp = (uintptr_t)next->usersp; 
	}
}
	extern uint32_t GUARD;
void thread_scheduler(registers_t *regs)
{
	uint32_t _esp = 0xfeedface;
	thread_t *cur, *next;
	//printf("asfasdf\n");
	return;
	next = NULL;
	cur = thread_current();
	CDL_SEARCH_SCALAR(cur, next, status, THREAD_READY);

	next = cur->next;
	if((uintptr_t)next < (uintptr_t)PHYS_BASE)
		next = cur;
	//i believe that signals go here:
	//if(next->signal_pending != 0)
	//signal(regs, next);
	//printf("next pid %i\n",next->pid);
//	dump_regs(regs);
//	printf("%X %X %X %X %X\n", cur->next, cur->sp, regs->ESP, regs, next->sp);
	cur->sp = (uint8_t *)regs->ESP;
	cur->regs = regs;
	_esp = (uint32_t)next->sp;
//	printf("next %i\n",next->pid);
	//printf("%p\n",next + 4096);
	tss_update((uintptr_t)next + 4096);	
	pagedir_install(next->pd);

	asm volatile(
					"mov %0,%%esp\n"
					"jmp intr_return"
					:: "r"(_esp)
				);
}
