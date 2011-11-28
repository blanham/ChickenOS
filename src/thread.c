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
//	printf("stack bottom %x\n", kernel_stack_bottom);

	kernel_thread = kernel_stack_bottom;
	uint32_t esp;
	asm ("mov %%esp, %0": "=m"(esp) );
	
//	printf("esp %x\n",esp);// & ~0x3FFF);
	kernel_thread->pid = 0;
	kernel_thread->parent = 12345;
//	kernel_thread->sp = (uint8_t  *)esp;//(uint8_t *)kernel_thread + STACK_SIZE;
	
	kernel_thread->sp = (uint8_t *)0xDEADBEEF;
//	all_list = &kernel_thread->list;
//	INIT_LIST_HEAD(&kernel_thread->list);
	//LIST_HEAD(&(kernel_thread->list));
	list_add(&kernel_thread->list, &all_list);

	asm volatile( "sti");
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

int test2 = 0;

void thread_scheduler(struct registers *regs)
{


	regs = regs;
	uint32_t _esp = 0xdeadbeef;
	thread_t *cur = thread_current();
	thread_t *next = 0;
	uint32_t esp_old = 0xdeadbeef;

//	uint32_t TEST = 0;
//	TEST = TEST;
	//asm volatile ("mov %%esp, %0\n" : "=r"(esp_old));

thread_t *tt;
struct list_head *pos;
list_for_each(pos, &all_list)	
{
	tt = list_entry(pos, thread_t,list);
//	printf("PID &%X\n", tt->pid);
	if(tt->pid != cur->pid)
		break;

}
	if(cur != kernel_thread)
		tt = kernel_thread;
	else if(test)
		tt = test_thread;
//		next = list_entry(&cur->list.next, thread_t, list);
	//	printf("PID = %i\n",next->pid);
	

//	thread_t *tes = list_entry(&next->list.next, thread_t, list);
//	printf("%X %X %X\n", next, tes, cur);
	//next = tes;
	next = tt;
//		printf("PID = %i\n",next->pid);
//	if(cur->pid == 1)
	//	while(1);
//	printf("lulx next->sp %X\n", next->sp);

//	if(next->sp == 0)
//		next->sp = (uint8_t *)(0xc0110f4c + 13*4);
	esp_old = regs->esp - 0x24;
// 	printf("%x %x\n", esp_old, regs->esp - 0x24);
	cur->sp = (uint8_t *)(esp_old + 0);

	_esp = (uint32_t)next->sp;// + 56;
//	printf("%x %x\n", next->sp, cur->sp);
//	if(next != cur) while(1);
//	dump_regs((registers_t *)next->sp);
	//return;
//	printf("SCHED ESP %X ESPOLD %X\n",_esp, esp_old);
	
//	extern void pic_send_end(int);
//		pic_send_end(0x20);
	//if(test2 == 1)
	//while(1);
	//if(next->pid == 1)
	//	goto new;
//	asm volatile("sti");
	//if(next->pid == 1)
	//;//	while(1);
	asm volatile ("mov %0,%%esp\n"
					"jmp intr_return"
					:: "m"(_esp)
				);


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


	char *derp = "test";
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
	uint32_t new_sp = (uint32_t)(new + STACK_SIZE);
	new->pid = 1;//pid_allocate();
	new->parent = thread_current()->pid;
	void *new_eip = func;
	aux = derp;

//add new to list
	//list_add_tail(&(new->list),&all_list.list);
//set new to run
	//will set status to run here
//setup stack

//uint8_t *new_ksp = palloc()
//asm volatile (" " );
//	new_sp = new_sp - sizeof(registers_t);
	printf("NEW_SP %X\n", new_sp);
	uint32_t saved_sp = new_sp;	

	printf("INTR FRAME %X\n",new_sp - sizeof(struct registers));
	new_sp -= sizeof(struct registers);	
	printf("NEW_SP %X\n", new_sp);
//	struct registers *reg_frame = (struct registers *)new_sp;
	struct registers *reg_frame= (struct registers *)new_sp;
	reg_frame->eip = (uint32_t)new_eip;
	printf("NEW EIP %X\n",new_eip);
	printf("EIP %x\n",reg_frame->eip);
//	reg_frame->esp = (uint32_t)new_sp;
	uint32_t _cs, _ss;
		
	asm volatile ("mov %%cs, %0\n" : "=r"(_cs));
	asm volatile ("mov %%ss, %0\n" : "=r"(_ss));
	reg_frame->cs = _cs;
 	reg_frame->ss = _ss;
test = 1;
/*	new_sp = (uint8_t *)aux;
		

	new_sp--;
*/	printf("new_esp = %x\n",new_sp);
	
	reg_frame->useresp = (uint32_t)saved_sp;
	reg_frame->esp = (uint32_t)saved_sp;
	new->sp = (uint8_t *)new_sp - 4;
//	list_add_tail(&new->list,&kernel_thread->list);
	list_add_tail(&new->list,&all_list);
	//while(1);
	test_thread = new;
	asm volatile("sti");
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
