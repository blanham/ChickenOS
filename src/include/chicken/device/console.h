#ifndef C_OS_CONSOLE_H
#define C_OS_CONSOLE_H
#include <stdint.h>
#include <stddef.h>
#include <termios.h>

enum console_color {
	BLACK, BLUE, GREEN, CYAN,
	RED, MAGENTA, BROWN, LT_GREY,
	DK_GREY,LT_BLUE,LT_GREEN, LT_CYAN,
	LT_RED, LT_MAGENTA, LT_BROWN, WHITE

};

struct console;
typedef struct console console_t;


typedef void (*console_putc_fn)(console_t *, int c);
typedef void (*console_switch_fn)(console_t *);

struct console {
	uint32_t x,y;
	uint32_t w,h;
	int num;
	int foreground;
	struct termios termios;
	console_putc_fn putc;
	console_switch_fn switch_fn;
	void *aux;
};
#define NUM_CONSOLES 3


/*
void console_init();
*/
void console_fs_init();
/*
void console_clear();
void console_set_color(uint8_t bg_color, uint8_t fg_color);*/
void console_putc(int c);
void console_putchar(int c);
int console_puts(char *string);
void console_switch(int num);
void console_register(console_t *console);
void serial_putc(char c);
int serial_puts(char *string);
/* device/kbd.c */
int kbd_getc();
void kbd_init();
#endif
