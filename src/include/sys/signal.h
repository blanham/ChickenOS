#ifndef C_OS_SYS_SIGNAL_H
#define C_OS_SYS_SIGNAL_H
typedef unsigned int sigset_t;
typedef struct {
	int si_signo;
	int si_code;
} siginfo_t;

struct sigaction {
	void (*sa_handler)(int);
	sigset_t sa_mask;
	int sa_flags;
	void (*sa_sigaction)(int, siginfo_t *, void *);

};
#endif
