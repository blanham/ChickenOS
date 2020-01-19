#include <common.h>
#include <stdint.h>
#include <chicken/thread.h>
#include <kernel/memory.h>
#include <kernel/interrupt.h>
#include <arch/i386/interrupt.h>
#include "i386_defs.h"

struct user_desc {
	uint32_t entry_number;
	uint64_t base_addr;
	uint32_t limit;
	uint32_t seg_32bit:1;
	uint32_t contents:2;
	uint32_t read_exec_only:1;
	uint32_t limit_in_pages:1;
	uint32_t seg_not_present:1;
	uint32_t useable:1;
	// x86-64
	//uint32_t lm:1;
};

void arch_thread_reschedule(thread_t *cur, thread_t *next)
{
	//this could be done with a straight assignment, instead of having to call a function
	tss_update((uintptr_t)next + STACK_SIZE);


	//PANIC("NOT IMPLEMENTED");

	if (next->tls != NULL) {
		struct user_desc *desc = next->tls;
		gdt_set_entry(6, desc->base_addr, desc->limit, GDTF_BOTH, GDTA_USER_DATA);
	} else {
		gdt_set_entry(6, 0x4000, 0xFFFFFFFF, GDTF_BOTH, GDTA_USER_DATA);

	}

	registers_t *regs = (void*)next+STACK_SIZE;
	regs--;
	//serial_printf("PID NEXT %i\n", next->pid);
	//dump_regs(regs);
	extern void switch_threads(thread_t *, thread_t *);
	switch_threads(cur, next);
}

void arch_thread_set_ip_and_usersp(uintptr_t ip, uintptr_t usersp)
{
	thread_t *cur = thread_current();
	// registers_t *regs = (void *)cur + STACK_SIZE - sizeof(*regs);
	registers_t *regs = cur->registers;
	regs->eip = ip;
	regs->useresp = usersp;
	regs->cs = 0x1b;
	regs->ds = regs->es = regs->fs = regs->ss = 0x23;
	regs->gs = 0x33; // TODO: This should be a different segment for thread local storage
}

void thread_set_ip_and_sp_kernel(uintptr_t ip, uintptr_t usersp)
{
	thread_t *cur = thread_current();
	// registers_t *regs = (void *)cur + STACK_SIZE - sizeof(*regs);
	registers_t *regs = cur->registers;
	regs->eip = ip;
	regs->useresp = usersp; // Is this needed?
	regs->cs = 0x1b;
	regs->ds = regs->es = regs->fs = regs->ss = 0x10;
	regs->gs = 0x10; // TODO: This should be a different segment for thread local storage
}

void print_desc(struct user_desc *desc) {
	printf("Entry: %i Base Addresss: %X Limit: %X\n", desc->entry_number, desc->base_addr, desc->limit);
	printf("32bit: %i Contents: %X  Read exec only: %i\n", desc->seg_32bit, desc->contents, desc->read_exec_only);
	printf("Limit in pages: %i Segment not preset: %i  useable: %i\n", desc->limit_in_pages, desc->seg_not_present, desc->useable);
}

void thread_dump_tls(void *tls)
{
	print_desc(tls);
}

void thread_set_tls(void *tls_descriptor)
{
	struct user_desc *desc = tls_descriptor;
	// FIXME: this is very basic to start
	// XXX: This does not handle flags
	gdt_set_entry(6, desc->base_addr, desc->limit, GDTF_BOTH, GDTA_USER_DATA);
}

int sys_get_thread_area(void *desc) {
	struct user_desc *desc2 = desc;
	printf("get thread area\n");
	print_desc(desc2);
	PANIC("NOT IMPLEMENTED");
	//desc->entry_number = 4;
	return 0;
}
// XXX: Not finished
int sys_set_thread_area(void *desc2) {

	struct user_desc *desc = kcalloc(sizeof(*desc), 1);
	memcpy(desc, desc2, sizeof(*desc));
	thread_current()->tls = desc;


	print_desc(desc);
	gdt_set_entry(6, (uint32_t)desc->base_addr, desc->limit, GDTF_BOTH, GDTA_USER_DATA);
	struct user_desc *user_desc = desc2;
	user_desc->entry_number = 6;

	extern void gdt_floosh();
	//gdt_floosh();
	return 0;
}
void thread_copy_stackframe(thread_t *thread, void *stack, uintptr_t eax)
{
	registers_t *src, *dst;

	src = (void *)(((void *)thread + STACK_SIZE) - sizeof(*src));
	dst = (void *)((stack + STACK_SIZE) - sizeof(*dst));

	memcpy(dst, src, sizeof(*dst));

	dst->eax = eax;

	//dump_regs()
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


(void)eip;
(void)esp;
	if (eax != 0) {
		//kmemcpy(reg_frame, (void *)eax, sizeof(*reg_frame));
		reg_frame->eax = 0;
		//reg_frame->eip = eip;
		//reg_frame->esp = 0;
		printf("Build that shit:\n");
		dump_regs(reg_frame);
		return;
	} else {
	reg_frame->eip = (uintptr_t)eip;
	reg_frame->ebp = esp;
	reg_frame->eax = 0;
	reg_frame->useresp = esp;
	if (eax != 0) {
		reg_frame->ebp = esp;
		reg_frame->useresp = eax;
	}
	reg_frame->cs = 0x1b;
	reg_frame->ds = reg_frame->es = reg_frame->fs = reg_frame->ss = 0x23;
	reg_frame->gs = 0x33; // TODO: This should be a different segment for thread local storage
	reg_frame->eflags = 0x200;
	}
	if (eax != 0) {
	//printf("\n");
	//printf("New:\n");
	//dump_regs(reg_frame);
	//printf("Old:\n");
	//dump_regs((void *)eax);
	//printf("\n");
	}
}