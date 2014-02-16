#ifndef C_OS_DEVICE_VGA_H
#define C_OS_DEVICE_VGA_H
#include <device/console.h>
#include <chicken/boot.h>
#include <fs/vfs.h>
//#define 640_480_TEXT 0x0001
#define BLANK 0x0700


console_t * vga_init();
void new_vga_init(struct kernel_boot_info *info);

//int vga_switch_mode(int mode);


#endif
