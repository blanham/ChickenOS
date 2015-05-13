#ifndef C_OS_DEVICE_VGA_H
#define C_OS_DEVICE_VGA_H
#include <device/console.h>
#include <chicken/boot.h>
#include <fs/vfs.h>
//#define 640_480_TEXT 0x0001
#define BLANK 0x0700


console_t * vga_init();
void new_vga_init(struct kernel_boot_info *info);
void vga_putchar(int c, int x, int y);
void vga_clear_rows(int row, int n);
void vga_scroll();


void vga_put_line(int row, const char *line);
void vga_clear_rows(int row, int n);
void vga_set_cursor_position(uint32_t x, uint32_t y);
//int vga_switch_mode(int mode);


#endif
