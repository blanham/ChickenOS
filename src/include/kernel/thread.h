#ifndef C_OS_THREAD_H
#define C_OS_THREAD_H
#include <kernel/list.h>
#include <kernel/vm.h>
#include <kernel/fs/vfs.h>
enum thread_stat {DEAD, RUNNING};

typedef unsigned short pid_t;

typedef struct thread {
	pid_t pid;
	pid_t parent;
	struct list_head list;
	struct list_head all_list;
	struct file *cur_dir;
	uint8_t *sp;
	struct registers *regs;
	pagedir_t pd;
	enum thread_stat status;	
} thread_t;

void thread_init();
thread_t * thread_current();
thread_t * thread_create(void (*func)(void*), void *aux);
void thread_scheduler();
void thread_yield();
void thread_exit();
pid_t fork();
pid_t get_pid();
#endif

