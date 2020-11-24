#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <chicken/common.h>
#include <chicken/interrupt.h>
#include <chicken/thread.h>
#include <chicken/thread/signal.h>

int sigisemptyset(const sigset_t *set)
{
	static const unsigned long  empty[_NSIG/8/sizeof(long)];
	int ret =  !memcmp(set, &empty, _NSIG/8);
	return ret;
}

int sigaddset(sigset_t *set, int sig)
{
	unsigned int s = sig - 1;
	if (s >= _NSIG-1 || sig-32U < 3) {
		printf("Sig %i %i %i\n", sig, s >= _NSIG-1, sig-32U < 3);
		ASSERT(0, "Invalid signal number passed!");
	}
	set->__bits[s/8/sizeof *set->__bits] |= 1UL << ( s & (8 * sizeof(*set->__bits) - 1));
	return 0;
}

int sigdelset(sigset_t *set, int sig)
{
	unsigned int s = sig - 1;
	if (s >= _NSIG-1 || sig-32U < 3) {
		printf("Sig %i %i %i\n", sig, s >= _NSIG-1, sig-32U < 3);
		ASSERT(0, "Invalid signal number passed!");
	}
	set->__bits[s/8/sizeof *set->__bits] &= ~(1UL << ( s & (8 * sizeof(*set->__bits) - 1)));
	return 0;
}

int sigpopset(sigset_t *set)
{
	int ret = 32 - __builtin_clzl(set->__bits[0]);
/*	printf("ret %i\n", ret);ret =__builtin_clzl(set->__bits[1]);
	printf("ret %i\n", ret);*/
	return ret;
}
int sigismember(const sigset_t *set, int sig)
{
	unsigned int s = sig - 1;
	if (s >= _NSIG-1 || sig-32U < 3) {
		printf("Sig %i %i %i\n", sig, s >= _NSIG-1, sig-32U < 3);
		ASSERT(0, "Invalid signal number passed!");
	}

	return !!(set->__bits[s/8/sizeof *set->__bits] & 1UL<<(s&(8*sizeof *set->__bits-1)));
}