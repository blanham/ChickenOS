#include <errno.h>
#include <sched.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
//#include <arch/i386/interrupt.h> // FIXME this import should be made redundant
#include <chicken/common.h>
#include <chicken/interrupt.h>
#include <chicken/mm/vm.h>
#include <chicken/mm/regions.h>
#include <chicken/thread.h>

void clone_flags_dump(uint32_t flags);

static void init_exited(void) __attribute__((section(".user")));
static void init_exited(void)
{
	PANIC("init() exited!");
}

// This function manually builds a new thread so we can get to userspace
pid_t thread_start_init(void (*fn)(void *), void *aux)
{
	enum intr_status old_level = interrupt_disable();

	//printf("Creating thread with eip: %x _esp: %x\n", eip, _esp);

	thread_t *new = thread_new(false);
	new->mm = mm_alloc();
	new->files = thread_files_alloc(NULL);
	new->tgid = 1;

	thread_t *cur = thread_current();
	new->fs_info = kcalloc(sizeof(*new->fs_info), 1);
	new->fs_info->cur = cur->fs_info->cur;
	new->fs_info->root = cur->fs_info->root;

	uint32_t *usersp = palloc();
	memregion_map_data(new->mm, PHYS_BASE - PAGE_SIZE, PAGE_SIZE, PROT_GROWSDOWN, MAP_GROWSDOWN | MAP_FIXED, usersp);

	usersp += 1024;
	*--usersp = (uintptr_t)init_exited;
	*--usersp = (uintptr_t)aux;

	uintptr_t esp = PHYS_BASE - 8;
	thread_build_stackframe((void *)new, (uintptr_t)fn, esp, 0);

	//printf("ESP %X USERSP %X\n", esp, usersp);

	uint8_t *new_sp = (void*)((uintptr_t)new + STACK_SIZE);
	new->sp = (uint8_t *)(new_sp - (sizeof(registers_t) + sizeof(uint32_t)*5));

	//registers_dump((void *)new->sp + (5*4));

	thread_set_ready(new);

	interrupt_set(old_level);

	return new->pid;
}

thread_t *thread_clone_new(unsigned long flags, void *stack, uintptr_t tls)
{
	thread_t *parent = thread_current();
	thread_t *new = thread_new(true);

	if (flags & CLONE_FILES) {
		new->files = parent->files;
		new->files->ref++;
	} else {
		new->files = thread_files_alloc(parent->files);
		new->files->ref++;
	}

	if (flags & CLONE_FS) {
		new->fs_info = parent->fs_info;
		new->fs_info->ref++;
	} else {
		new->fs_info = kcalloc(sizeof(*new->fs_info),1);
		new->fs_info->cur = parent->fs_info->cur;
		new->fs_info->root = parent->fs_info->root;
		new->fs_info->ref++;
	}

	if (flags & CLONE_PARENT) {
		new->ppid = parent->ppid;
	} else {
		new->ppid = parent->pid;
	}

	if (flags & CLONE_SETTLS) {
		if (verify_pointer((void *)tls, -1, VP_READ))
			//return -EFAULT;
			PANIC("BAD POINTER");
		thread_set_tls(new, (void *)tls);
	} else {
		// XXX: Do we need to do something here?
	}

	if (flags & CLONE_SIGHAND) {
		new->sig_info = parent->sig_info;
		new->sig_info->ref++;
	} else {
		memcpy(new->sig_info->signals, parent->sig_info->signals, sizeof(struct k_sigaction) * NUM_SIGNALS);
	}

	if (flags & CLONE_SYSVSEM) {
		// XXX: Implement this when i have SysV semaphores
	}

	if (flags & CLONE_VFORK) {
		// XXX: block parent until child exits or execs()
	}

	if (flags & CLONE_THREAD) {
		new->tgid = parent->tgid;
	} else {
		// put in new thread group
		new->tgid = new->pid;
	}

	// NOTE: is there a race here?
	if (flags & CLONE_VM) {
		new->mm = parent->mm;
		new->mm->ref++;
	} else {
		new->mm = mm_clone(parent->mm);
		new->mm->ref++;
	}

	registers_t *regs = parent->registers;
	registers_dump(regs);
	thread_build_stackframe((void *)new, REGS_IP(regs), (uintptr_t)stack, 0);

	uint8_t *new_sp = (void*)((uintptr_t)new + STACK_SIZE);
	// XXX: This is platform specific, and the 5 uint32_t's are so that the stack
	//		is ready for switch_thread()
	// TODO: return the stack pointer from thread_build_stackframe
	new->sp = (uint8_t *)(new_sp - (sizeof(registers_t) + sizeof(uint32_t)*5));
	
	return new;
}

struct start_args {
	void *(*start_func)(void *);
	void *start_arg;
	volatile int control;
	unsigned long sig_mask[_NSIG/8/sizeof(long)];
};

