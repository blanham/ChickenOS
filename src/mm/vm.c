/*	ChickenOS - mm/vm.c
 *	Handles paging, gdt, and page allocation
 *  Heap allocation is implemented with liballoc
 *  which uses pages from the page allocator
 *  Uses code borrowed from JamesM's kernel tutorial
 *
 */
#include <common.h>
#include <kernel/vm.h>
#include <kernel/console.h>
#include <kernel/interrupt.h>
#include <stdio.h>

extern uint32_t end;

static void gpf(struct registers *regs);
static void page_fault(struct registers * regs);
static void stack_fault(struct registers * regs);

void vm_init(struct multiboot_info *mb)
{
	virt_addr_t placement = 0;
	uint32_t size = mb->mem_upper;
	
	if(mb->mods_count > 0 )
	{
		//TODO: doublecheck if this is correct
		placement =(uint32_t) P2V(*(void **)P2V(mb->mods_addr + 4));	
	}else{	
		placement = (unsigned)&end;
	}	

	//round up placement to nearest page	
	placement = (placement & PAGE_MASK) + PAGE_SIZE;
	//size = 1024 * 1024; //needed for machines with large amounts of memory at the moment
						//becuase we only have 4MB, and the bitmap overflows it 
	uint32_t page_count = (size/4) - (V2P(placement)/PAGE_SIZE);
	
//	printf("%iMB installed %i\n", (size)/1024 + 2);
	
	palloc_init(page_count, placement);
	
	paging_init();	
	
	interrupt_register(12, &stack_fault);
	interrupt_register(13, &gpf);
	interrupt_register(14, &page_fault);

}

static void gpf(struct registers *regs)
{
	uint32_t error_code = regs->err_code;
	console_set_color(LT_GREY,WHITE);
	printf("Error %x\n",error_code);
	dump_regs(regs);
	//triggers debugger in BOCHS
	asm volatile("xchg %bx, %bx");
	PANIC("GENERAL PROTECTION FAULT!");
}

static void stack_fault(struct registers * regs)
{
	uint32_t error_code = regs->err_code;
	printf("Error %x\n",error_code);
	dump_regs(regs);
	//triggers debugger in BOCHS
	asm volatile("xchg %bx, %bx");
	PANIC("STACK FAULT!");


}
static void page_fault(struct registers * regs)
{
	void *faulting_addr;
	uint32_t error_code = regs->err_code;

	bool is_user, is_write, not_present;	
		
	is_user = ((PAGE_USER & error_code) ? TRUE : FALSE);
	is_write = ((PAGE_WRITE & error_code) ? TRUE : FALSE);
	not_present = ((PAGE_VIOLATION & error_code) ? FALSE : TRUE);
	
	asm volatile ("mov %%cr2, %0"
				  : "=r" (faulting_addr)

				 );
	console_set_color(GREEN, WHITE);
	printf("\nPAGE FAULT! @ %x\n",faulting_addr);

	if(is_user)
		printf("user space\n");
	else
		printf("kernel space\n");

	if(is_write)
		printf("write\n");
	else
		printf("read\n");
	
	if(not_present)
		printf("page not present\n");
	else
		printf("protection violation\n");

	printf("\n");
	
	if(!is_user)
	{
		console_set_color(RED, WHITE);
		printf("REGS:\n");
		dump_regs(regs);
		printf("\n");
		PANIC("Page fault in kernel space!");
	}else {
		//kill process
		console_set_color(RED, WHITE);
		printf("REGS:\n");
		dump_regs(regs);
		printf("\n");
		PANIC("Page fault in user space!");

	}
}

