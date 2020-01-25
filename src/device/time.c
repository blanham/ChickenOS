/*	ChickenOS time routines
 */
#include <common.h>
#include <stdio.h>
#include <chicken/thread.h>
#include <chicken/time.h>
#include <kernel/hw.h>
#include <errno.h>
#include <sys/time.h>

struct c_os_time system_datetime;
uint32_t ticks = 0;
uint32_t unix_time;

char * days[7] = {
	"Sunday", "Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"
};

// Gregorian date to Julian day function
// taken from:http://www.hermetic.ch/cal_stud/jdn.htm#comp
int32_t jdt(int8_t m, int8_t d, int16_t y)
{
	int32_t jd = ( 1461 * ( y + 4800 + ( m - 14 ) / 12 ) ) / 4 +
		( 367 * ( m - 2 - 12 * ( ( m - 14 ) / 12 ) ) ) / 12 -
		( 3 * ( ( y + 4900 + ( m - 14 ) / 12 ) / 100 ) ) / 4 +
		d - 32075;

	return jd;
}

void time_print(struct c_os_time *time)
{
	printf("%s ",days[time->weekday]);
	printf("%i/%i/%i ", time->month, time->day, time->year+2000);
	printf("%i:%i:%i\n", time->hour, time->minute, time->second);
}

void time_set_from_rtc(struct c_os_time *time)
{
	uint32_t julian_day = jdt(time->month, time->day,time->year+2000);
	uint32_t days_since_epoch = (julian_day - jdt(1,1,1970));
	uint32_t seconds_since_midnight = time->hour*60*60 + time->minute*60 + time->second;
	unix_time = (days_since_epoch * 24 * 60 * 60) + seconds_since_midnight;
	time->weekday = (julian_day + 1) % 7;
	time_print(time);
}

void rtc_init()
{
	arch_rtc_init(&system_datetime);

	time_set_from_rtc(&system_datetime);
}

void timer_intr(registers_t * regs)
{
	ticks++;
	if (ticks % 100 == 0)
		unix_time++;
	if (!(ticks & 0x3))
		scheduler_run(regs);
}

// FIXME: Why not have a busy wait function that takes a timespec?

//XXX: These sleep functions are shit, should be done as a priority queue,
//		where the priority is the time the thread should be rescheduled
void time_sleep(int seconds)
{
	uint32_t wait = seconds + unix_time;
	while(unix_time <  wait);
	return;
}

void time_msleep(int mseconds)
{
	uint32_t wait = mseconds + ticks;
	while(ticks <  wait);
	return;
}

void time_usleep(int useconds)
{
	uint32_t wait = useconds/10 + ticks;
	while(ticks <  wait);
	return;
}

void time_init()
{
	rtc_init();
	arch_timer_init(100);
}

int sys_gettimeofday(struct timeval *tp, void *tzp UNUSED)
{
	if(tp != NULL)
	{
		tp->tv_sec = unix_time;
		tp->tv_usec = (ticks % 100) * 100;
	}

	return 0;
}

int sys_clock_gettime(int type, struct timespec *tp)
{
	(void)type;
	static int s = 0;
	tp->tv_sec = unix_time;
	tp->tv_nsec = s;
	// XXX: lolwat:
	s += 100;
	unix_time++;
	return 0;
}

int sys_nanosleep(const struct timespec *req, struct timespec *rem)
{
	if (verify_pointer(req, sizeof(req), VP_READ))
		return -EFAULT;


	(void)rem;



	// FIXME: If a signal happened before we return, return -EINTR, write remaining time into rem if rem not null
	return 0;
}
