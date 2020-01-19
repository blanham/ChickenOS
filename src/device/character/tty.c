#include <common.h>
#include <memory.h>
#include <stdbool.h>
#include <stdlib.h>
#include <termios.h>
#include <device/tty.h>
#include <fs/vfs.h>
#include <mm/liballoc.h>
#include <mm/vm.h>

#define TTY_HISTORY_SIZE 128 // MUST BE POWER OF TWO!
#define TTY_HISTORY_MASK (TTY_HISTORY_SIZE - 1)
#define TTY_HISTORY_ROW_LENGTH 80
#define TTY_COUNT 4

// FIXME: This should be set through a function or something
struct tty_ops *global_ops = NULL;

struct tty_state {
	struct winsize winsize;
	struct termios termios;
	pid_t pgrp;
	bool foreground;

	// ANSI escape state
	int escape;
	char ansibuf[32];
	int bufpos;

	struct ringbuffer {
		char **buf;
		int r_top;
		int r_cur;
		int x, y;
	} ringbuffers[2];
	struct ringbuffer *cur_ring;
	int ring_number;

	// FIXME: this should use the ones in the ringbuffer struct
	int x, y;
} *ttys[TTY_COUNT];

char tty_history[TTY_COUNT][2][TTY_HISTORY_SIZE][TTY_HISTORY_ROW_LENGTH];

#define TTY_GET_ROW_PTR(row) tty->cur_ring->buf[(tty->cur_ring->r_top + row) & 0x7F]
#define TTY_INSERT_BUF(tty, c) ((tty)->cur_ring->buf[((tty)->cur_ring->r_top + (tty)->y) & 0x7f][(tty)->x] = c)

void tty_redraw(tty_t *tty)
{
	for (unsigned i = 0; i < tty->winsize.ws_row; i++)
		global_ops->set_row(global_ops->aux, i, TTY_GET_ROW_PTR(i));
}

void tty_clear_rows(tty_t *tty, int row, int count)
{
	for (int i = row; i < count; i++)
		kmemset(TTY_GET_ROW_PTR(i), 0, tty->winsize.ws_col);

	printf("COUNT %i\n", count);
	global_ops->clear_rows(global_ops->aux, row, count);
}

void tty_scroll(tty_t *tty)
{
	// XXX: Should I make a wrapper for this?
	kmemset(tty->cur_ring->buf[tty->cur_ring->r_top & (TTY_HISTORY_SIZE-1)], 0, 80);
	tty->cur_ring->r_top++;

	global_ops->scroll(global_ops->aux);
	global_ops->set_row(global_ops->aux, tty->y, TTY_GET_ROW_PTR(tty->y));
}

