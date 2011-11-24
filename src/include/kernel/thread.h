#ifndef C_OS_THREAD_H
#define C_OS_THREAD_H
#include <kernel/list.h>
typedef unsigned short pid_t;
typedef struct thread {
	pid_t pid;
	pid_t parent;
	struct list_head list;

} thread_t;

void thread_init();
thread_t * thread_current();

void thread_scheduler();
pid_t fork();
#endif

