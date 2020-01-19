#include <common.h>
#include <chicken/thread.h>
#include <kernel/interrupt.h>
#include <arch/i386/interrupt.h> // FIXME this import should be made redundant
#include <errno.h>

void clone_flags_dump(uint32_t flags);


// XXX: On x86-64 these arguments aren't in the same order?
//int sys_clone(unsigned long flags, void *stack, int *parent_tid, int *child_tid, unsigned long tls);
int sys_clone(unsigned long flags, void *stack, int *parent_tid, unsigned long tls, int *child_tid)
{
    printf("CLONE: %x %p %p %p %p\n", flags, stack, parent_tid, tls, child_tid);
    thread_t *cur = thread_current();
    printf("Current TLS:\n");
    if (cur->tls)
        thread_dump_tls(cur->tls);
    printf("New TLS:\n");
    if (tls)
        thread_dump_tls((void *)tls);
    clone_flags_dump(flags);
    return 0;//-ENOSYS;
}

pid_t sys_fork(void *aux)
{
	// FIXME: 
	registers_t *regs = aux;

	serial_printf("REGS: User ESP: %X ESP: %X EBP: %X\n", regs->useresp, regs->esp, regs->ebp);
	return thread_create2(regs->eip, regs->useresp, NULL);
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