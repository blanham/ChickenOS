#include <common.h>
#include <stdarg.h>
#include <stdio.h>
//TODO:
//	always outputs to serial
//	outputs to console until we hit init
//	in memory log for dmesg functionality
void kprintf(char *fmt, ...)
{
	va_list args;
	
	va_start(args, fmt);
//	i=linux_vsprintf(buf,fmt,args);
	va_end(args);

	
}
