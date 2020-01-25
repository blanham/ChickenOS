#include <errno.h>
#include <sys/types.h>
#include <arch/i386/interrupt.h>
#include <chicken/common.h>
#include <chicken/thread.h>

// NOTE: Might be worth using a macro for these?
uid_t sys_getegid()
{
	return thread_current()->egid;
}

// NOTE: Might be worth using a macro for these?
uid_t sys_geteuid()
{
	return 2;//thread_current()->euid;
}

int sys_setuid(uid_t uid)
{
	thread_current()->uid = uid;
	return 0;
}

uid_t sys_getuid()
{
	return 2;//thread_current()->uid;
}

// TODO: Should return tgid instead
pid_t sys_getpid()
{
	return thread_current()->tgid;
}

pid_t sys_getppid()
{
	return thread_current()->ppid;
}

pid_t sys_getpgrp()
{
	thread_t *cur = thread_current();
	pid_t pgid = cur->pgid;
//	printf("cur %X pid = %i\n",cur,pgid);
	return pgid;
}

// FIXME: Incorrect behavior
int sys_setpgid(pid_t pid, pid_t pgid)
{
	thread_t *thread = thread_current();
	(void)pid;
	thread->pgid = pgid;
	return 0;
}

gid_t sys_getgid()
{
	return thread_current()->gid;
}

int sys_setgid(gid_t gid)
{
	thread_t *cur = thread_current();
	cur->gid = gid;
	return 0;
}

void sys_exit(int exit_code)
{
	thread_exit(exit_code);
}

//TODO: For threading, exit()s all with the same tgid
void sys_exit_group(int return_code)
{
	(void)return_code;
}


int sys_set_tid_address(int *ptr)
{
	thread_t *cur = thread_current();
	cur->clear_child_tid = ptr;
	return cur->pid;
}