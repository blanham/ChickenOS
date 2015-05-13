#include <common.h>
#include <hw.h>
#include <stdint.h>
#include <memory.h>
#include <device/video.h>
#include <device/video/vga.h>

#define EGA_TEXT_BASE 0xC00B8000
#define EGA_TEXT_LENGTH 80*25*2
uint16_t *videoram = (uint16_t *)EGA_TEXT_BASE;
struct new_vga_state {
	uint16_t *videoram;
	uint16_t *buffer;
	int x, y;
	int h, w;	
	union {
		uint16_t attribute;
		uint8_t fg, bg;
	};
	int escape;
	char ansibuf[32];
	int bufpos;
};

void vga_putchar(int c, int x, int y)
{
	videoram[x + y *80] = BLANK | c;
	//vga_set_cursor_position(x, y);
}

void vga_set_cursor_position(uint32_t x, uint32_t y)
{
	if(x > 0xFF)
		x = 0xFF;
	if(y > 0xFF)
		y = 0xFF;
	uint16_t pos = x + y * 80;
	outb(0x3D4, 14); //cursor highbyte register selected
	outb(0x3D5, pos >> 8);
	outb(0x3D4, 15); //cursor lowbyte register selected
	outb(0x3D5, pos & 0xFF);
}

void vga_clear_rows(int row, int n)
{
	kmemsetw(&videoram[row *80], BLANK, 80 * n);
}
void vga_put_line(int row, const char *line)
{
	uint8_t *ram_line = (uint8_t *)videoram;//[row *80*2] ;
	ram_line += (row * 80 * 2) + 0;
	for(int i = 0; i < 80; i++)
	{
		*ram_line++ = *line++; 
		*ram_line++ = BLANK >> 8;//*line++; 
	//	ram_line += 2;
	}
}

void vga_scroll()
{
	// move lines up one 
	for(int i = 0; i < 24; i++)
	{
		kmemcpyw(&videoram[i*80],&videoram[(i + 1)*80], 80);
	}
	// blank last line 
	kmemsetw(&videoram[24*80], BLANK, 80);
}

void new_vga_setcolors(int fg, int bg)
{

	(void)fg;
	(void)bg;

}

void new_vga_init(struct kernel_boot_info *info)
{
	(void)info;
	kmemset(videoram, 0x00, EGA_TEXT_LENGTH);
}
