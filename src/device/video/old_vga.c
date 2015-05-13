#include <common.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <mm/liballoc.h>
#include <kernel/hw.h>
#include <device/console.h>
#include <device/video/vga.h>
#include <mm/vm.h>

/*
void vga_init(struct multiboot_info *mb)
{
	struct vbe_controller_info *info = (void *)P2V(mb->vbe_control_info);
	printf("location: %X\n",info);
	for(int i =0; i < 4; i++)
		printf("%c", info->signature[i]);
	printf("\n");
	//printf("%s\n",(info->signature));	
	//uintptr_t test = info->oem_string;
	//pagedir_insert_page(kernel_pd, test, test,0x7);
	//printf("Modes %s\n",(test));
}
*/
/*
struct vga_state {
	uint16_t *videoram;
	uint16_t *buffer;
	int x, y;
	int h, w;	
	uint16_t attribute;
	int escape;
	char ansibuf[32];
	int bufpos;
};


void vga_set_cursor_position(uint32_t x, uint32_t y)
{
	if(x > 0xFF)
		x = 0xFF;
	if(y > 0xFF)
		y = 0xFF;
	outb(0x3D4, 14); //cursor highbyte register selected
	outb(0x3D5, x);
	outb(0x3D4, 15); //cursor lowbyte register selected
	outb(0x3D5, y);
}

void vga_put_character(int c)
{
	(void)c;

}

static  uint16_t * videoram = (uint16_t *) 0xc00b8000;
#define CURSOR_POS (state->x + state->y*80)

static void vga_cursor_move(uint16_t pos)
{
	outb(0x3D4, 14); //cursor highbyte register selected
	outb(0x3D5, pos >> 8);
	outb(0x3D4, 15); //cursor lowbyte register selected
	outb(0x3D5, pos & 0xFF);
}
void vga_set_color(console_t *con, uint8_t bg_color, uint8_t fg_color)
{
	struct vga_state *state = (struct vga_state *)con->aux;
	state->attribute = (bg_color << 12) | (fg_color << 8) ;
}

static void vga_scroll(console_t *con)
{
	struct vga_state *state = (struct vga_state *)con->aux;

//	ASSERT((uintptr_t)con->videoram > PHYS_BASE, "Invalid console videoram pointer!\n");
	
	// move lines up one 
	for(int i = 0; i < 24; i++)
	{
		kmemcpyw(&state->videoram[i*80],&state->videoram[(i + 1)*80], 80);
	}
	// blank last line 
	kmemsetw(&state->videoram[24*80], BLANK, 80);

	state->y = 24;
}

void vga_clear(console_t *con)
{
	struct vga_state *state = (struct vga_state *)con->aux;
	kmemsetw(state->videoram, BLANK, 80*25);
	state->y = state->x = 0;
	vga_cursor_move(0);
}

void vga_switch(console_t *con)
{
	struct vga_state *state = (struct vga_state *)con->aux;
	(void)state;

}

void vga_putc_internal(console_t *con, int c);
void vga_putc(console_t *con, int c);
void vga_escape(console_t *con, int c)
{
	struct vga_state *state = (struct vga_state *)con->aux;
	(void)c;
	char *p;
	char *d;
	switch(state->escape)
	{
		case 1:
			if(c == '[')
				state->escape = 2;
			else
				state->escape = 0;
			break;			
		case 2:
			switch(c)
			{
				case 0 ... 63:
					state->ansibuf[state->bufpos++] = c & 0xff;
					break;
				case 'H':
					state->ansibuf[state->bufpos] = 0;
					p = (char *)&state->ansibuf;
					int x = strtol(state->ansibuf, &p, 10);
					p++;
					int y = strtol(p, &d, 10);
					state->x = y-1;
					state->y = x-1;	
					vga_cursor_move(CURSOR_POS);
					state->escape = 0;
					break;
				case 'J':
					vga_clear(con);
					state->escape = 0;
					break;
				case 'K':
					//kmemsetw(&state->videoram[state->x*80], BLANK, 80);
					state->escape = 0;
					break;
				default:
					state->escape = 0;
			//vga_putc_internal(con, c);	
					break;

			}
			break;
		default:
			state->escape = 0;

			vga_putc_internal(con, '?');	


	}
}
void vga_putc_internal(console_t *con, int c)
{
	struct vga_state *state = (struct vga_state *)con->aux;
	
	state->videoram[CURSOR_POS] = state->attribute | c;
	state->x++;



}
void vga_putc(console_t *con, int c)
{
	struct vga_state *state = (struct vga_state *)con->aux;
//	enum intr_status old = interrupt_disable();

	if(state->x >= 80) 
	{
		state->x = 0;
		state->y++;
	}
	if(state->y >= 25)
	{
		vga_scroll(con);
	//	console.y = 0;
	}
	
//	if(con == console)
//	interrupt_set(old);
	//All of this should be handled in tty/console driver
	//Just have a function for cursor positioning
	//and one for putting characters in certain spots
	if(state->escape)
	{
		vga_escape(con, c);
		return;
	}
	state->bufpos = 0;
	switch(c)
	{
		//FIXME Doesn't handle backspace properly
		case 0x08:
			if(state->x > 0)
				state->x--;
			else if(state->y > 0)
				state->y--;
			//state->videoram[CURSOR_POS] = state->attribute | 0;
			break;
		//case 0x29://tab
		case '\t'://tab
			state->x = (state->x + 8) & ~(7);
			break;
		case '\n':
			state->y++;
			state->x = 0;
			break;
		case '\r':
			state->x = 0;
			break;
		case 0x1b:
			state->escape = 1;
			break;	

		default:
			//bochs_vga_putc(c, con->x, con->y);
			state->videoram[CURSOR_POS] = state->attribute | c;
			state->x++;
	}
	vga_cursor_move(CURSOR_POS);
}


int tty_puts(int num, char *string)
{
	int cnt = 0;
	(void)num;
	while(*string != '\0')
	{
	//	tty_putc(consoles[num],*string++);
		cnt++;
	}

	return cnt;
}


void vga_console_init(console_t *con, int num UNUSED)
{
	
	struct vga_state *state = (struct vga_state *)con->aux;

	state->x = state->y = 0;
	state->attribute = BLANK;
	//allocates a page for now, works fine
	//for 80x25 screens
	state->buffer = (uint16_t *)palloc();
//	if(num)
		state->videoram = state->buffer;
//	else
	//	state->videoram = videoram;
//	state->num = num;
//	if(num == 0)
		state->videoram = videoram;
	state->escape = 0;
	state->bufpos = 0;
}

void console_init(console_t *con)
{
//	for(int i = 0; i < NUM_CONSOLES; i++)
//		console_init_one(consoles[i],i);
//	console->videoram = videoram;
	//console_clear();
(void)con;	
	//kbd_init();
}



console_t *vga_init()
{
	console_t* new = kcalloc(sizeof(*new), NUM_CONSOLES);


	for(int i = 0; i < NUM_CONSOLES; i++)
	{
		new[i].putc = &vga_putc;
		new[i].aux = kcalloc(sizeof(struct vga_state),1);
		vga_console_init(&new[i], i);
		console_register(&new[i]);	
			
	}
	vga_clear(new);
	return new;
}
*/
