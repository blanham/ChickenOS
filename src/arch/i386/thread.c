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

void arch_threading_init()
{
	tss_init();
}

void switch_threads(thread_t *, thread_t *);
void arch_thread_reschedule(thread_t *cur, thread_t *next)
{
	(void)cur;
	(void)next;
	//this could be done with a straight assignment, instead of having
	//to call a function
	tss_update((uintptr_t)next + STACK_SIZE);

	printf("RES %p %p\n", cur, next);

	switch_threads(cur, next);
}

/*
void thread_reschedule(registers_t *regs, thread_t *cur, thread_t *next)
{
	extern void pic_send_end(int irq);
	uint32_t _esp = 0;
	cur->sp = (uint8_t *)regs->ESP;

	pagedir_install(next->mm->pd);
//	printf("dfaddfafds\n");
//	dump_regs((void *)_esp + 4);
	tss_update((uintptr_t)next + STACK_SIZE);

	if(next->sig_info->signal_pending != 0)
	{
	//registers_t *regs_bottom = (void *)next + STACK_SIZE - sizeof(registers_t);
	//	printf("NEXT %X sp %x eip %x usereip %x\n", next, next->sp, next->regs->eip, regs_bottom->eip);
		signal_do(next->regs, next);
	}
	_esp = (uint32_t)next->sp;
	//have to reset timer interrupt here
	pic_send_end(0);

	asm volatile (
		"mov %0,%%esp\n"
		"jmp intr_return"
		:: "r"(_esp)
	);
}
*/
void thread_copy_stackframe(thread_t *thread, void *stack, uintptr_t eax)
{
	registers_t *src, *dst;

	src = (void *)(((void *)thread + STACK_SIZE) - sizeof(*src));
	dst = (void *)((stack + STACK_SIZE) - sizeof(*dst));

	memcpy(dst, src, sizeof(*dst));

	dst->eax = eax;
}

void thread_build_signal_stackframe(void *stack, int sig_num, void * restore)
{
	uint32_t *s = stack;
	*s-- = sig_num;
	*s-- = (uintptr_t)restore;
	//I might switch to the linux/bsd method of actually copying a short bit of
	//code, written in assembly, to the user stack, though that also depends on
	//libc

}


extern uintptr_t THIS_IS_A_TEST;
//This should be interrupt_stackframe or something
void thread_build_stackframe(void * stack, uintptr_t eip, uintptr_t esp, uintptr_t eax)
{
	registers_t *reg_frame;

	reg_frame = (void *)((stack + STACK_SIZE) - sizeof(*reg_frame));

	reg_frame->eip = (uintptr_t)eip;
	reg_frame->ebp = esp;
	reg_frame->eax = eax;
	reg_frame->useresp = esp;
	reg_frame->cs = 0x1b;
	reg_frame->ds = reg_frame->es =  reg_frame->ss = 0x23;
	//Thread local storage here
	reg_frame->fs = reg_frame->gs =	0x23;
	reg_frame->eflags = 0x200;

	uint32_t *test = (void *)reg_frame;
	printf("TEST %x\n", &THIS_IS_A_TEST);
	*test = (uintptr_t)5;test--;
	*test = (uintptr_t)&THIS_IS_A_TEST;test--;
	*test = (uintptr_t)4;test--;
	*test = (uintptr_t)3;test--;
	*test = (uintptr_t)2;test--;
	*test = (uintptr_t)1;test--;

}