// XXX: On x86-64 these arguments aren't in the same order?
//int sys_clone(unsigned long flags, void *stack, int *parent_tid, int *child_tid, unsigned long tls);
// TODO:
//      * Check if too many processes are open, return EAGAIN
// XXX: We need to be sure that another thread isn't exec()ing
int sys_clone(unsigned long flags, void *stack, int *parent_tid, unsigned long tls, int *child_tid)
{
	thread_t *cur = thread_current();
	int ret = 0;

	struct pthread *test = (void *)parent_tid - 7*4;
	printf("Address: %p\n", test);
	//printf("%p %x %p %x %x\n", test->map_base, test->map_size, test->stack, test->stack_size, test->tsd);

	struct start_args *sargs = stack;
	printf("start func %p arg: %p\n", sargs->start_func, sargs->start_arg);
	registers_dump(cur->registers);






	if (flags & CLONE_SIGHAND)
		if (!(flags & CLONE_VM))
			return -EINVAL;

	if (flags & CLONE_THREAD)
		if (!(flags & CLONE_SIGHAND))
			return -EINVAL;

	// TODO: there's more EINVAL checks to add

	// Who knows if I'll ever support these
	if ((ret = (flags & CLONE_NEWIPC)))
		goto unsupported_flag;

	if ((ret = (flags & CLONE_NEWNET)))
		goto unsupported_flag;

	if ((ret = (flags & CLONE_NEWPID)))
		goto unsupported_flag;

	if ((ret = (flags & CLONE_NEWUSER)))
		goto unsupported_flag;

	if ((ret = (flags & CLONE_NEWUTS)))
		goto unsupported_flag;

	if ((ret = (flags & CLONE_NEWCGROUP)))
		goto unsupported_flag;

	if ((ret = (flags & CLONE_NEWNS)))
		goto unsupported_flag;

	if ((ret = (flags & CLONE_PIDFD)))
		goto unsupported_flag;

	if ((ret = (flags & CLONE_PTRACE)))
		goto unsupported_flag;

	if ((ret = (flags & CLONE_UNTRACED)))
		goto unsupported_flag;

	// This flag is for advanced IO scheduling that we don't support
	if ((ret = (flags & CLONE_IO)))
		goto unsupported_flag;

	enum intr_status old_level = interrupt_disable();
	printf("CLONE: %x %p %p %p %p\n", flags, stack, parent_tid, tls, child_tid);
	printf("Current TLS:\n");
	if (cur->tls)
		thread_dump_tls(cur->tls);
	thread_t *new = thread_clone_new(flags, stack, tls);

	if (flags & CLONE_PARENT_SETTID) {
		*(int *)parent_tid = cur->pid;
	}

	if (flags & CLONE_CHILD_SETTID) {
		*(int *)child_tid = new->pid;
	}

	if (flags & CLONE_CHILD_CLEARTID) {
		new->clear_child_tid = child_tid;
	}

	if (flags & CLONE_VFORK) {
		printf("VFORK doesn't work yet!\n");
		PANIC("OOPS");
		return -EINVAL;
	}


    printf("New TLS:\n");
    if (tls)
        thread_dump_tls((void *)tls);
    clone_flags_dump(flags);
	thread_set_ready(new);
	interrupt_set(old_level);
    return new->pid;//-ENOSYS;
unsupported_flag:
	serial_printf("CLONE: Unsupported flag: %8x all: %x\n", ret, flags);
	PANIC("OOPS");
	return -EINVAL;
}

pid_t sys_fork()
{
	// FIXME: 
	registers_t *regs = thread_current()->registers;
	uint32_t flags = 0;//CLONE_VM | CLONE_FS | CLONE_SIGHAND | 
	registers_dump(regs);
	return sys_clone(flags, (void *)REGS_SP(regs), NULL, 0, NULL);
//	serial_printf("REGS: User ESP: %X ESP: %X EBP: %X\n", regs->useresp, regs->esp, regs->ebp);
	//return thread_create2(REGS_IP(regs), REGS_SP(regs), NULL);
}

// This is crap but good enough for now
void clone_flags_dump(uint32_t flags)
{
    if (flags & 0x000000ff)
    	printf("CSIGNAL, ");
    if (flags & 0x00000100)
    	printf("CLONE_VM, ");
    if (flags & 0x00000200)
    	printf("CLONE_FS, ");
    if (flags & 0x00000400)
    	printf("CLONE_FILES, ");
    if (flags & 0x00000800)
    	printf("CLONE_SIGHAND, ");
    if (flags & 0x00001000)
    	printf("CLONE_PIDFD, ");
    if (flags & 0x00002000)
    	printf("CLONE_PTRACE, ");
    if (flags & 0x00004000)
    	printf("CLONE_VFORK, ");
    if (flags & 0x00008000)
    	printf("CLONE_PARENT, ");
    if (flags & 0x00010000)
    	printf("CLONE_THREAD, ");
    if (flags & 0x00020000)
    	printf("CLONE_NEWNS, ");
    if (flags & 0x00040000)
    	printf("CLONE_SYSVSEM, ");
    printf("\n");
    if (flags & 0x00080000)
    	printf("CLONE_SETTLS, ");
    if (flags & 0x00100000)
    	printf("CLONE_PARENT_SETTID, ");
    if (flags & 0x00200000)
    	printf("CLONE_CHILD_CLEARTID, ");
    if (flags & 0x00400000)
    	printf("CLONE_DETACHED, ");
    if (flags & 0x00800000)
        printf("CLONE_UNTRACED, ");
    if (flags & 0x01000000)
    	printf("CLONE_CHILD_SETTID, ");
    if (flags & 0x02000000)
    	printf("CLONE_NEWCGROUP, ");
    if (flags & 0x04000000)
    	printf("CLONE_NEWUTS, ");
    if (flags & 0x08000000)
    	printf("CLONE_NEWIPC, ");
    if (flags & 0x10000000)
    	printf("CLONE_NEWUSER, ");
    if (flags & 0x20000000)
    	printf("CLONE_NEWPID, ");
    if (flags & 0x40000000)
    	printf("CLONE_NEWNET, ");
    if (flags & 0x80000000)
    	printf("CLONE_IO, ");
    printf("\n");
}
