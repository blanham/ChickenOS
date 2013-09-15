#ifndef C_OS_SYS_TIME_H
#define C_OS_SYS_TIME_H
#include <sys/types.h>
struct timeval {
	time_t tv_sec;
	suseconds_t tv_usec;

};
#endif
