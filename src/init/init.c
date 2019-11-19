#include <common.h>
#include <stdio.h>
#include <thread/syscall.h>
#include <chicken/init.h>

//TODO: Here we should do housekeeping, launch any shells or login processes
//		on the various psuedoterminals, and wait()s on children (which takes
//		care of zombie processes)
char *e = "/dev/tty0";
void init(void *aux UNUSED)
{
	char *argv[] = {"/sh", NULL};
	char *envp[] = {"PATH=/", NULL};
	char *path = "/toybox";

	SYSCALL_3N(SYS_OPEN, "/dev/tty0", 0, NULL);
	SYSCALL_3N(SYS_OPEN, "/dev/tty0", 0, NULL);
	SYSCALL_3N(SYS_OPEN, "/dev/tty0", 0, NULL);

	//XXX: Fix this
//	if(!SYSCALL_0N(SYS_FORK))
	{
		//uintptr_t p = 0;
		//stackpointer_get(p);
		//

		//SYSCALL_3N(SYS_OPEN, p, 0, NULL);
		//SYSCALL_3N(SYS_OPEN, e, 0, NULL);

		SYSCALL_3N(SYS_EXECVE, (void *)path, argv, envp);

		PANIC("execv(init) failed!");
	}

	while(1)
		;
}
