/*	ChickenOS - thread/scheduler.c
 *	Very basic scheduler, called on every timer interrupt  
 */
#include <common.h>
#include <kernel/thread.h>
#include <thread/tss.h> 
#include <kernel/memory.h>
#include <device/console.h>
#include <kernel/interrupt.h>
#include <kernel/vm.h>
#include <thread/syscall.h>
#include <stdio.h>
extern void hex_dump(void *ptr, int n);
extern struct list_head all_list;

//This should be done differently:
//push the current eip on the stack
//push the signal number
//and then set the eip to the handler
//when the handler returns, it will return to eip
//thus resuming control flow 
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
extern bool thread_start;
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
extern void pic_send_end(int irq);
extern thread_t *kernel_thread;
//FIXME: FOR THE LOVE OF GOD FIXME

void thread_scheduler(registers_t *regs)
{
	(void)regs;
	uint32_t _esp = 0xfeedface;
	thread_t *cur, *next;
	next = NULL;
	cur = thread_current();
//	asm volatile ("mov %%esp, %0":"=m"(_esp));
//	printf("ESP %X\n",_esp);
//	cur->status = 0;
//	CDL_SEARCH_SCALAR(cur, next, status, THREAD_READY);
//	cur->status = THREAD_READY;
//	if(thread_start == false) return;
//	dump_regs(regs);
//	printf("ESP %X\n",regs->ESP);
	//return;
	next = cur->next;

	if(next == NULL)
		next = kernel_thread;//cur;
	if(next->status == THREAD_DEAD)
		next = next->next;
	if(next == NULL)
		next = kernel_thread;//cur;

//	if((uintptr_t)next < (uintptr_t)PHYS_BASE)
		//next = cur;
	//i believe that signals go here:
	//if(next->signal_pending != 0)
	//signal(regs, next);
	
	cur->sp = (uint8_t *)regs->ESP;
	cur->regs = regs;
	_esp = (uint32_t)next->sp;
	//dump_regs((registers_t *)(_esp + 4));

	tss_update((uintptr_t)next + 4096);	
	pagedir_install(next->pd);

	pic_send_end(0);

	asm volatile(
					"mov %0,%%esp\n"
					"jmp intr_return"
					:: "r"(_esp)
				);
}
