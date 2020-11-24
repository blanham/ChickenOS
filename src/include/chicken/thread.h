#ifndef C_OS_THREAD_H
#define C_OS_THREAD_H
#include <stdatomic.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <chicken/fs/vfs.h>
#include <chicken/fs/dentry.h>
#include <chicken/interrupt.h>
#include <chicken/mm/vm.h>
#include <chicken/mm/paging.h>
#include <chicken/thread/ksigaction.h>

#include <util/utlist.h>
#include <util/uthash.h>

#define STACK_SIZE 0x1000
#define STACK_MASK ~(STACK_SIZE-1)
#define STACK_PAGES (STACK_SIZE)/(PAGE_SIZE)

#define NUM_SIGNALS 32

#define MAX_THREADS 256
#define THREAD_MAGIC 0xc001c0de

#define THREAD_ASSERT(x) ASSERT(x->magic == THREAD_MAGIC, "Thread's kernel stack overflowed")

enum thread_stat;
struct thread_signals;
struct thread_files;

enum thread_stat {
	THREAD_NEW,
	THREAD_DEAD,
	THREAD_READY,
	THREAD_RUNNING,
	THREAD_BLOCKED,
	THREAD_UNINTERRUPTIBLE
};

typedef struct {
	int files_open;
	int files_count;
	struct file **files;
	int *files_flags;
	int ref;
} thread_files_t;

typedef struct {
	dentry_t *root;
	dentry_t *cur;
	mode_t umask;
	int ref;
} thread_fs_t;

typedef struct {
	int signal_pending;
	struct k_sigaction signals[NUM_SIGNALS];
	sigset_t sigmask, pending;
	int ref;
} thread_signals_t;

//TODO: Priorities? Would really like to maybe implement the 4.3BSD scheduler
// 		since we didn't get it working in pintos
typedef struct thread_struct thread_t;
typedef struct thread_struct {
	pid_t pid, tgid, ppid, pgid;
	// linux thread id stuff
	int *set_child_tid, *clear_child_tid;
	// TODO: We need to add tgid and return that instead of ->pid for getpid(2)
	uid_t uid, euid;
	gid_t gid, egid;
	mode_t umask;
	char *name;
	int ret_val; // this is the value passed into close()

	//Scheduler hash
	UT_hash_handle hh;

	//wait list
	
	//thread_t *wait_next, *wait_prev;
	//Would a tree work better for this?
	//Children and children list
	thread_t *children; // Why not have a pointer to a master tree of processes?
	thread_t *child_next, *child_prev;

	//fs stuff
	thread_files_t *files;
	thread_fs_t *fs_info;
	thread_signals_t *sig_info;

	//saved kernel stack
	uint8_t *sp;
	uintptr_t ip, usersp; // For fork() and clone()
	void *registers; // system calls and fork()/clone() need access to the interrupt context
	void *tls; // pointer to a architecture specific TLS description


	//this pointer to registers on kernel
	//stack is used for manipulating the user stack
	//for signal handling
	//struct registers *regs, *signal_regs;

	struct mm *mm;

	/* status of thread (DEAD, READY, RUNNING, BLOCKED, ) */
	enum thread_stat status;

	thread_t *next;

	/* magic number so we can detect if stack collided with thread info */
	uint32_t magic;
} __attribute__((packed)) thread_t;


// Possible design for a thread queue
struct thread_queue {
	thread_t *next, *prev;
};
typedef struct thread_queue thq_t;

//FIXME: split this stuff up?

/* arch/$ARCH/thread.c */
void arch_thread_reschedule(thread_t *cur, thread_t *next);
void arch_thread_set_ip_and_usersp(uintptr_t ip, uintptr_t usersp);
void thread_dump_tls(void *tls);

/* thread.c */
void 	threading_init();
thread_t *thread_new(bool copy_old);

