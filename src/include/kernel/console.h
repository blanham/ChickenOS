#ifndef C_OS_CONSOLE_H
#define C_OS_CONSOLE_H
#include <kernel/types.h>
enum console_color {
	BLACK, BLUE, GREEN, CYAN,
	RED, MAGENTA, BROWN, LT_GREY,
	DK_GREY,LT_BLUE,LT_GREEN, LT_CYAN,
	LT_RED, LT_MAGENTA, LT_BROWN, WHITE

};
void console_init();
void console_clear();
void console_set_color(uint8_t bg_color, uint8_t fg_color);
void console_putc(uint8_t c);
int console_puts(char *string);

void kbd_init();
#endif
