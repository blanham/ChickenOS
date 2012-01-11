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
	//fs stuff
	struct file *cur_dir;
	int fd;
	struct file *files[8];
	//saved kernel stack
	uint8_t *sp;
	//location of brk
	void * brk;
	//signal stuff
	int signal_pending;
	//this pointer to registers on kernel
	//stack probably not needed	
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

int sys_execve(const char *path, char *const argv[], char *const envp[]); 
int execv(const char *path, char *const argv[]);
/* thread/scheduler.c */
void thread_scheduler();
/* thread/load_elf.c */
#define ELF_MAGIC "\177ELF"
int load_elf(const char *path, uintptr_t *eip);

/* */
pid_t sys_fork(registers_t *regs);
pid_t sys_getpid();
int sys_kill(int pid, int sig);
#endif