/* thread_ops.c - system calls */
uid_t 	sys_geteuid();
pid_t 	sys_getpid();
pid_t 	sys_getppid();
pid_t 	sys_getpgrp();
int		sys_setuid(uid_t uid);
uid_t	sys_getuid();
gid_t	sys_getgid();
gid_t	sys_getegid();
int		sys_setgid(gid_t gid);
int 	sys_setpgid(pid_t pid, pid_t pgid);
int 	sys_brk(void *addr);
void *	sys_sbrk(intptr_t ptr);
void 	sys_exit(int return_code);
void 	sys_exit_group(int return_code);
pid_t 	sys_wait4(pid_t pid, int *status, int options, struct rusage *rusage);
int		sys_get_thread_area(void *desc);
int		sys_set_thread_area(void *desc);
int		sys_set_tid_address(int *ptr);

/* thread/clone.c */
pid_t 	sys_fork();
int		sys_clone(unsigned long flags, void *stack, int *parent_tid, unsigned long tls, int *child_tid);
pid_t	thread_start_init(void (*fn)(void *), void *aux);

/* thread/exec.c */
int	sys_execve(const char *path, char *const argv[], char *const envp[]);

/* thread/futex.c */
int sys_futex(int *uaddr, int futex_op, int val, const struct timespec *timeout, int *uaddr2, int val3);

/* thread/scheduler.c */
void scheduler_init(thread_t *kernel_thread);
void thread_scheduler(registers_t *regs);
void thread_yield();
void thread_exit(int exit);
void thread_set_ready(thread_t *thread);
void thread_queue(thread_t *thread);
void scheduler_run(registers_t *regs);
thread_t *thread_next();


/* thread/thread_util.c */
pid_t	pid_allocate();
pid_t	tgid_allocate();
thread_files_t *thread_files_alloc(thread_files_t *old); // Copies old to new if not NULL
thread_t * thread_current();
thread_t *thread_by_pid(pid_t pid);
void thread_add_child(thread_t *parent, thread_t *child); // FIXME: Make this a macro instead

/* arch/ARCH/thread.c */
void thread_copy_stackframe(thread_t *thread, void *stack, uintptr_t eax);
void thread_build_stackframe(void * stack, uintptr_t eip, uintptr_t esp, uintptr_t eax);
void thread_set_tls(thread_t *thread, void *tls_descriptor);
void thread_get_tls(void *tls_descriptor);

/* arch/ARCH/switch.s */
void switch_threads(thread_t *cur, thread_t *new);


// FIXME: not sure where to put this
struct uname;
int sys_uname(struct uname *uname);

typedef struct __locale_struct
{
  /* Note: LC_ALL is not a valid index into this array.  */
  void *__locales[13]; /* 13 = __LC_LAST. */
  /* To increase the speed of this solution we add some special members.  */
  const unsigned short int *__ctype_b;
  const int *__ctype_tolower;
  const int *__ctype_toupper;
  /* Note: LC_ALL is not a valid index into this array.  */
  const char *__names[13];
} locale_t;

struct pthread {
	/* Part 1 -- these fields may be external or
	 * internal (accessed via asm) ABI. Do not change. */
	struct pthread *self;
	uintptr_t *dtv;
	struct pthread *prev, *next; /* non-ABI */
	uintptr_t sysinfo;
	uintptr_t canary, canary2;

	/* Part 2 -- implementation details, non-ABI. */
	int tid;
	int errno_val;
	volatile int detach_state;
	volatile int cancel;
	volatile unsigned char canceldisable, cancelasync;
	unsigned char tsd_used:1;
	unsigned char dlerror_flag:1;
	unsigned char *map_base;
	size_t map_size;
	void *stack;
	size_t stack_size;
	size_t guard_size;
	void *result;
	struct __ptcb *cancelbuf;
	void **tsd;
	struct {
		volatile void *volatile head;
		long off;
		volatile void *volatile pending;
	} robust_list;
	volatile int timer_id;
	locale_t locale;
	volatile int killlock[1];
	char *dlerror_buf;
	void *stdio_locks;

	/* Part 3 -- the positions of these fields relative to
	 * the end of the structure is external and internal ABI. */
	uintptr_t canary_at_end;
	uintptr_t *dtv_copy;
};

#endif
