#ifndef C_OS_THREAD_H
#define C_OS_THREAD_H
typedef unsigned short pid_t;
typedef struct thread {
	pid_t pid;


} thread_t;


void thread_init();
thread_t * thread_current();
#endif

