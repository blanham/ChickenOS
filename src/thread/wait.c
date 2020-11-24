#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <chicken/boot.h>
#include <chicken/thread.h>
/*
#define WNOHANG    1
#define WUNTRACED  2

#define WSTOPPED   2
#define WEXITED    4
#define WCONTINUED 8
#define WNOWAIT    0x1000000

#define __WNOTHREAD 0x20000000
#define __WALL      0x40000000
#define __WCLONE    0x80000000

#define WEXITSTATUS(s) (((s) & 0xff00) >> 8)
#define WTERMSIG(s) ((s) & 0x7f)
#define WSTOPSIG(s) WEXITSTATUS(s)
#define WCOREDUMP(s) ((s) & 0x80)
#define WIFEXITED(s) (!WTERMSIG(s))
#define WIFSTOPPED(s) ((short)((((s)&0xffff)*0x10001)>>8) > 0x7f00)
#define WIFSIGNALED(s) (((s)&0xffff)-1 < 0xffu)
#define WIFCONTINUED(s) ((s) == 0xffff)*/


//FIXME: Busy waiting :c
pid_t sys_wait4(pid_t pid, int *status, int options, struct rusage *rusage)
{
	printf("WAIT4: %i %p %X %p\n", pid, status, options, rusage);






	(void)pid; (void)status; (void)options; (void)rusage;
	thread_t *cur = thread_current();
	thread_t *child;
	pid_t ret = 0;
//	printf("STATUS %i\n", *status);

	while(1)
	{
		LL_FOREACH2(cur->children, child, child_next)
		{
			ret = child->pid;
			if(child->status == THREAD_DEAD)
			{
				//printf("Child %x %i\n", child, child->pid);
				cur->children = NULL;
				*status = 0x80;
				return ret;
			}

		}
		if(ret)
		{
			if(options & WNOHANG)
				return 0;
			//wait
			if(!sigismember(&cur->sig_info->pending, SIGCHLD))
				continue;
			else
				return -EINTR;
		}

		return -ECHILD;
	}
	PANIC("lulz\n");
	return -ECHILD;
}
