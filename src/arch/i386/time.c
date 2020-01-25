/*	ChickenOS PIT and RTC routines
 */
#include <arch/i386/interrupt.h>
#include <chicken/common.h>
#include <chicken/interrupt.h>
#include <chicken/time.h>
#include <chicken/device/ioport.h>
#include "i386_defs.h"

#define PIT0_DATA	0x40
#define PIT1_DATA	0x41
#define PIT2_DATA	0x42
#define PIT_CMD		0x43

#define RTC_REG		0x70
#define RTC_DATA	0x71
#define RTC_UPDATE	0x80

#define BCD_TO_DEC(x) (((x / 16) * 10) + (x & 0xf))

int rtc_format = 0;

uint8_t read_rtc_reg(uint8_t reg)
{
	// FIXME: This is supposed to wait, but I disabled it for some reason
/*
	int chk = 0;
start:
	outb(RTC_REG, 0xA);
	chk = inb(RTC_REG);
	if(!(chk&RTC_UPDATE))
		goto start;
*/
	outb(RTC_REG, reg);

	uint8_t ret = inb(RTC_DATA);
	if (rtc_format == 2)
		return BCD_TO_DEC(ret);
	return ret;
}

void arch_rtc_init(struct c_os_time *time)
{
	rtc_format = read_rtc_reg(0xB);

	time->second = read_rtc_reg(0);
	time->minute = read_rtc_reg(2);
	time->hour = read_rtc_reg(4);
	//Should be Weekday, doesn't work correctly
	time->weekday = read_rtc_reg(6);

	time->day = read_rtc_reg(7);
	time->month = read_rtc_reg(8);
	time->year = read_rtc_reg(9);
	time->century = read_rtc_reg(0x32);
	//XXX: Is this right?
	if(time->hour == 0) time->hour = 12;
	if(time->hour > 12) time->hour -= 12;
}

void arch_timer_init(uint32_t frequency)
{
	int div = 1193180 / frequency;

	outb(PIT_CMD, 0x36);
	outb(PIT0_DATA, div & 0xFF);
	outb(PIT0_DATA, div >> 8);

	extern void timer_intr(registers_t *);
	interrupt_register(IRQ0, &timer_intr);
}

