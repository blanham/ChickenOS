#include <kernel/types.h>
#include <kernel/thread.h>
#include <kernel/memory.h>
#include <kernel/interrupt.h>
#include "debug.h"

void *kernel_stack_bottom;
extern uint32_t stack;
thread_t *kernel_thread;

LIST_HEAD(all_list);

#define PAGE_SIZE 0x1000
#define STACK_SIZE 0x4000
#define STACK_PAGES STACK_SIZE/PAGE_SIZE

typedef struct {
	uint32_t old_tss;
	uint32_t esp0;
	uint32_t ss0;
	uint32_t esp1;
	uint32_t ss1;
	uint32_t esp2;
	uint32_t ss2;
	uint32_t cr3;
	uint32_t eip;
	uint32_t eflags;
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint32_t es;
	uint32_t cs;
	uint32_t ss;
	uint32_t ds;
	uint32_t fs;
	uint32_t gs;
	uint32_t ldt;
	uint32_t io_bmap;
} tss_t;

tss_t tss;

pid_t pid_allocate();

void thread_init()
{
	asm volatile("cli");
	kernel_stack_bottom = (void *)&stack;

	kernel_thread = thread_current();//kernel_stack_bottom;
	
	kernel_thread->pid = 0;
	kernel_thread->parent = 0;
	kernel_thread->sp = (uint8_t *)0xDEADBEEF;

	kernel_thread->status = RUNNING;

	INIT_LIST_HEAD(&kernel_thread->list);

}

thread_t * thread_current()
{

	thread_t *tmp; 
	asm ("mov %%esp, %0": "=m"(tmp) );

	return (thread_t *) ((uint32_t)tmp & ~0x3FFF);

}

void thread_scheduler(struct registers *regs)
{
	uint32_t _esp = 0xdeadbeef;
	thread_t *cur = thread_current();
	thread_t *next = list_entry(cur->list.next, thread_t, list);
	
	cur->sp = (uint8_t *)regs->ESP;
	_esp = (uint32_t)next->sp;
	
	asm volatile ("mov %0,%%esp\n"
					"jmp intr_return"
					:: "m"(_esp)
				);
}

/* throw an int 32, manually invoking the timer interrupt */
void thread_yield()
{
	asm volatile("int $32");
}


void thread_exit()
{
	asm volatile("cli");
	thread_t *cur = thread_current();
	//keep a tmp pointer to next process
	//which the scheduler uses to get the next process
	struct list_head *tmp = cur->list.next;
	list_del(&cur->list);
	cur->list.next = tmp;
	//if we aren't the original kernel thread, free the stack
	if(cur->pid == 0)
	{
		pallocn_free(cur, STACK_PAGES);	

	}
	asm volatile("sti");
	thread_yield();

}
extern uint32_t get_eip();
/* need to clone page directory properly first */
pid_t fork()
{
	return -1;
	/* this fixe interrupt in created threads, needs to be fixed */
/*	uint32_t _eflags;
	asm volatile ("pushfl\n" "pop %0\n" : "=r"(_eflags)::"esp");
	
	
	asm volatile ("cli");
	
	thread_t *new = pallocn(STACK_PAGES);
	thread_t *cur = thread_current();
//	uint32_t *args;
	uint32_t new_sp, cur_sp;
	uint32_t new_bp = 0, cur_bp;
	uint32_t top_sp;
	struct registers *reg_frame;
	int i = 1;
	while((int)new % STACK_SIZE)
	{
		
		pallocn_free(new, STACK_PAGES);
		new = pallocn(i);
		i++;	
	}
	
	kmemset((uint8_t *)new, 0, STACK_SIZE);

	//printf("thread create\n");	

	if(new == NULL)
		return -1;

	new->pd = pagedir_new();	
	
	new_sp = (uint32_t)((uint32_t)new + STACK_SIZE);

	kmemcpy((uint8_t *)new,(uint8_t*)cur, STACK_SIZE);

	//args = (uint32_t *)(new_sp - 4);
	
	asm ("mov %%esp, %0": "=m"(cur_sp));
	asm ("mov %%ebp, %0": "=m"(cur_bp));
	top_sp = ((uint32_t)cur_sp & (uint32_t)0x3FFF);// + STACK_SIZE;
	uint32_t offset = top_sp;// - cur_sp;
	new_bp = (cur_bp & 0x3fff);

	uint32_t eip = get_eip();
	printf("THREAD %X\n", thread_current());
	if(thread_current() == cur)
	{
		new->pid = pid_allocate();
		new->parent = cur->pid;

		new_sp -= offset - 64;
		uint32_t tmp = new_sp;
		new_bp = new_sp - new_bp;
		printf("%X %X %X %X\n",tmp, new_sp, cur_sp, cur_bp);
		new_sp -= sizeof(struct registers);	
		reg_frame= (struct registers *)new_sp;
		new->regs = (struct registers *)new_sp;
		reg_frame->eip = (uint32_t)eip;
	
		uint32_t _cs, _ss, _ds;
		asm volatile ("mov %%cs, %0\n" : "=r"(_cs));
		asm volatile ("mov %%ds, %0\n" : "=r"(_ds));
		asm volatile ("mov %%ss, %0\n" : "=r"(_ss));
		reg_frame->cs = _cs;
 		reg_frame->ss = _ss;
 		reg_frame->ds = _ds;
		reg_frame->eflags = _eflags;
	
		reg_frame->esp = (uint32_t)tmp;

		reg_frame->ebp = (uint32_t)new_bp;	
		reg_frame->useresp = (uint32_t)tmp;

		new->sp = (uint8_t *)new_sp - 4;

		list_add_tail(&new->all_list,&all_list);
		list_add_tail(&new->list,&kernel_thread->list);
		printf("DUMP\n");
		dump_regs(reg_frame);
		
		printf("HERE\n");	
		asm volatile("sti");
		return new->pid;
	}else{
	//	PANIC("FORKED\n");

	//	while(1);
		return 0;
	}
	// this has to be set here, otherwise trashed 
	// *args = (uint32_t)aux;
	
	//asm volatile("sti");
	
	return 0;*/
	
}

