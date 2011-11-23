/* PIT and RTC routines */
#include <kernel/timer.h>
#include <kernel/interrupt.h>
#include <kernel/console.h>
#include <kernel/hw.h>
#include <stdio.h>

#define PIT0_DATA 0x40
#define PIT1_DATA 0x41
#define PIT2_DATA 0x42
#define PIT_CMD   0x43

#define RTC_REG    0x70
#define RTC_DATA   0x71
#define RTC_UPDATE 0x80
uint8_t rtc_data[8];
struct c_os_time {
	uint8_t hour;
	uint8_t minute;
	uint8_t sec;
	uint8_t month;
	uint8_t day;
	uint8_t year;
	uint8_t weekday;
	uint8_t century;

};
#define BCD_TO(x) (((x / 16) * 10) + (x & 0xf))
char * days[7] = {
	"Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","Sunday"
};
uint8_t read_rtc_reg(uint8_t reg)
{
/*	int chk = 0;
start:
	outb(RTC_REG, 0xA);
	chk = inb(RTC_REG);
	if(!(chk&RTC_UPDATE))
		goto start;
*/	outb(RTC_REG, reg);

	return inb(RTC_DATA);

}
/* gregorian date to julian day function
 * taken from:http://www.hermetic.ch/cal_stud/jdn.htm#comp
 */
int32_t jdt(int8_t m, int8_t d, int16_t y)
{
int32_t jd = ( 1461 * ( y + 4800 + ( m - 14 ) / 12 ) ) / 4 +
          ( 367 * ( m - 2 - 12 * ( ( m - 14 ) / 12 ) ) ) / 12 -
          ( 3 * ( ( y + 4900 + ( m - 14 ) / 12 ) / 100 ) ) / 4 +
          d - 32075;

return jd;


}

void rtc_data_print(uint8_t *data)
{
	printf("%s ",days[data[3]]);
	printf("%i/%i/%i  ", data[5], data[4], data[6]);
	printf("%i:%i:%i ", data[2], data[1], data[0]);
//	uint32_t timestamp = (data[7]+30)*31556926;
//	timestamp +=
 
uint32_t m,d,y;
m = data[5];
d = data[4];
y = data[6] + 2000;
	//uint32_t test = jdt(11,22,2011);
	uint32_t test2 = (jdt(m,d,y) - jdt(1,1,1970));
	uint32_t test3 = data[2]*60*60 + data[1]*60 + data[0]; 
	printf("timestamp: %i\n",(test2)* 24 * 60 * 60 + test3);
}
void rtc_init()
{

	int format = read_rtc_reg(0xB);
	
	rtc_data[0] = read_rtc_reg(0);
	rtc_data[1] = read_rtc_reg(2);
	rtc_data[2] = read_rtc_reg(4);
		
	rtc_data[4] = read_rtc_reg(7);
	rtc_data[5] = read_rtc_reg(8);
	rtc_data[6] = read_rtc_reg(9);
	rtc_data[7] = read_rtc_reg(0x32);
	if(format == 2)
		for(int i = 0; i < 7; i++)
			rtc_data[i] = BCD_TO(rtc_data[i]);
	if(rtc_data[2] == 0) rtc_data[2] = 12;
	if(rtc_data[2] > 12) rtc_data[2] -= 12;

	rtc_data_print(rtc_data);


}

void timer_intr(struct registers * regs)
{
	regs = regs;
//	printf("lulz\n");
}
void timer_init(uint32_t frequency)
{
	frequency = frequency;
/*	int div = 1193180 / frequency;
	outb(PIT_CMD, 0x36);
	outb(PIT0_DATA, div & 0xFF);
	outb(PIT0_DATA, div >> 8);
*/	
	interrupt_register(IRQ0, &timer_intr);
}

void time_init()
{

	rtc_init();

	timer_init(2);


}
