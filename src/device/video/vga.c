#include <common.h>
#include <stdint.h>
#include <hw.h>
#include <memory.h>
#include <device/tty.h>
#include <device/video.h>
#include <device/video/vga.h>

#define EGA_TEXT_BASE 0xC00B8000
#define EGA_TEXT_LENGTH 80*25*2

struct vga_state {
	uint16_t *videoram;
	union {
		uint16_t attribute;
		struct {
			uint8_t padding;
			uint8_t fg:4, bg:4;
		};
	};
} vga_state_storage;

void vga_set_char_xy(struct vga_state *state, uint32_t x, uint32_t y, int c)
{
	// TODO: Should we handle special values of c here?
	state->videoram[x + y*80] = state->attribute | (c & 0xFF);
}

void vga_set_cursor_position(void *aux UNUSED , uint32_t x, uint32_t y)
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

void vga_clear_rows(struct vga_state *state, int row, int n)
{
	kmemsetw(&state->videoram[row*80], state->attribute, 80 * n);
}

void vga_clear(struct vga_state *state)
{
	kmemsetw(state->videoram, BLANK, EGA_TEXT_LENGTH/2);
}

void vga_set_row(struct vga_state *state, int row, const char *contents)
{
	for (int i = 0; i < 80; i++) {
		state->videoram[row * 80 + i] = state->attribute | contents[i];
	}
}

void vga_set_graphics_mode(struct vga_state *state, uint8_t ansi_graphics_mode)
{
	if (ansi_graphics_mode == ANSI_CLEAR) {
		state->fg = VGA_WHITE;
		state->bg = VGA_BLACK;
		return;
	}

	if (ansi_graphics_mode <= ANSI_CONCEALED) {
		// TODO: We could implement useless things like ANSI_BLINK here
		return;
	}

	if (ansi_graphics_mode >= ANSI_FG_BLACK && ansi_graphics_mode <= ANSI_BG_WHITE) {
		uint8_t color = 0;
		uint8_t mode = ansi_graphics_mode - (ansi_graphics_mode >= ANSI_BG_BLACK ? 40 : 30);
		switch (mode) {
			case ANSI_BLACK:	color = VGA_BLACK;	break;
			case ANSI_BLUE:		color = VGA_BLUE;	break;
			case ANSI_CYAN:		color = VGA_CYAN;	break;
			case ANSI_GREEN:	color = VGA_GREEN;	break;
			case ANSI_MAGENTA:	color = VGA_MAGENTA;break;
			case ANSI_RED:		color = VGA_RED;	break;
			case ANSI_WHITE:	color = VGA_WHITE;	break;
			case ANSI_YELLOW:	color = VGA_YELLOW;	break;
			// XXX: Should probably fail silently, but we'll panic for now for debugging's sake
			default: PANIC("This shouldn't happen");break;
		}

		if (ansi_graphics_mode <= ANSI_FG_WHITE) {
			state->fg = color;
		} else {
			state->bg = color;
		}
	}
}

void vga_scroll(struct vga_state *state)
{
	// Since we write to lower addresses we can just memcpyw the while screen
	kmemcpyw(state->videoram, &state->videoram[80], 80*24);
	
	// blank last line 
	kmemsetw(&state->videoram[24*80], BLANK, 80);
}

struct tty_ops vga_ops = {
	.aux = &vga_state_storage,
	.set_cursor_xy = (void *)vga_set_cursor_position,
	.set_char_xy = (void *)vga_set_char_xy,
	.set_row = (void *)vga_set_row,
	.set_graphics_mode = (void *)vga_set_graphics_mode,
	.clear_rows = (void *)vga_clear_rows,
	.clear_screen = (void *)vga_clear,
	.scroll = (void *)vga_scroll
};

void vga_init(struct kernel_boot_info *info UNUSED)
{
	extern struct tty_ops *global_ops;
	global_ops = &vga_ops;
	vga_state_storage.videoram = (uint16_t *)EGA_TEXT_BASE;
	vga_state_storage.fg = VGA_WHITE;
	vga_state_storage.bg = VGA_BLACK;
	vga_clear(&vga_state_storage);
}