thread_t * thread_create(void (*func)(void *), void *aux)
{
	/* this fixes interrupt in created threads, needs to be fixed */
	uint32_t _eflags;
	asm volatile ("pushfl\n" "pop %0\n" : "=r"(_eflags)::"esp");
	
	asm volatile ("cli");
	
	thread_t *new = pallocn(STACK_PAGES);
	uint32_t *args;
	uint32_t new_sp;
	struct registers *reg_frame;
	/* need a function to return aligned palloc requests */
	int i = 1;
	while((int)new % STACK_SIZE)
	{
		
		pallocn_free(new, STACK_PAGES);
		new = pallocn(i);
		i++;	
	}
	
	kmemset((uint8_t *)new, 0, STACK_SIZE);

	if(new == NULL)
		return NULL;

	new->pd = pagedir_new();	
	
	new_sp = (uint32_t)((uint32_t)new + STACK_SIZE);
	args = (uint32_t *)(new_sp - 4);
	
	new->pid = pid_allocate();
	new->parent = thread_current()->pid;
	
	new_sp -= sizeof(struct registers);	
	reg_frame= (struct registers *)new_sp;
	new->regs = (struct registers *)new_sp;
	reg_frame->eip = (uint32_t)func;
	
	uint32_t _cs, _ss, _ds;
	asm volatile ("mov %%cs, %0\n" : "=r"(_cs));
	asm volatile ("mov %%ds, %0\n" : "=r"(_ds));
	asm volatile ("mov %%ss, %0\n" : "=r"(_ss));
	reg_frame->cs = _cs;
 	reg_frame->ss = _ss;
 	reg_frame->ds = _ds;
	reg_frame->eflags = _eflags;
	
	
	reg_frame->useresp = (uint32_t)new_sp;

	new->sp = (uint8_t *)new_sp - 4;

	list_add_tail(&new->all_list,&all_list);
	list_add_tail(&new->list,&kernel_thread->list);

	/* this has to be set here, otherwise trashed */
	*args = (uint32_t)aux;
	
	asm volatile("sti");
	
	return new;
}

pid_t get_pid()
{
	return thread_current()->pid;
}

pid_t pid_allocate()
{
	static pid_t pid_count = 0;

	pid_count++;

	return pid_count;
}
/*

int sys_execv(const char *path, const char *arg[])
{






}*/
