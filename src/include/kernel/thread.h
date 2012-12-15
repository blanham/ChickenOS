#ifndef C_OS_THREAD_H
#define C_OS_THREAD_H
#include <kernel/list.h>
#include <kernel/interrupt.h>
#include <kernel/vm.h>
#include <fs/vfs.h>
#include <sys/signal.h>

#include <util/utlist.h>

#define THREAD_MAGIC 0xc001c0de

enum thread_stat {THREAD_DEAD, THREAD_READY, THREAD_RUNNING, THREAD_BLOCKED};

//FIXME: is this the right place for this? 
typedef int pid_t;
//TODO: Priorities? Would really like to maybe implement the 4.3BSD scheduler
// 		since we didn't get it working in pintos
typedef struct thread {
	
	pid_t pid;
	pid_t parent;
	pid_t pgid;
//TODO: will have to have list head for children
	char *name;

//	struct list_head list;
//	struct list_head ready_list;
//	struct list_head all_list;
	struct thread *ready_prev, *ready_next;
	struct thread *prev, *next;
	
	//fs stuff
	struct file *cur_dir;
	int fd;
//TODO: this needs to be a list or possibly a hash or something
	struct file *files[8];
	
	//saved kernel stack
	uint8_t *sp;
	
	//saved user stack
	uint8_t *usersp;
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
thread_t * thread_current();
//thread_t * thread_create(void (*func)(void *), void *aux);

//thread_t *thread_create(uint32_t eip, uint32_t esp);
void thread_yield();
void thread_exit();
void thread_usermode(void);
void thread_set_ready(thread_t *thread);

/* thread.c - system calls */
pid_t sys_fork(registers_t *regs);
pid_t sys_getpid();
pid_t sys_getpgrp();
int sys_kill(int pid, int sig);
int sys_sigaction(int sig, const struct sigaction *act, struct sigaction *oact);
int sys_sigsuspend(const sigset_t *mask);
int sys_execve(const char *path, char *const argv[], char *const envp[]);

//we define this so kmain can call the first user process 
int execv(const char *path, char *const argv[]);

/* thread/scheduler.c */
void thread_scheduler();

/* thread/load_elf.c */
#define ELF_MAGIC "\x7F""ELF"//why is this here?
int load_elf(const char *path, uintptr_t *eip);

#endif

