#ifndef C_SYS_SOCKET_H
#define C_SYS_SOCKET_H
#include <sys/types.h>

#define AF_LOCAL 	AF_UNIX
#define AF_UNIX 	0
#define AF_INET 	1
#define AF_INET6	28

#define PF_UNSPEC 	AF_UNSPEC
#define PF_LOCAL 	AF_LOCAL
#define PF_UNIX		AF_UNIX
#define PF_INET		AF_INET
#define PF_INET6	AF_INET6

#define SOCK_STREAM 1
#define SOCK_DGRAM	2
#define SOCK_RAW	3
typedef unsigned long sa_family_t;
typedef unsigned int socklen_t;

struct sockaddr {
	sa_family_t sa_family;
	char sa_data[];
};

int c_socket(int domain, int type, int protocol);;

int c_connect(int s, const struct sockaddr *name, socklen_t namelen);

int c_bind(int s, const struct sockaddr *addr, socklen_t addrlen);

int c_listen(int s, int backlog);

int c_accept(int s, struct sockaddr *addr, socklen_t *addrlen);

int c_close(int s);

ssize_t c_recv(int s, void *buf, size_t len, int flags);

ssize_t c_recvfrom(int s, void *buf, size_t len, int flags,
                 struct sockaddr *from, socklen_t *fromlen);

ssize_t c_send(int s, const void *buf, size_t len, int flags);

ssize_t c_sendto(int s, const void *buf, size_t len,
               int flags, const struct sockaddr *to,
               socklen_t tolen);
#endif
