#include <chicken/common.h>
#include <chicken/device/input.h>
#include <chicken/interrupt.h>

char input_buf[256];
int queue_pos = 0;

char input_queue_getc()
{
	char c;
	while(queue_pos == 0);

//	enum intr_status old = interrupt_disable();
	c = input_buf[queue_pos];
	queue_pos--;

//	interrupt_set(old);

	return c;
}

void input_queue_putc(char c)
{
	input_buf[queue_pos] = c;
	queue_pos++;
}