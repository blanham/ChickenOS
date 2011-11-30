#ifndef C_OS_STDIO_H
#define C_OS_STDIO_H
#include <stdarg.h>
void putc(char c);
int puts(char *string);
char *gets(char *str);
char getchar();
int vsprintf(char *buf, const char *fmt, __gnuc_va_list args);
void printf(char *fmt, ...);
#endif
