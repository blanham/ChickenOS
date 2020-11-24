#ifndef C_OS_THREAD_SIGNAL_H
#define C_OS_THREAD_SIGNAL_H

/* thread/signal.c */
void signal_do(registers_t *regs, thread_t *);
int sys_kill(int pid, int sig);
int sigisemptyset(const sigset_t *set);
int sys_sigaction(int sig, const struct k_sigaction *act, struct k_sigaction *oact);
int sys_sigsuspend(const sigset_t *mask);
int sys_sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
int sys_sigreturn(registers_t *regs, unsigned long dunno);

#endif