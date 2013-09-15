/*	ChickenOS - thread.c
 *  Threading system
 *
 */
#include <common.h>
#include <stdint.h>
#include <mm/vm.h>
#include <mm/paging.h>
#include <kernel/thread.h>
#include <kernel/memory.h>
#include <kernel/interrupt.h>
#include <thread/tss.h>
#include <mm/liballoc.h>


struct sigaction *default_signals[32];
char *_main_thread_name = "main";


void thread_init()
{
	thread_t *kernel_thread;
	
	interrupt_disable();

	//do this inline, otherwise assertion fails in thread_current()	
	asm ("mov %%esp, %0": "=m"(kernel_thread) );
	kernel_thread = (thread_t *) ((uintptr_t)kernel_thread & ~(STACK_SIZE -1));
	
	kernel_thread->pid = 0;
	kernel_thread->parent = 0;
	kernel_thread->pgid = 0;

	kernel_thread->pd = pagedir_new();
	kernel_thread->signals = default_signals;

	//FIXME:
	kernel_thread->name = _main_thread_name;
	kernel_thread->magic = THREAD_MAGIC;

	thread_scheduler_init(kernel_thread);
}



//extern uint32_t get_eip();
extern void pagedir_insert_page(pagedir_t pd, 
	virt_addr_t phys, virt_addr_t virt,uint8_t flags);
void thread_usermode(void)
{
	uint32_t cur_esp,new_esp;
	thread_t *cur;
	void *userstack;

	interrupt_disable();

	asm volatile("mov %%esp, %0":"=m"(cur_esp));

	cur = thread_current();

	userstack = pallocn(STACK_PAGES);
	kmemcpy(userstack, cur, STACK_SIZE);

	pagedir_insert_pagen(cur->pd, (uintptr_t)userstack, 
			(uintptr_t)PHYS_BASE - STACK_SIZE, 0x7, STACK_PAGES);

	
	//puts new kernel stack in tss
	//FIXME? Since this is the kernel thread
	//we have important stuff on the stack
	//without the -8 offset we have a race condition
	//where, if an interrupt happens between the iret
	//below and returning, the stack is trashed
	dump_regs(cur->regs);
	void *temp_kern = pallocn(STACK_PAGES);
	kmemcpy(temp_kern, cur, STACK_SIZE);
	tss_update((uintptr_t)temp_kern + STACK_SIZE);
//	tss_update((uintptr_t)cur_esp - 8);
	printf("%X %X %X ASDFAFSD\n", cur->regs, temp_kern, cur);
	cur->sp = (void *)cur_esp - 8;
	cur->sp = (void *)temp_kern - 8;
	cur->regs = (void*)(((uintptr_t)temp_kern & ~0xFFF) | ((uintptr_t)cur->regs & 0xFFF));
	dump_regs(cur->regs);
	printf("%X %X %X ASDFAFSD\n", cur->regs, temp_kern, cur);
//	cur->regs = (void *)palloc();//0xdeadbeef;
	printf("Entering user mode\n");
	//console_set_color(BLACK,WHITE);
	
	//TODO: 7/8/13 It appears the changes in this funtion
	//have made this call obsolete
	//triple faults in BOCHS, and pagefaults at 0x1000 below
	//PHYS_BASE without this pagedir_install
	pagedir_install(cur->pd);
	
	//use the previous offset we had before, but on the new userstack
	new_esp = (uintptr_t)(cur_esp & 0xfff) + (PHYS_BASE - 0x1000);

	
	asm volatile(
				"cli\n"
				"mov $0x23, %%ax\n"
				"mov %%ax, %%ds\n" 
				"mov %%ax, %%es\n" 
				"mov %%ax, %%fs\n" 
				"mov %%ax, %%gs\n" 
				"mov %0, %%eax\n"
				"push $0x23\n"
				"pushl %%eax\n"
				"push $0x200\n"
				"pushl $0x1b\n"
				"push $1f\n"
				"iret\n"
				"1:"	
				::
				"m"(new_esp)
				);
}



