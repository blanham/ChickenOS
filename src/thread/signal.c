/*	ChickenOS - thread/signal.c
 *	Thread level support of signals
 *	XXX: This is hot garbage
 */
#include <common.h>
#include <errno.h>
#include <kernel/thread.h>
#include <kernel/memory.h>
#include <device/console.h>
#include <kernel/interrupt.h>
#include <mm/vm.h>
#include <thread/syscall.h>
#include <stdio.h>
#include <mm/liballoc.h>

/*
	Signal control flow:
	When switching to a new thread:
		Check if thread->signal_pending != 0:
			If so run do_signal(regs, thread)
				If in user mode:
					save old usermode esp
					set regs->useresp = thread->signal_stack()

   */

extern bool thread_start;
void signal_test_user_func(int sig)  __attribute__((section(".user")));
void signal_test_restore_func(long unknown)  __attribute__((section(".user")));

int sigisemptyset(const sigset_t *set)
{
	static const unsigned long  empty[_NSIG/8/sizeof(long)];
	int ret =  !memcmp(set, &empty, _NSIG/8);
	return ret;
}
int sigaddset(sigset_t *set, int sig)
{
	unsigned int s = sig - 1;
	if(s >= _NSIG-1 || sig-32U < 3)
	{
		printf("Sig %i %i %i\n", sig, s >= _NSIG-1, sig-32U < 3);
		ASSERT(0, "Invalid signal number passed!");
	}
	set->__bits[s/8/sizeof *set->__bits] |= 1UL << ( s & (8 * sizeof(*set->__bits) - 1));
	return 0;
}
int sigdelset(sigset_t *set, int sig)
{
	unsigned int s = sig - 1;
	if(s >= _NSIG-1 || sig-32U < 3)
	{
		printf("Sig %i %i %i\n", sig, s >= _NSIG-1, sig-32U < 3);
		ASSERT(0, "Invalid signal number passed!");
	}
	set->__bits[s/8/sizeof *set->__bits] &= ~(1UL << ( s & (8 * sizeof(*set->__bits) - 1)));
	return 0;
}

int sigpopset(sigset_t *set)
{
	int ret =32 - __builtin_clzl(set->__bits[0]);
/*	printf("ret %i\n", ret);ret =__builtin_clzl(set->__bits[1]);
	printf("ret %i\n", ret);*/
	return ret;
}
int sigismember(const sigset_t *set, int sig)
{
	unsigned int s = sig - 1;
	if(s >= _NSIG-1 || sig-32U < 3)
	{
		printf("Sig %i %i %i\n", sig, s >= _NSIG-1, sig-32U < 3);
		ASSERT(0, "Invalid signal number passed!");
	}

	return !!(set->__bits[s/8/sizeof *set->__bits] & 1UL<<(s&(8*sizeof *set->__bits-1)));
}
int sys_sigreturn(registers_t *regs, unsigned long dunno UNUSED)
{
	(void)regs;
	/*
	thread_t *cur = thread_current();
	registers_t *regs_bottom = (void *)cur + STACK_SIZE - sizeof(registers_t);
//	printf("DUNON %i\n", dunno);
	interrupt_disable();
	ASSERT(regs_bottom == regs, "Tried to handle a signal inside interrupt context\n");
//	dump_regs(regs);
//	printf("eax %i\n", regs->eax);

	kmemcpy(regs_bottom, cur->signal_regs, sizeof(*regs_bottom));
//
//	printf("eax %i\n", regs->eax);

//	dump_regs(regs);
	sigdelset(&cur->sig_info->pending, 17);
//	cur->status = THREAD_READY;
	thread_scheduler(regs_bottom);
	return -EINTR;//regs->eax;
	*/
	return 0;
}


//#define SIGNAL_TEST
void signal_do(registers_t *regs, thread_t *next)
{
	(void)regs;
	(void)next;
	/*
	volatile registers_t *regs_bottom = (void *)next + STACK_SIZE - sizeof(registers_t);
	int sig_num;// = next->sig_info->signal_pending;
	struct k_sigaction *sig;
	volatile uint32_t *push = NULL;
	(void)regs;
//	printf("Maybe %i\n",sigpopset(&next->sig_info->pending));
	sig_num = sigpopset(&next->sig_info->pending);
	sig = &next->sig_info->signals[sig_num];
	//hack until we do proper signal ignoring
	if(sig->handler == NULL)
	{
		next->sig_info->signal_pending = 0;
		return;
	}
	interrupt_disable();
	void * restore;
	uint32_t new_eip;

//	printf("handler %p flgs %lx restorer %p mask1 %x mask2 %x sig %i\n",
//			sig->handler, sig->flags, sig->restorer, sig->mask[0], sig->mask[1], sig_num);

#ifdef SIGNAL_TEST
	restore = (void*)signal_test_user_func;
	new_eip = (uint32_t)signal_test_restore_func;
#else
	restore = (void*)sig->restorer;
	new_eip = (uintptr_t)sig->handler;
#endif

	if(next->signal_regs == NULL)
		next->signal_regs = kcalloc(1, sizeof(registers_t));

	kmemcpy(next->signal_regs, (void *)regs_bottom, sizeof(*regs_bottom));
	next->useresp = (uint8_t *)regs_bottom->useresp;

	push = (uint32_t *)regs_bottom->useresp;
	*--push = 0xDEADC0DE;
	*--push = sig_num;
	*--push =(uintptr_t)restore;

	regs_bottom->useresp = (uintptr_t)push;
	regs_bottom->eip = (uint32_t)new_eip;

	next->sig_info->signal_pending = 0;
//	next->status = THREAD_UNINTERRUPTIBLE;
	interrupt_enable();
*/
}

