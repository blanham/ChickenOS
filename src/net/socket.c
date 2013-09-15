#include <stdio.h>
#include <mm/liballoc.h>
#include <net/net_core.h>
#include <kernel/common.h>
#include <kernel/memory.h>
#include <sys/socket.h>
#include <queue.h>
#include <string.h>

int socketfd = 0;

/*
struct socket {
	struct network_dev *dev;
	int pid;
	enum sock_type type;
	enum proto_type proto;
	uint16_t src_port;
	uint16_t dst_port;	

};
*/

int sys_accept(int socket UNUSED, struct sockaddr *address UNUSED, socklen_t *address_len UNUSED)
{
	return -1;
}

int sys_bind(int socket UNUSED, const struct sockaddr *address UNUSED, socklen_t address_len UNUSED)
{
	return -1;
}

int sys_connect(int socket UNUSED, const struct sockaddr *address,socklen_t address_len);

int sys_getpeername(int socket, struct sockaddr *address,socklen_t *address_len);

int sys_getsockname(int socket, struct sockaddr *address,socklen_t *address_len);

int sys_getsockopt(int socket, int level, int option_name, void *option_value, socklen_t *option_len);

int sys_listen(int socket, int backlog);

ssize_t sys_recv(int socket, void *buffer, size_t length, int flags);

ssize_t sys_recvfrom(int socket, void *buffer, size_t length, int flags, 
	struct sockaddr *address, socklen_t *address_len);

//ssize_t recvmsg(int socket, struct msghdr *message, int flags);

ssize_t sys_send(int socket, const void *message, size_t length, int flags);

//ssize_t sendmsg(int socket, const struct msghdr *message, int flags);

ssize_t sys_sendto(int socket, const void *message, size_t length, int flags, 
	const struct sockaddr *dest_addr, socklen_t dest_len);

int sys_setsockopt(int socket, int level, int option_name, const void *option_value,
	socklen_t option_len);

int sys_shutdown(int socket, int how);

int sys_socket(int domain UNUSED, int type, int protocol UNUSED)
{
	switch(type)
	{
		case SOCK_STREAM:
		case SOCK_DGRAM:
		case SOCK_RAW:
		default:
		break;


	}
	return -1;
}

int sys_socketpair(int domain, int type, int protocol, int socket_vector[2]);
