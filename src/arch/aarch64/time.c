/*	ChickenOS - Rasberry Pi Timer and RTC code
 */
#include <chicken/common.h>
#include <chicken/interrupt.h>
#include <chicken/time.h>
#include <chicken/device/ioport.h>
#include <_interrupt.h>
#include "aarch64_defs.h"


#define BCD_TO_DEC(x) (((x / 16) * 10) + (x & 0xf))


int read_rtc_reg(int f UNUSED) { return 0; }

void arch_rtc_init(struct c_os_time *time)
{

	time->second = read_rtc_reg(0);
	time->minute = read_rtc_reg(2);
	time->hour = read_rtc_reg(4);
	//Should be Weekday, doesn't work correctly
	time->weekday = read_rtc_reg(6);

	time->day = read_rtc_reg(7);
	time->month = read_rtc_reg(8);
	time->year = read_rtc_reg(9);
	time->century = read_rtc_reg(0x32);
}

void arch_timer_init(uint32_t frequency)
{
    (void)frequency;
	//int div = 1193180 / frequency;

	//outb(PIT_CMD, 0x36);
	//outb(PIT0_DATA, div & 0xFF);
	//outb(PIT0_DATA, div >> 8);

	//extern void timer_intr(registers_t *);
	//interrupt_register(IRQ0, &timer_intr);
}

