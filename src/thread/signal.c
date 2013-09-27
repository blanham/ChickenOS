/*	ChickenOS - thread/signal.c
 *	Thread level support of signals  
 */
#include <common.h>
#include <kernel/thread.h>
#include <thread/tss.h> 
#include <kernel/memory.h>
#include <device/console.h>
#include <kernel/interrupt.h>
#include <mm/vm.h>
#include <thread/syscall.h>
#include <stdio.h>
#include <mm/liballoc.h>

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
	(void)SYSCALL_0N(200);
}
extern bool thread_start;
void signal(registers_t *regs UNUSED, thread_t *next UNUSED)
{
/*	if(next->signal_pending > 0 && next->pid != 0)
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
	}*/
}

int sys_sigsuspend(const sigset_t *mask)
{
	(void) mask;
	//for(int i = 0; i < 1; i++)
	//	printf("Signal: %x\n",  *(uint32_t *)*mask);
	printf("Sigsuspend called, hanging\n");
	while(1);
	return 0;
}

int sys_sigaction(int sig, const struct sigaction *act, struct sigaction *oact)
{
	thread_t *cur = thread_current();
	
	if((oact != NULL) && (sig < 32))
	{
		if(cur->signals[sig] != NULL)
			kmemcpy(oact, cur->signals[sig], sizeof(*oact));
	}
	
	if((act != NULL) && (sig < 32))
	{
		if(cur->signals[sig] == NULL)
			cur->signals[sig] = kmalloc(sizeof(struct sigaction));

		kmemcpy(cur->signals[sig], (void *)act, sizeof(*oact));
		
	//	printf("mask %x %x %x\n", act->sa_flags, act->sa_mask, act->sa_handler);
	}
	 
	if((act == NULL) && (oact == NULL))
		return -1; 

	return 0;
}

int sys_kill(int pid, int sig)
{
//	thread_t * p = NULL;
	(void)pid;
	(void)sig;
	printf("Kill(): pid %i sig %i\n", pid, sig);
	//list_for_each_entry(p, &all_list, all_list)
	{
	//	if(p->pid == pid)
	//	{
	//		p->signal_pending = sig;
	//	}
	}
//	thread_yield();
	return 0;
}

