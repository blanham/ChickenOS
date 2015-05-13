

struct lock {
	unsigned locked;
	char *name;
};
typedef struct lock lock_t;
void acquire(lock_t *lock);
void try_acquire(lock_t *lock);
void release(lock_t *lock);
//spinlock_t
//spinlock_acquire
//while(test_set())
//	thread_yield()
//spinlock_release(lock)
//	sync_lock_relase(lock)


