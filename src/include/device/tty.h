#ifndef C_OS_DEVICE_TTY_H
#define C_OS_DEVICE_TTY_H
#include <stdint.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <errno.h>
#include <arch/i386/serial.h>
#include <chicken/boot.h>
struct tty_state;
typedef struct tty_state tty_t;
void tty_init(struct kernel_boot_info *info);
void tty_putchar(tty_t *tty, int c);
void tty_termios_print(struct termios *termios);
#endif
