#include <kernel/types.h>
#include <kernel/thread.h>
#include <kernel/interrupt.h>
#include <stdio.h>

void *kernel_stack_bottom;

thread_t *kernel_thread;

//thread_t *ready_list;
//thread_t *all_list;
//thread_t all_list;
LIST_HEAD(all_list);
#define PAGE_SIZE 0x1000
#define STACK_SIZE 0x4000
#define STACK_PAGES STACK_SIZE/PAGE_SIZE
extern uint32_t stack;



pid_t pid_allocate();

void thread_init()
{
	kernel_stack_bottom = (void *)&stack;
//	printf("stack bottom %x\n", kernel_stack_bottom);

	kernel_thread = kernel_stack_bottom;
	uint32_t esp;
	asm ("mov %%esp, %0": "=m"(esp) );
	
//	printf("esp %x\n",esp);// & ~0x3FFF);
	kernel_thread->pid = 0;
//	kernel_thread->sp = (uint8_t  *)esp;//(uint8_t *)kernel_thread + STACK_SIZE;
	

//	all_list = &kernel_thread->list;
//	kernel_thread->list = INIT_LIST_HEAD(&all_list.list);
//	LIST_HEAD(&(kernel_thread->list));
	list_add(&kernel_thread->list, &all_list);
//	INIT_LIST_HEAD(&all_list->list);
}

thread_t * thread_current()
{

	uint32_t _esp;
	asm ("mov %%esp, %0": "=m"(_esp) );
//	printf("esp %x\n",_esp);// & ~0x3FFF);

	thread_t *tmp = (thread_t *) (_esp & ~0x3FFF);

	return tmp;
}
void jump()
{
	printf("jumped here\n");
	while(1);

}
int test = 0;
thread_t *test_thread = 0;
void thread_scheduler(struct registers *regs)
{
	regs = regs;
	thread_t *cur = thread_current();//?
 cur = cur;
//	struct list_head *head = &cur->list;
	uint32_t _esp;
	asm ("mov %%esp, %0": "=r"(_esp) );
//	printf("esp sched %X\n",_esp);
	thread_t *next;
	struct list_head *pos = cur->list.next;
//	list_for_each_entry(next, all_list, list)
//	list_for_each(pos, &all_list)
//	{
		//later check if thread is ready
		next = list_entry(pos, thread_t, list);
//		if(next == cur) continue;	
	//	printf("pid %i\n", next->pid);
		if(test_thread == 0)
			goto aer;
		else
			next = test_thread;
//	}
////	if(next->pid == 0)// == kernel_thread)
//		goto aer; 
	//extern void intr_return();
	printf("PID= %i\n", next->pid);
	printf("Size %i\n",sizeof(registers_t));
//	_esp -= 76 + 8;//0x30 -i 4;
	printf("test %X\n", _esp + 0x30);
	//	_esp = (uint32_t)next->sp + 56;//56;
	_esp = (uint32_t)next->sp;
	printf("THREAD ESP %X\n",_esp);
	asm volatile ("mov %0, %%esp\n" :: "r"(_esp));
	asm volatile("cli");
	
	asm volatile("iret\n");
//	asm volatile ("jmp intr_return");
/*	asm volatile ("mov %0,%%esp\n"
					"jmp intr_return"
					: "=m"(next->sp)
				);
*/
aer:
	;
//	printf("switch\n");
//	context_switch()


}
pid_t fork()
{



	return 0;
}
void dummy()
{

	printf("hello from dummy\n");

}
struct tregs
{
   uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
} __attribute__((packed));


thread_t * thread_create(void (*func)(void *), void *aux)
{
//allocate pagest
	asm volatile ("cli");
	thread_t *new = pallocn(STACK_PAGES);
	printf("thread create\n");	
	
	if(new == NULL)
		return NULL;

	new->pd = pagedir_new();	
	printf("new %x\n", new);
	uint8_t *new_sp = (uint8_t *)(new + STACK_SIZE);
	new->pid = 1;//pid_allocate();
	new->parent = thread_current()->pid;
	void *new_eip = func;
	aux = aux;

//add new to list
	list_add(&new->list,&all_list);
	//list_add_tail(&(new->list),&all_list.list);
//set new to run
	//will set status to run here
//setup stack

//uint8_t *new_ksp = palloc()
//asm volatile (" " );
//	new_sp = new_sp - sizeof(registers_t);
	new_sp -= sizeof(struct tregs);	
//	struct registers *reg_frame = (struct registers *)new_sp;
	struct tregs *reg_frame= (struct tregs *)new_sp;
	reg_frame->eip = (uint32_t)new_eip;
	printf("NEW EIP %X\n",new_eip);
	printf("EIP %x\n",reg_frame->eip);
	reg_frame->useresp = (uint32_t)new_sp;
//	reg_frame->esp = (uint32_t)new_sp;
	reg_frame->cs = reg_frame->ss = 0x10;
test = 1;
	new->sp = new_sp;
	printf("new_esp = %x\n",new_sp);
	asm volatile("sti");
	test_thread = new;
	return new;
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
