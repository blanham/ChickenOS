#ifndef C_OS_SYSCALL_H
#define C_OS_SYSCALL_H
enum { SYS_FORK,SYS_GETPID, SYS_PUTS, SYS_DUMMY = 256};
void syscall_init();
pid_t fork();
pid_t get_pid();
#endif
