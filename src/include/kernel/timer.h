#ifndef C_OS_TIMER_H
#define C_OS_TIMER_H
#include <types.h>
#include <sys/types.h>
/*typedef uint32_t time_t;
typedef int32_t suseconds_t;
*/
struct timeval {
	time_t tv_sec;
	suseconds_t tv_usec;
};
void time_init();
int sys_gettimeofday(struct timeval *tp, void *tzp);
void time_sleep(int seconds);
void time_msleep(int mseconds);
void time_usleep(int useconds);
#endif
