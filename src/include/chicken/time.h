#ifndef C_OS_TIME_H
#define C_OS_TIME_H
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

/* arch/$ARCH/time.c */
void arch_timer_init(uint32_t frequency);
void arch_rtc_init(struct c_os_time *time);

/* device/time.c */
void time_init();
void time_sleep(int seconds);
void time_msleep(int mseconds);
void time_usleep(int useconds);
int sys_clock_gettime(int, struct timespec *);
int sys_gettimeofday(struct timeval *tp, void *tzp);
int sys_nanosleep(const struct timespec *req, struct timespec *rem);

#endif