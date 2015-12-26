#ifndef C_OS_TIMER_H
#define C_OS_TIMER_H
#include <stdint.h>
#include <sys/time.h>

struct c_os_time {
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t month;
	uint8_t day;
	uint8_t year;
	uint8_t weekday;
	uint8_t century;
};

void time_init();
int sys_gettimeofday(struct timeval *tp, void *tzp);
void time_sleep(int seconds);
void time_msleep(int mseconds);
void time_usleep(int useconds);
int sys_clock_gettime(int, struct timespec *);
#endif
