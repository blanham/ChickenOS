#include <common.h>
#include <stdio.h>
#include <thread/syscall.h>
#include <chicken/init.h>

//TODO: Here we should do housekeeping, launch any shells or login processes
//		on the various psuedoterminals, and wait()s on children (which takes
//		care of zombie processes)
void init(void *aux UNUSED)
{
	char *argv[] = {"/dash2", NULL};
	char *envp[] = {"PATH=/", NULL};
	char *path = "/dash";

//XXX: Fix this
//	if(!SYSCALL_0N(SYS_FORK))
	{

		SYSCALL_3N(SYS_OPEN, "/dev/tty0", 0, NULL);
		SYSCALL_3N(SYS_OPEN, "/dev/tty0", 0, NULL);
		SYSCALL_3N(SYS_OPEN, "/dev/tty0", 0, NULL);

		SYSCALL_3N(SYS_EXECVE, path, argv, envp);

		PANIC("execv(init) failed!");
	}

	while(1)
		;
}