void tty_escape(tty_t *tty, int c)
{
	int cmd = 0;
	if (tty->escape == 1) {
		tty->bufpos = 0;
		// TODO: Apparently '[' isn't the only escape
		if (c == '[')
			tty->escape = 2;
		else
			tty->escape = 0;
		return;
	}

	switch(c)
	{
		case 0 ... 63: // XXX: Is this range right?
			// TODO: Why not handle cmd parsing as we get bytes in?
			tty->ansibuf[tty->bufpos++] = c & 0xff;
			return;
		case 'C': // Cursor forward
			tty->ansibuf[tty->bufpos] = 0;
			cmd = strtol(tty->ansibuf, NULL, 10);
			serial_printf("Cursor %i\n", cmd);
			tty->x += cmd;
			break;
		case 'l': // Reset mode
		case 'h': // Set mode
			if (tty->ansibuf[0] != '?') {
				serial_printf("Unhandled tty mode ('?h') command %X\n", cmd);
				break;
			}

			tty->ansibuf[tty->bufpos] = 0;
			cmd = strtol(tty->ansibuf + 1, NULL, 10);

			if (cmd != 1049) {
				serial_printf("Unhandled tty mode ('h') command %X\n", cmd);
				break;
			}

			tty->cur_ring->x = tty->x;
			tty->cur_ring->y = tty->y;
			if (tty->ring_number) {
				tty->cur_ring = &tty->ringbuffers[0];
				tty->ring_number = 0;
			} else {
				tty->cur_ring = &tty->ringbuffers[1];
				tty->ring_number = 1;
			}
			tty->x = tty->cur_ring->x;
			tty->y = tty->cur_ring->y;

			// FIXME: Since we don't save color info we lose it here
			//		  Fixing it would likely require significant restructuring
			//		  Or we might have to require the driver to have an alternate buffer too
			tty_redraw(tty);
			break;
		case 'H': // Set cursor to 0,0 or x,y
			if (tty->bufpos == 0) {
				tty->x = 0;
				tty->y = 0;
			} else {
				char *p, *d;
				tty->ansibuf[tty->bufpos] = 0;
				p = (char *)&tty->ansibuf;
				int x = strtol(tty->ansibuf, &p, 10);
				p++;
				int y = strtol(p, &d, 10);
				tty->x = y-1;
				tty->y = x-1;
			}
			global_ops->set_cursor_xy(global_ops->aux, tty->x, tty->y);
			break;
		case 'J': // Clear screen from cursor
			cmd = 0; // No cmd # and cmd #0 do the same thing
			if (tty->bufpos > 0) {
				tty->ansibuf[tty->bufpos] = 0;
				cmd = strtol(tty->ansibuf, 0, 10);
			}

			//printf("WINSIZE: %x %x %x %x\n", tty->bufpos, cmd, tty->winsize.ws_row, tty->y);

			switch (cmd) {
				case 0:
					tty_clear_rows(tty, tty->y, tty->winsize.ws_row - tty->y);
					break;
				case 1:
					tty_clear_rows(tty, 0, tty->y);
					break;
				case 2:
					tty_clear_rows(tty, 0, tty->winsize.ws_row);
					break;
			}
			break;
		//case 'd': // Move cursor to indicted row, current column
		//	{
		//		char *p;
		//		tty->ansibuf[tty->bufpos] = 0;
		//		p = (char *)&tty->ansibuf;
		//		int x = strtol(tty->ansibuf, &p, 10);
		//		//printf("XX%i\n", x);
		//		global_ops->set_cursor_xy(global_ops->aux, x, tty->y);
		//	}
		//	break;
		case 'K': // Clear line from cursor
			if (tty->bufpos > 0) {
				tty->ansibuf[tty->bufpos] = 0;
				cmd = strtol(tty->ansibuf, 0, 10);
			}

			char *row_ptr = TTY_GET_ROW_PTR(tty->y);
			size_t count = TTY_HISTORY_ROW_LENGTH;
			switch (cmd) {
				case 0: // Command 0 and no command do the same thing
					count = TTY_HISTORY_ROW_LENGTH - tty->x;
					row_ptr = row_ptr + tty->x;
					break;
				case 1:
					count = tty->x;
					break;
			}

			kmemset(row_ptr, ' ', count);
			global_ops->set_row(global_ops->aux, tty->y, TTY_GET_ROW_PTR(tty->y));
			break;
		case 'M':

		case 'm': // Set graphics mode
			{
				char *p, *d;
				tty->ansibuf[tty->bufpos] = 0;
				p = (char *)&tty->ansibuf;
				int x = strtol(tty->ansibuf, &p, 10);
				p++;
				int y = strtol(p, &d, 10);
				global_ops->set_graphics_mode(global_ops->aux, y);
				global_ops->set_graphics_mode(global_ops->aux, x);
			}
			break;
		default:
			serial_printf("Unhandled ANSI code %x %c\n",c,c);
			break;
	}

	tty->escape = 0;
}


// NOTE: This is used by the kernel printf
void tty_putc(int c)
{
	tty_t *tty = ttys[0];
	tty_putchar(tty, c);
}

