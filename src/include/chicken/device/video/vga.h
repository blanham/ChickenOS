#ifndef C_OS_DEVICE_VGA_H
#define C_OS_DEVICE_VGA_H
#include <chicken/boot.h>

//#define 640_480_TEXT 0x0001
#define BLANK 0x0700

enum vga_colors {
    VGA_BLACK,
    VGA_BLUE,
    VGA_GREEN,
    VGA_CYAN,
    VGA_RED,
    VGA_MAGENTA,
    VGA_BROWN,
    VGA_LIGHT_GRAY,
    VGA_DARK_GRAY,
    VGA_LIGHT_BLUE,
    VGA_LIGHT_GREEN,
    VGA_LIGHT_CYAN,
    VGA_LIGHT_RED,
    VGA_PINK,
    VGA_YELLOW,
    VGA_WHITE
};

// FIXME: Should this return a tty handle of some sort?
void vga_init(struct kernel_boot_info *info);

#endif