#include <common.h>
#include <stdio.h>
#include <string.h>
#include <thread/syscall.h>
#include <chicken/init.h>

char *e = "/dev/tty0";
void init(void *aux UNUSED)
{
	char *argv[] = {"bash", NULL};
	char *envp[] = {"TERM=linux"};// {"PATH=/", NULL};
	//char *path = "/busybox";
	char *path = "/bin/bash";

	SYSCALL_3N(SYS_OPEN, "/dev/tty0", 0, NULL);
	SYSCALL_3N(SYS_OPEN, "/dev/tty0", 0, NULL);
	SYSCALL_3N(SYS_OPEN, "/dev/tty0", 0, NULL);

	//XXX: Fix this
	//if(!SYSCALL_0N(SYS_FORK))
	{
		//uintptr_t p = 0;
		//stackpointer_get(p);
		//

		//SYSCALL_3N(SYS_OPEN, p, 0, NULL);
		//SYSCALL_3N(SYS_OPEN, e, 0, NULL);

		//printf("\033[2J");

		//char *clear_screen = "\033[2J\033[H";
		//SYSCALL_3N(SYS_WRITE, 0, clear_screen, strlen(clear_screen));
		SYSCALL_3N(SYS_EXECVE, (void *)path, argv, envp);

		PANIC("execv(init) failed!");
	}

	while(1)
		;
}
