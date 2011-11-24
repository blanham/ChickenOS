#include <kernel/types.h>
#include <kernel/thread.h>
#include <kernel/interrupt.h>
#include <stdio.h>

void *kernel_stack_bottom;

thread_t *kernel_thread;

//thread_t *ready_list;
//thread_t *all_list;
struct list_head *all_list;
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
	
	INIT_LIST_HEAD(all_list);
	list_add_tail(&kernel_thread->list, all_list);
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

void thread_scheduler()//struct registers *regs)
{
//	regs = regs;
	thread_t *cur = thread_current();//?
 cur = cur;
//	struct list_head *head = &cur->list;
	thread_t *next;
	list_for_each_entry(next, all_list, list)
	{
		//later check if thread is ready
		printf("pid %i\n", next->pid);


	} 


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
thread_t * thread_create(void (*func)(void), void *aux)
{
//allocate pagest
	thread_t *new = pallocn(STACK_PAGES);
	
	if(new == NULL)
		return NULL;

	new->pd = pagedir_new();	

	uint8_t *new_sp = (uint8_t *)(new + STACK_SIZE);
	new_sp = new_sp;
	
	new->pid= pid_allocate();

	void *new_eip = func;
	new_eip = new_eip;
	aux = aux;

//add new to list
	list_add_tail(&new->list,all_list);
//set new to run
	//will set status to run here
//setup stack

//uint8_t *new_ksp = palloc()
//asm volatile (" " );

	return new;
}

pid_t pid_allocate()
{
	static pid_t pid_count = 1;

	pid_count++;

	return pid_count;
	
}
/*

int sys_execv(const char *path, const char *arg[])
{






}*/
