#ifndef C_OS_THREAD_H
#define C_OS_THREAD_H
#include <kernel/list.h>
#include <kernel/interrupt.h>
#include <kernel/vm.h>
#include <fs/vfs.h>
enum thread_stat {THREAD_DEAD, THREAD_READY, THREAD_RUNNING, THREAD_BLOCKED};
//is this the right place for this?
typedef unsigned short pid_t;

typedef struct thread {
	pid_t pid;
	pid_t parent;
	char *name;
	struct list_head list;
	struct list_head all_list;
	struct file *cur_dir;
	uint8_t *sp;
	struct registers *regs;
	pagedir_t pd;
	enum thread_stat status;
	uint32_t magic;	
} thread_t;

/* thread.c */
void thread_init();
thread_t * thread_current();
//thread_t * thread_create(void (*func)(void *), void *aux);
void thread_yield();
void thread_exit();
void thread_usermode(void);

/* thread/scheduler.c */
void thread_scheduler();

pid_t sys_fork(registers_t *regs);
pid_t sys_getpid();
#endif

