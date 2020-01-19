#include <common.h>
//#include <stdint.h>
#include <stddef.h>
//#include <string.h>
//#include <stdio.h>
//#include <chicken/thread.h>
//#include <kernel/memory.h>
//#include <mm/vm.h>
//#include <fs/vfs.h>
//#include <fs/ext2/ext2.h>
//#include <mm/liballoc.h>
//#include <thread/syscall.h>
//#include <sys/stat.h>
//#include <fcntl.h>
#include <errno.h>
#include <sys/uio.h>
#include <sys/select.h>
#include <poll.h>
int poll_toggle_hack = 0;
int sys_poll(struct pollfd *fds UNUSED, nfds_t nfds UNUSED, int timeout UNUSED)
{
	//for (unsigned i = 0; i < nfds; i++) {
		//serial_printf("Test: %i %x %x\n", fds[i].fd, fds[i].events, fds[i].revents);
	//}
	//serial_printf("Polling: %i %i\n", nfds, timeout);
	if (poll_toggle_hack)
		poll_toggle_hack = 0;
	else
		poll_toggle_hack = 1;
	return poll_toggle_hack;
}

int fart2 = 0;
int sys_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
	(void)nfds;
	(void)readfds;
	(void)writefds;
	(void)exceptfds;
	//(void)timeout;
	//serial_printf("select() nfds: %i", nfds);
	if (readfds) {
		//serial_printf(" READFDS: %x", readfds->fds_bits[0]);
	}
	if (writefds) {
		//serial_printf(" WRITEFDS: %x", writefds->fds_bits[0]);
	}
	if (exceptfds) {
		//serial_printf(" EXCEPTFDS: %x\n", exceptfds->fds_bits[0]);
	}
	if (timeout) {
		timeout->tv_sec = 0;
		timeout->tv_usec = 0;
	}
	//serial_printf("\n");
	//if (fart2 < 2)
		//fart2++;
	//else
		//fart2 = 0;
	//return nfds;
	if (fart2) {
		fart2 = 0;
		return 1;
	}
	return 0;//fart2 & 0x3 ? 1 : 0;
}
