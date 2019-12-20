#ifndef C_OS_DEVICE_TTY_H
#define C_OS_DEVICE_TTY_H
#include <stdint.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <errno.h>
#include <chicken/boot.h>

enum ANSI_MODES {
	ANSI_CLEAR,
	ANSI_BOLD,
	ANSI_UNDERSCORE,
	ANSI_BLINK,
	ANSI_REVERSE_VIDEO,
	ANSI_CONCEALED,
	ANSI_BLACK = 0,
	ANSI_RED,
	ANSI_GREEN,
	ANSI_YELLOW,
	ANSI_BLUE,
	ANSI_MAGENTA,
	ANSI_CYAN,
	ANSI_WHITE,
	ANSI_FG_BLACK = 30,
	ANSI_FG_RED,
	ANSI_FG_GREEN,
	ANSI_FG_YELLOW,
	ANSI_FG_BLUE,
	ANSI_FG_MAGENTA,
	ANSI_FG_CYAN,
	ANSI_FG_WHITE,
	ANSI_BG_BLACK = 40,
	ANSI_BG_RED,
	ANSI_BG_GREEN,
	ANSI_BG_YELLOW,
	ANSI_BG_BLUE,
	ANSI_BG_MAGENTA,
	ANSI_BG_CYAN,
	ANSI_BG_WHITE,
};

struct tty_ops {
	void *aux;
	void (*set_cursor_xy)(void *,uint32_t, uint32_t);
	void (*set_char_xy)(void *, uint32_t, uint32_t, int);
	void (*set_row)(void *, int row, const char *line);
	void (*set_graphics_mode)(void *, uint8_t);
	void (*clear_rows)(void *, uint32_t, size_t);
	void (*clear_screen)(void *);
	void (*scroll)(void *);
};

struct tty_state;
typedef struct tty_state tty_t;

void tty_init(struct kernel_boot_info *info);
void tty_putchar(tty_t *tty, int c);
void tty_putc(int c);
void tty_termios_print(struct termios *termios);

#endif