void tty_putchar(tty_t *tty, int c)
{
	// XXX: Is this really the best place for these checks?
	if (tty->x >= tty->winsize.ws_col) {
		tty->x = 0;
		tty->y++;
	}

	if (tty->y >= tty->winsize.ws_row) {
		tty->y = tty->winsize.ws_row - 1;
		tty->x = 0;
		tty_scroll(tty);
	}

	if (tty->escape) {
		tty_escape(tty, c);
		return;
	}

	switch (c) {
		// FIXME Doesn't handle backspace properly
		case '\b': // 0x8 - Backspace
			if (tty->x > 0)
				tty->x--;
			else if (tty->y > 0)
				tty->y--;
			TTY_INSERT_BUF(tty, ' ');
			global_ops->set_char_xy(global_ops->aux, tty->x, tty->y, ' ');
			break;
		case '\t': // 0x09 - Horizontal Tab
			tty->x = (tty->x + 8) & ~(7); // Clever
			break;
		case '\n': // 0x0A - Newline
			tty->x = 0;
			tty->y++;
			break;
		case '\r': // 0x0D - Carraige Return
			tty->x = 0;
			break;
		case '\a': // 0x07 - Bell
			break;
		case '\e': // 0x1B - Escape sequence start
			tty->escape = 1;
			break;
		default:
			TTY_INSERT_BUF(tty, c);
			global_ops->set_char_xy(global_ops->aux, tty->x, tty->y, c);
			tty->x++;
	}

	global_ops->set_cursor_xy(global_ops->aux, tty->x, tty->y);
}

// FIXME: Should read from pipe
int tty_getc(tty_t *tty, bool block)
{
	int c = -1;
	// FIXME: handle background tty properly
	while (tty->foreground == false);
	extern int kbd_getc();
	while (c == -1 && block)
		c = kbd_getc();
	return c;
}

size_t tty_getline(tty_t *tty, uint8_t *buf, size_t len)
{
	size_t ret = 0;
	while (len--) {
		int c = tty_getc(tty, true);

		if (c == 0x8) {
			buf[ret] = '\0';
			if ((signed)ret > 0) {
				ret--;
				if(tty->termios.c_lflag & ECHO)
					tty_putchar(tty, c);
			}
		} else {
			if(tty->termios.c_lflag & ECHO)
				tty_putchar(tty, c);

			buf[ret] = c;
			ret++;
		}

		if(c == 0xa)
			break;
	}
	return ret;
}

//int tty_read(struct inode *inode, void *_buf, off_t off UNUSED, size_t count)?
// TODO: Implement more termios checking/functionality
size_t tty_read(struct inode *inode, uint8_t *buf, size_t count, off_t offset UNUSED)
{
	(void)inode;
	//dev_t dev = inode->info.st_rdev;
	size_t ret = 0;
	int tty_num = 0;//MINOR(dev) - 1;

	//if (MAJOR(dev) == 0x3)
	//	tty_num = 0;
	tty_t *tty = ttys[tty_num];

	uint32_t vmin = (unsigned)tty->termios.c_cc[VMIN];
	//int time = tty->termios.c_cc[VTIME];

	if (tty->termios.c_lflag & ICANON)
		return tty_getline(tty, buf, count);

	while (count--) {
		int c = tty_getc(tty, true);
		if (c < 0 && count < vmin)
			continue;

		if (tty->termios.c_lflag & ECHO)
			tty_putchar(tty, c);

		if (c == 0xa) {
			if ((tty->termios.c_iflag & INLCR) == 0) {
			//	c = '\r';
			}
		}
		buf[ret] = c;
		ret++;

		if (ret >= (unsigned)vmin)
			break;
	}

	//serial_printf("\nReading %x:%c %i %i\n", ((char*)_buf)[0], ((char *)_buf)[0], count, offset);
	return ret;
}

// XXX: Might be worth moving the putchar logic here instead of iterating over buffer
size_t tty_write(struct inode *inode, uint8_t *_buf, size_t count, off_t offset UNUSED)
{
	(void)inode;
	//dev_t dev = inode->info.st_rdev;
	int tty_num = 0;//MINOR(dev) - 1;
	//if (MAJOR(dev) == 0x3)
	//	tty_num = 0;
	//if (MAJOR(dev) == 0x4)
	//	tty_num = 0;
	const char *p = (const char *)_buf;

	while (count--) {
		serial_putc(*p);
		tty_putchar(ttys[tty_num], *p++);
	}

	return p - (const char *)_buf;
}

// XXX: This is probably just temporary:
#define ENOIOCTLCMD 515

