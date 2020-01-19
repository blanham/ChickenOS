#include <common.h>
#include <stdio.h>
#include <string.h>
#include <thread/syscall.h>
#include <chicken/init.h>

char *e = "/dev/tty0";
void init(void *aux UNUSED)
{
	char *envp[] = {"TERM=linux", "HOME=/home/blanham"};// {"PATH=/", NULL};
	//char *argv[] = {"python3", "-v", "-c", "print(\"farts\")", NULL};
	//char *path = "/usr/bin/python3";
	char *path = "/home/blanham/chocolate-doom/build/src/chocolate-doom";
	char *argv[] = {"chocolate-doom", "-iwad", "/home/blanham/chocolate-doom/build/doom1.wad", NULL};
	//char *path = "/bin/bash_old";
	//char *argv[] = {"bash", NULL};

	//char *argv[] = {"frotz", "/home/blanham/infocom/hhgg.z5", NULL};
	//char *path = "/usr/bin/frotz";
	//char *path = "/bin/busybox";
	//char *argv[] = {"ld-musl-i386.so.1", "/bin/busybox", NULL};
	//char *path = "/lib/ld-musl-i386.so.1";

	SYSCALL_3N(SYS_OPEN, "/dev/tty", 0, NULL);
	SYSCALL_3N(SYS_OPEN, "/dev/tty", 0, NULL);
	SYSCALL_3N(SYS_OPEN, "/dev/tty", 0, NULL);

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
