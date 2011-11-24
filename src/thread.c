#include <kernel/types.h>
#include <kernel/thread.h>
#include <stdio.h>

void *kernel_stack_bottom;

thread_t *kernel_thread;

extern uint32_t stack;
void thread_init()
{
	kernel_stack_bottom = (void *)&stack;
//	printf("stack bottom %x\n", kernel_stack_bottom);

	kernel_thread = kernel_stack_bottom;
	uint32_t esp = 0;
	asm ("mov %%esp, %0": "=m"(esp) );
//	printf("esp %x\n",esp);// & ~0x3FFF);
	kernel_thread->pid = 1;
	INIT_LIST_HEAD(&kernel_thread->list);

}

thread_t * thread_current()
{

	uint32_t _esp = 0;
	asm ("mov %%esp, %0": "=m"(_esp) );
//	printf("esp %x\n",_esp);// & ~0x3FFF);

	thread_t *tmp = (thread_t *) (_esp & ~0x3FFF);

	return tmp;
}

void thread_scheduler()
{

	printf("switch\n");



}
pid_t fork()
{



	return 0;
}
/*


int sys_execv(const char *path, const char *arg[])
{






}*/