int tty_ioctl(struct inode *inode, int request, char *args)
{
	(void)inode;
	//dev_t dev = inode->info.st_dev;
	int tty_num = 0;//MINOR(dev) - 1;
	//if (MAJOR(dev) == 0x3)
	//	tty_num = 0;
	tty_t *tty = ttys[tty_num];

	// These don't use arguments, so process them first
	switch (request) {
		// TODO: implement these IOCTLs if needed
		case TIOCSBRK:
		case TIOCCBRK:
		case TIOCCONS:
		case TIOCNOTTY:
		case TIOCEXCL:
		case TIOCNXCL:
			serial_printf("Unimplemented tty ioctl %.4X\n", request);
			return -ENOIOCTLCMD;
	}

	//printf("REQ %x args %x\n", request, args);

	if (args == NULL || (uintptr_t)args >= PHYS_BASE)
		return -EFAULT;

	// FIXME: Do we need to lock the termios when r/w it?
	switch (request) {
		case TCGETS:
			kmemcpy(args, &tty->termios, sizeof(struct termios));
			break;
		case TCSETSW:
			// FIXME: TCSETSW should allow the output buffer to drain
		case TCSETSF:
			// FIXME: TCSETSF should allow the output buffer to drain, and discard pending input
		case TCSETS:
			kmemcpy(&tty->termios, args, sizeof(struct termios));
			break;
		case TIOCSWINSZ:
			// FIXME: This should send SIGWINCH to foreground process group
			kmemcpy(&tty->winsize, args, sizeof(struct winsize));
			break;
		case TIOCGWINSZ:
			kmemcpy(args, &tty->winsize, sizeof(struct winsize));
			break;
		case TIOCGPGRP:
			*(int *)args = tty->pgrp;
			break;
		case TIOCSPGRP:
			tty->pgrp = *(int *)args;
			break;
		default:
			serial_printf("Unimplemented tty ioctl %.4X\n", request);
			return -ENOIOCTLCMD;
	}

	return 0;
}

// TODO: Verify that these make sense
struct termios default_termios = {
	.c_iflag = ICRNL,
	.c_oflag = OPOST | ONLCR,
	.c_cflag = 0,
	.c_lflag = IXON | ISIG | ICANON | ECHO | ECHOCTL | ECHOKE,
	.c_line = 0,
	.c_cc = {
		0x03, 0x1C, 0x7F, 0x15,
		0x04, 0x00, 0x01, 0x00,
		0x11, 0x13, 0x1A, 0xFF,
		0x12, 0x0F, 0x17, 0x16,
		0xFF
		},
};

void termios_init(struct termios *termios)
{
	kmemcpy(termios, &default_termios, sizeof(*termios));

	termios->c_cflag |= B38400;
}

void tty_init(struct kernel_boot_info *info UNUSED)
{
	for (int i = 0; i < TTY_COUNT; i++) {
		ttys[i] = kcalloc(sizeof(tty_t), 1);
		ttys[i]->pgrp = 0;

		termios_init(&ttys[i]->termios);
		ttys[i]->winsize.ws_col = info->x_chars;
		ttys[i]->winsize.ws_row = 25;//info->y_chars;

		ttys[i]->ringbuffers[0].buf = kcalloc(TTY_HISTORY_SIZE, sizeof(char *));
		ttys[i]->ringbuffers[1].buf = kcalloc(TTY_HISTORY_SIZE, sizeof(char *));

		for (unsigned row = 0; row < TTY_HISTORY_SIZE; row++) {
			ttys[i]->ringbuffers[0].buf[row] = &tty_history[i][0][row][0];
			ttys[i]->ringbuffers[1].buf[row] = &tty_history[i][1][row][0];
		}

		ttys[i]->cur_ring = &ttys[i]->ringbuffers[0];
	}

	ttys[0]->foreground = true;

	device_register(S_IFCHR, 0x500, tty_read, tty_write, tty_ioctl);
	device_register(S_IFCHR, 0x400, tty_read, tty_write, tty_ioctl);
	device_register(S_IFCHR, 0x800, tty_read, tty_write, tty_ioctl);
}