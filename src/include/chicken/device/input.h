#ifndef C_OS_DEVICE_INPUT_H
#define C_OS_DEVICE_INPUT_H
/*  ChickenOS - Input device defines, same structs/values as Linux/FreeBSD/etc
 */

#include <stdint.h>
#include <sys/time.h>

struct input_event {
	struct timeval time;
	uint16_t type;
	uint16_t code;
	int32_t value;
};

struct input_keymap_entry {
	uint8_t flags;
	uint8_t len;
	uint16_t index;
	uint32_t keycode;
	uint8_t scancode[32];
};




char input_queue_getc();
void input_queue_putc(char c);

#endif