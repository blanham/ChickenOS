#ifndef C_OS_SYS_SOCKET_H
#define C_OS_SYS_SOCKET_H
#include <kernel/types.h>
typedef uint32_t socklen_t;
typedef uint32_t sa_family_t;

struct sockaddr {
	sa_family_t sa_family;
	char		sa_data[];
};

enum {
	AF_UNIX,
	AF_UNSPEC,
	AF_INET
};

enum {
	SHUT_RD,
	SHUT_WR,
	SHUT_RDWR
};

enum {
	SOCK_STREAM,
	SOCK_DGRAM,
	SOCK_RAW,
	SOCK_SEQPACKET,
	SOCK_RDM
};
/*
struct msghdr {
void         *msg_name        optional address
socklen_t     msg_namelen     size of address
struct iovec *msg_iov         scatter/gather array
int           msg_iovlen      members in msg_iov
void         *msg_control     ancillary data, see below
socklen_t     msg_controllen  ancillary data buffer len
int           msg_flags       flags on received message
};
struct cmsghdr {
socklen_t     cmsg_len        data byte count, including the cmsghdr
int           cmsg_level      originating protocol
int           cmsg_type       protocol-specific type

};

*/
int sys_accept(int socket, struct sockaddr *address, socklen_t *address_len);
int sys_bind(int socket, const struct sockaddr *address, socklen_t address_len);
int sys_connect(int socket, const struct sockaddr *address,socklen_t address_len);
int sys_getpeername(int socket, struct sockaddr *address,socklen_t *address_len);
int sys_getsockname(int socket, struct sockaddr *address,socklen_t *address_len);
int sys_getsockopt(int socket, int level, int option_name, void *option_value,
	socklen_t *option_len);
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
int sys_socket(int domain, int type, int protocol);
int sys_socketpair(int domain, int type, int protocol, int socket_vector[2]);
#endif
