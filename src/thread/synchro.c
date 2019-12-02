// FIXME: actually use locks and semaphores 
#include <chicken/thread.h>

struct semaphore {
	int count; //this should only be accessed with atomic operations
	thread_t *waiters;
};

int semaphore_down(struct semaphore *sema) {
	(void)sema;
	return 0;
}

int semaphore_try(struct semaphore *sema) {
	(void)sema;
	return 0;
}

int semaphore_up(struct semaphore *sema) {
	(void)sema;
	return 0;
}
/*

lock init
lock acquire
lock release

semaphore init
semaphore up
semaphore down

conditions

probably define memory barrier as well

wait queues

















*/