thread_t *
thread_new()
{
	thread_t *new = pallocn(STACK_PAGES);
	kmemset(new, 0, STACK_SIZE);
	new->magic = THREAD_MAGIC;
	return new;
}


thread_t *
thread_clone(thread_t *cur)
{
	thread_t *new;
	
	new = thread_new();
	kmemcpy(new, cur, sizeof(thread_t));

	
	new->parent = cur->pid;
	new->pgid = cur->pgid;
	new->pid = pid_allocate();
	
	new->cur_dir = cur->cur_dir;

	//FIXME: Should be a vfs call that increases reference counts	
	kmemcpy(new->files, cur->files, sizeof(struct file)*8);
	printf("here\n");

	//TODO: Add constant for number of signals and use it here
	new->signals = (struct sigaction **)kmalloc(sizeof(struct sigaction*) * 32);
	kmemcpy(new->signals, cur->signals, sizeof(struct sigaction*) * 32);

	new->magic = THREAD_MAGIC;

	return new;
}

//FIXME: doesn't copy open files over to new process in the case of a fork
thread_t * 
thread_create(registers_t *regs ,uint32_t eip, uint32_t esp)
{
	thread_t *new, *cur;
	enum intr_status old_level;
	registers_t *reg_frame;
	uint8_t *kernel_stack, *user_stack;
	uintptr_t new_sp;
	return NULL;	
	old_level = interrupt_disable();
	
	cur = thread_current();
	new = thread_clone(cur);
	
	kernel_stack = (uint8_t *)new;
	
	//TODO: Perhaps we should allocate more than one page to the user stack?
	user_stack = pallocn(1);
	
	new_sp = (uintptr_t)kernel_stack + STACK_SIZE;
	//kmemset((uint32_t*)user_stack, 	0, PAGE_SIZE);
	
	printf("sizeof %i\n",sizeof(registers_t));
	new->pd = pagedir_clone(cur->pd);
	pagedir_insert_page(new->pd, (uintptr_t)user_stack, 
		(uintptr_t)PHYS_BASE - 0x1000, 0x7);

//	kmemcpy(user_stack, (void *)(PHYS_BASE - 0x1000), 0x1000);	

	reg_frame = (void *)((kernel_stack + STACK_SIZE) - sizeof(*reg_frame));
	new->regs = (struct registers *)reg_frame;
	
	if(regs != NULL)
	{
		kmemcpy(reg_frame, regs, sizeof(registers_t));		
	//	reg_frame->useresp = PHYS_BASE - 0x1000 + (regs->esp&0xfff);
		
	}
	else
	{
		kmemset(reg_frame, 0, sizeof(*reg_frame));
		//Build new register frame
		reg_frame->eip = eip;
		reg_frame->ebp = PHYS_BASE;	
		reg_frame->esp = PHYS_BASE;
		reg_frame->cs = 0x1b;
		reg_frame->ds = reg_frame->es = reg_frame->fs = 
		reg_frame->gs = reg_frame->ss = 0x23;
		reg_frame->eflags = 0x200;

		(void)eip;
		(void)esp;
	}	

	reg_frame->eax = 0;
	reg_frame->esp = new_sp - 14*4;	
	
	new->sp = (uint8_t *)(new_sp - (sizeof(registers_t) + 4));
	
	thread_set_ready(new);
	thread_queue(new);

	printf("finished thread_create\n");	

	interrupt_set(old_level);

	return new;	
}

pid_t pid_allocate()
{
	static pid_t pid_count = 0;
	
	pid_count++;

	return pid_count;
}

bool
in_kernel(void)
{
	uint16_t ss;	
	asm volatile ("mov %%ss, %0\n" : "=r"(ss));
	if((ss & 3) == 0)
		return true;

	return false;
}

thread_t * thread_current()
{
	thread_t *ret; 
	asm ("mov %%esp, %0": "=m"(ret) );

	ret = (thread_t *) ((uintptr_t)ret & ~(STACK_SIZE -1));
	
	THREAD_ASSERT(ret);

	return ret;
}


