#include <common.h>
#include <stdint.h>
#include <chicken/thread.h>
#include <kernel/memory.h>
#include <kernel/interrupt.h>
#include <arch/i386/interrupt.h>
#include "i386_defs.h"

void arch_thread_reschedule(thread_t *cur, thread_t *next)
{
	//this could be done with a straight assignment, instead of having to call a function
	tss_update((uintptr_t)next + STACK_SIZE);

	extern void switch_threads(thread_t *, thread_t *);
	switch_threads(cur, next);
}

void arch_thread_set_ip_and_usersp(uintptr_t ip, uintptr_t usersp)
{
	thread_t *cur = thread_current();
	registers_t *regs = (void *)cur + STACK_SIZE - sizeof(*regs);
	regs->eip = ip;
	regs->useresp = usersp;
}

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

void thread_build_stackframe(void *sp, uintptr_t eip, uintptr_t esp, uintptr_t eax)
{
	uint32_t *stack = (void *)((sp + STACK_SIZE) - sizeof(registers_t) - sizeof(int));

	extern uintptr_t intr_return; // Defined in intr-core.s
	*stack++ = (uint32_t)&intr_return;

	registers_t *reg_frame = (registers_t*)stack;
	reg_frame->eip = (uintptr_t)eip;
	reg_frame->ebp = esp;
	reg_frame->eax = eax;
	reg_frame->useresp = esp;
	reg_frame->cs = 0x1b;
	reg_frame->ds = reg_frame->es = reg_frame->fs = reg_frame->ss = 0x23;
	reg_frame->gs = 0x23; // TODO: This should be a different segment for thread local storage
	reg_frame->eflags = 0x200;
}