// FIXME: actually use locks and semaphores 
#include <stdatomic.h>
#include <chicken/thread.h>

struct semaphore {
	atomic_uint count;
	thread_t *owner;
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
