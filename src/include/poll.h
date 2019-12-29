#ifndef CHICKEN_POLL_H
#define CHICKEN_POLL_H
#include <stdint.h>

typedef uint32_t nfds_t;
struct pollfd {
	int fd; // The following descriptor being polled.
	uint16_t events; // The input event flags
	uint16_t revents; // The output event flags
};

#endif
