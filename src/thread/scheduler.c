//*	ChickenOS - thread/scheduler.c
 /*	Very basic scheduler, called on every timer interrupt  
 */
#include <common.h>
#include <kernel/list.h>
#include <kernel/thread.h>
#include <thread/tss.h> 
#include <kernel/memory.h>
#include <kernel/console.h>
#include <kernel/interrupt.h>
#include <kernel/vm.h>
#include <thread/syscall.h>
#include <stdio.h>
extern void hex_dump(void *ptr, int n);

void derp_signal(int test)
{
	static int i = 0;
	uint32_t esp;
	asm volatile("mov %%esp, %0":"=m"(esp));
	printf("derp signal %i %X %X\n",test, i++, esp);
	

}
void test_signals()
{

	printf("test_signals!\n");
}
void return_from_signal()//struct registers *regs)
{
//	static int i = 0;
//	i++;
//	if(i == 2)
	//	while(1);
//	printf("trying to return!\n");
	thread_current()->signal_pending = -1;
	SYSCALL_0N(200);
}
void thread_scheduler(struct registers *regs)
{

	uint32_t _esp = 0xfeedface;
	thread_t *cur, *next;

	cur = thread_current();
	next = list_entry(cur->list.next, thread_t, list);

/*	while(next->status == THREAD_BLOCKED)
	{
		next = list_entry(next->list.next, thread_t, list);
	}*/
	if(next->signal_pending != 0 && next->pid != 0)
	{
		//	printf("signal %i pending\n",next->signal_pending);	
	}

	cur->sp = (uint8_t *)regs->ESP;
//	cur->regs = regs;
	_esp = (uint32_t)next->sp;

	tss_update((uintptr_t)next + 4096);	
	pagedir_install(next->pd);
	if(next->signal_pending > 0 && next->pid != 0)
	{
		uint8_t *new_useresp = (uint8_t *)next->regs->useresp;
	//	printf("top %X\n",*(uint32_t *)new_useresp);
		next->usersp = (void *)new_useresp;

	//	printf("new_useresp %X\n",new_useresp);

		new_useresp -= sizeof(*regs);

		kmemcpy(new_useresp, (void *)next->regs, sizeof(*regs));
		uint32_t *test = (uint32_t *)new_useresp;
	//	dump_regs(next->regs);
	//	dump_regs((void *)new_useresp);
		*test-- = next->signal_pending;
		*test =(uintptr_t)return_from_signal;
		next->regs->eip = (uintptr_t)derp_signal;
		next->regs->useresp = (uintptr_t)test;
		next->signal_pending = -2;
	}else if(next->signal_pending == -2)
	{
		registers_t *r = (void *)next->usersp - 4;
		r--;
		kmemcpy((void *)_esp, r, sizeof(*regs));
		
		next->regs->useresp = (uintptr_t)next->usersp; 
	}
	asm volatile(
					"mov %0,%%esp\n"
					"jmp intr_return"
					:: "r"(_esp)
				);
}

void new_scheduler()
{







}