//TODO: Doesn't return EINTR if we were interrupted by a signal
int sys_sigsuspend(const sigset_t *mask)
{
	thread_t *cur = thread_current();
	struct thread_signals *sig_info = cur->sig_info;
	sigset_t save;

	interrupt_disable();
	//verify_pointer(mask, sizeof(*mask));

	kmemcpy(&save, &sig_info->sigmask, sizeof(sig_info->sigmask));
	kmemcpy(&sig_info->sigmask, (void *)mask, sizeof(*mask));
//	printf("Blocking! Esp ~= %x\n", &mask);
	cur->status = THREAD_BLOCKED;
//	thread_scheduler(sig_info->regs);
//	thread_yield();
	while(cur->status == THREAD_BLOCKED);
	printf("We woke up! pid %i\n", thread_current()->pid);
//	printf("We woke up! pid %i\n", sig_info->pid);
	kmemcpy(&sig_info->sigmask, &save, sizeof(sig_info->sigmask));
//	registers_t *regs_bottom = (void *)sig_info + STACK_SIZE - sizeof(registers_t);
//	dump_regs(regs_bottom);

	interrupt_enable();

	return -EINTR;
}

int sys_sigaction(int sig, const struct k_sigaction *act, struct k_sigaction *oact)
{
	thread_t *cur = thread_current();

	//verify_pointer(act, sizeof(*act));
	//verify_pointer(oact, sizeof(*oact));
	interrupt_disable();

//	printf("%i %x %x cur %x\n", sig, act, oact, cur);

	if(sig > NUM_SIGNALS -1 || sig == SIGKILL || sig == SIGSTOP)
		return -EINVAL;

//	printf("sigaction %ii %i %i %i\n", sig, SIGKILL, SIGSTOP, NUM_SIGNALS-1);

	if(oact != NULL)
	{
		kmemcpy(oact, &cur->sig_info->signals[sig], sizeof(*oact));
	}

	if(act != NULL)
	{
	//	printf("%x\n", act->handler);
		kmemcpy(&cur->sig_info->signals[sig], act, sizeof(*act));
	//	printf("%x\n", &cur->sig_info->signals[sig]);
	}

	interrupt_enable();

	return 0;
}

int sys_sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
	thread_t *cur = thread_current();
	struct thread_signals *sig_info = cur->sig_info;

	//verify_pointer(set, sizeof(*set));
	//verify_pointer(oldset, sizeof(*set));
//	printf("%p %p\n", set, oldset);
	if(oldset != NULL)
	{
		kmemcpy(oldset->__bits, sig_info->sigmask.__bits, sizeof(sigset_t));
	}
//	printf(" HOW %i\n", how);
	if(set != NULL)
	switch(how)
	{
		case SIG_BLOCK:
			sig_info->sigmask.__bits[0] |= set->__bits[0];
			sig_info->sigmask.__bits[1] |= set->__bits[1];
			return 0;
		case SIG_UNBLOCK:
			sig_info->sigmask.__bits[0] &= ~set->__bits[0];
			sig_info->sigmask.__bits[1] &= ~set->__bits[1];
			return 0;
		case SIG_SETMASK:
			kmemcpy(sig_info->sigmask.__bits, set->__bits, sizeof(sigset_t));
			return 0;
		default:
			return -EINVAL;
	}
	return -EINVAL;
}

//FIXME handle permissions?
int sys_kill(pid_t pid, int sig)
{
	thread_t *p = thread_by_pid(pid);

	if(p == NULL)
		return -ESRCH;

	if(sig > NUM_SIGNALS-1)
		return -EINVAL;

	//printf("sys_kill: Thread %x Pid %i Sig %i sp %x\n", p, pid, sig, p->sp);

	//FIXME: Not right
	if(p->status == THREAD_BLOCKED)
		p->status = THREAD_READY;

	sigaddset(&p->sig_info->pending, sig);
	p->sig_info->signal_pending = 1;

	return 0;
}

void signal_test_restore_func(long unknown)
{
	//thread_t *cur = thread_current();
	printf("Signal #%x Address %x\n", unknown, &unknown);
	printf("Return address: %p\n",  __builtin_return_address (0));
	printf("Frame address: %p\n",  (uint32_t *)__builtin_frame_address (0));
}
void signal_test_user_func(int sig)
{
	printf("Signal #%x Address %x\n", sig, &sig);
	printf("Return address: %p\n",  __builtin_return_address (0));
	printf("Frame address: %p\n",  (uint32_t *)__builtin_frame_address (0));

}

