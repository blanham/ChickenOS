#ifndef C_OS_THREAD_H
#define C_OS_THREAD_H
#include <kernel/interrupt.h>
#include <mm/vm.h>
#include <mm/paging.h>
#include <fs/vfs.h>
#include <sys/signal.h>
#include <sys/types.h>

#define STACK_SIZE 0x1000
#define STACK_PAGES STACK_SIZE/PAGE_SIZE

#define MAX_THREADS 256
#define THREAD_MAGIC 0xc001c0de
#define THREAD_ASSERT(x) ASSERT(x->magic == THREAD_MAGIC, "Thread's kernel stack overflowed")

enum thread_stat {THREAD_DEAD, THREAD_READY, THREAD_RUNNING, THREAD_BLOCKED};

//TODO: Priorities? Would really like to maybe implement the 4.3BSD scheduler
// 		since we didn't get it working in pintos
typedef struct thread {
	
	pid_t pid;
	pid_t parent;
	pid_t pgid;

	char *name;

	//TODO: will have to have list head for children
	
	//fs stuff
	struct file *cur_dir;
	int fd;
	//TODO: Obviously we should be allowed to have more files open
	//Probably copy Linux and have a files struct pointer
	/*
	struct files {

		count
		close_on_exec (flag)
		open_fs
		struct files *fds[256];

	};


	*/
	struct file *files[8];
	
	//saved kernel stack
	uint8_t *sp;
	
	//saved user stack
	uint8_t *user;
	
	//location of brk
	void * brk;
	
	//signal stuff - need to add handlers - list?
	int signal_pending;
	struct sigaction **signals;
	
	//this pointer to registers on kernel
	//stack probably not needed	
	struct registers *regs;
	
	/* pagedirectory for this thread */
	pagedir_t pd;
	
	/* status of thread (DEAD,READY, RUNNING, BLOCKED) */
	enum thread_stat status;
	
	/* magic number so we can detect if stack collided with thread info */
	uint32_t magic;	
} thread_t;

/* thread.c */
void thread_init();
void thread_usermode(void);
pid_t pid_allocate();
thread_t * thread_current();
thread_t * thread_create(registers_t *regs ,uint32_t eip, uint32_t esp);

/* thread_ops.c - system calls */
pid_t sys_fork(registers_t *regs);
pid_t sys_getpid();
pid_t sys_getpgrp();
int sys_execve(const char *path, char *const argv[], char *const envp[]);
void *sys_brk(uintptr_t ptr);

//we define this so kmain can call the first user process 
int execv(const char *path, char *const argv[]);

/* thread/scheduler.c */
void thread_scheduler_init(thread_t *kernel_thread);
void thread_scheduler(registers_t *regs);
void thread_yield();
void thread_exit();
void thread_set_ready(thread_t *thread);
void thread_queue(thread_t *thread);
thread_t *thread_next();

/* thread/signal.c */
int sys_kill(int pid, int sig);
int sys_sigaction(int sig, const struct sigaction *act, struct sigaction *oact);
int sys_sigsuspend(const sigset_t *mask);

/* thread/load_elf.c */
#define ELF_MAGIC "\x7F""ELF"//why is this here?
int load_elf(const char *path, uintptr_t *eip);

#endif

