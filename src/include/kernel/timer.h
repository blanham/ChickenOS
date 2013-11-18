#ifndef C_OS_TIMER_H
#define C_OS_TIMER_H
#include <sys/time.h>
void time_init();
int sys_gettimeofday(struct timeval *tp, void *tzp);
void time_sleep(int seconds);
void time_msleep(int mseconds);
void time_usleep(int useconds);
int sys_clock_gettime(int, struct timespec *);
#endif
