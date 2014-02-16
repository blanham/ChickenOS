#ifndef C_OS_THREAD_H
#define C_OS_THREAD_H
#include <kernel/interrupt.h>
#include <mm/vm.h>
#include <mm/paging.h>
#include <util/utlist.h>
#include <fs/vfs.h>
#include <signal.h>
#include <sys/types.h>
#include <thread/ksigaction.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <util/uthash.h>

#define STACK_SIZE 0x1000
#define STACK_PAGES (STACK_SIZE)/(PAGE_SIZE)

#define NUM_SIGNALS 32

#define MAX_THREADS 256
#define THREAD_MAGIC 0xc001c0de

#define THREAD_ASSERT(x) ASSERT(x->magic == THREAD_MAGIC, "Thread's kernel stack overflowed")

enum thread_stat {
	THREAD_DEAD,
	THREAD_READY,
	THREAD_RUNNING,
	THREAD_BLOCKED,
	THREAD_UNINTERRUPTIBLE
};

typedef struct thread thread_t;

struct thread_files {
	struct inode *root;
	struct inode *cur;
	int files_open;
	int files_count;
	struct file **files;
	int *files_flags;
};	

struct thread_signals {
	int signal_pending;
	struct k_sigaction signals[NUM_SIGNALS];
	sigset_t sigmask, pending;
};

//TODO: Priorities? Would really like to maybe implement the 4.3BSD scheduler
// 		since we didn't get it working in pintos
struct thread {
	pid_t pid, ppid, pgid;
	uid_t uid, euid;
	gid_t gid;
	mode_t umask;
	char *name;

	//Scheduler hash
	UT_hash_handle hh;

	//wait list
	thread_t *wait_next, *wait_prev;
	//Would a tree work better for this?
	//Children and children list	
	thread_t *children;
	thread_t *child_next, *child_prev;

	//fs stuff
	struct thread_files *file_info;
	//saved kernel stack, user stack, and location of user stack
	uint8_t *sp, *useresp, *user;
	
	//location of brk
	void * brk;
	uintptr_t sbrk;

	//FIXME: Refactor this into a struct, takes up a fuckton of space
	struct thread_signals *sig_info;
	
	//this pointer to registers on kernel
	//stack is used for manipulating the user stack
	//for signal handling	
	struct registers *regs, *signal_regs;
	
	/* pagedirectory for this thread */
	pagedir_t pd;
	/*
		struct memory {
			pagedir_t pd;
			//memory region treet
			tree_t memoory_regions;

		};
	*/
/*	struct memory_region {
		uintptr_t start_addr;
		uintptr_t end_addr;
		int flags;
		union {
			struct file *file;
		};
	};*/
	
	/* status of thread (DEAD, READY, RUNNING, BLOCKED, ) */
	enum thread_stat status;
	
	/* magic number so we can detect if stack collided with thread info */
	uint32_t magic;	
} __attribute__((packed));

//FIXME: split this stuff up?

/* arch/$ARCH/thread.c */
void arch_thread_init();

/* thread.c */
void 		thread_init();
void 		thread_usermode(void);
thread_t * 	thread_current();
thread_t *	thread_by_pid(pid_t pid);
pid_t 		thread_create(registers_t *regs, void (*eip)(void *), void * esp);
pid_t 		pid_allocate();

/* thread_ops.c - system calls */
pid_t 	sys_fork(registers_t *regs);
uid_t 	sys_geteuid();
pid_t 	sys_getpid();
pid_t 	sys_getppid();
pid_t 	sys_getpgrp();
int sys_setuid(uid_t uid);
uid_t sys_getuid();
gid_t sys_getgid();
int sys_setgid(gid_t gid);
int 	sys_setpgid(pid_t pid, pid_t pgid);
int 	sys_execve(const char *path, char *const argv[], char *const envp[]);
int 	sys_brk(void *addr);
void *	sys_sbrk(intptr_t ptr);
void 	sys_exit(int exit);
pid_t 	sys_wait4(pid_t pid, int *status, int options, struct rusage *rusage);

//we define this so kmain can call the first user process 
int execv(const char *path, char *const argv[]);

/* thread/scheduler.c */
void thread_scheduler_init(thread_t *kernel_thread);
void thread_scheduler(registers_t *regs);
void thread_yield();
void thread_exit(int exit);
void thread_set_ready(thread_t *thread);
void thread_queue(thread_t *thread);
thread_t *thread_next();

/* thread/signal.c */
void signal_do(registers_t *regs, thread_t *);
int sys_kill(int pid, int sig);
int sigisemptyset(const sigset_t *set);
int sys_sigaction(int sig, const struct k_sigaction *act, struct k_sigaction *oact);
int sys_sigsuspend(const sigset_t *mask);
int sys_sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
int sys_sigreturn(registers_t *regs, unsigned long dunno);

/* thread/load_elf.c */
int load_elf(const char *path, uintptr_t *eip);
bool elf_check_magic(void *magic); 
#endif

