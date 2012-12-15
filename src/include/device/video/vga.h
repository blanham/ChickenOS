#ifndef C_OS_DEVICE_VGA_H
#define C_OS_DEVICE_VGA_H
#include <device/console.h>
#include <fs/vfs.h>
//#define 640_480_TEXT 0x0001
#define BLANK 0x0700


console_t * vga_init();

//int vga_switch_mode(int mode);


#endif
