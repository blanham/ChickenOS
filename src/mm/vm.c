/*	ChickenOS - mm/vm.c
 *	Handles paging, gdt, and page allocation
 *  Heap allocation is implemented with liballoc
 *  which uses pages from the page allocator
 */
#include <common.h>
#include <mm/vm.h>
#include <kernel/hw.h>
#include <kernel/thread.h>
#include <stdio.h>

uint32_t mem_size;

void vm_page_fault_dump(registers_t *regs, uintptr_t addr, int flags)
{
	thread_t *cur = thread_current();
	printf("Page fault in %s space @ %X PID %i eip %x\n",
		(flags & PAGE_USER) ? "user" : "kernel",
		addr, cur->pid, regs->eip);
	printf("%s\t", (flags & PAGE_WRITE) ? "write" : "read");
	printf("%s\n", (flags & PAGE_VIOLATION) ? "protection violation" :
												"page not present");
	printf("\nREGS:\n");
	dump_regs(regs);
	printf("\n");
}

void vm_page_fault(registers_t *regs, uintptr_t addr, int flags)
{
	thread_t *cur = thread_current();
	enum intr_status status = interrupt_disable();

	//TODO: Check if this is a swapped out or mmaped or COW etc
	if(memregion_fault(cur->mm, addr, flags) == 0)
		return;

	if(flags & PAGE_USER)
	{
		vm_page_fault_dump(regs, addr, flags);

		//TODO: send sigsegv to thread
		//signal(cur, SIGSEGV);
		thread_current()->status = THREAD_DEAD;
		interrupt_set(status);

		//FIXME: should reschedule here instead of busy waiting
		//		I think the best bet is to return, and have checks to see
		//		if thread died before returning
		printf("Page fault in user space\n");
		while(1)
			;

		thread_yield();
		thread_exit(1);
	}
	else
	{
		vm_page_fault_dump(regs, addr, flags);
		PANIC("Page fault in kernel space!");
	}

	PANIC("Unhandled page fault");
}

struct mm *mm_alloc()
{
	struct mm *new = kcalloc(sizeof(*new), 1);

	new->pd = pagedir_new();

	return new;
}

void vm_init(struct kernel_boot_info *info)
{
	uint32_t page_count = info->mem_size/PAGE_SIZE;
	mem_size = info->mem_size;

	palloc_init(page_count, (uintptr_t)info->placement);
	paging_init(mem_size);
	frame_init(info->mem_size);
}
