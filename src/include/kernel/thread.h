#ifndef C_OS_THREAD_H
#define C_OS_THREAD_H
#include <kernel/list.h>
#include <kernel/vm.h>
typedef unsigned short pid_t;
typedef struct thread {
	pid_t pid;
	pid_t parent;
	struct list_head list;
	uint8_t *sp;
	pagedir_t pd;	
} thread_t;

void thread_init();
thread_t * thread_current();

thread_t * thread_create(void (*func)(void*), void *aux);
void thread_scheduler();
pid_t fork();
#endif

