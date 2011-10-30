#ifndef C_OS_CONSOLE_H
#define C_OS_CONSOLE_H
#include <stdint.h>
void console_init();
void console_clear();
void console_putc(uint8_t c);
int console_puts(char *string);
#endif
