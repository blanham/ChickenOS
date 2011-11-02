#include <kernel/memory.h>
#include <kernel/console.h>
#include <kernel/hw.h>
#define NULL 0
static  uint16_t * videoram = (uint16_t *) 0xb8000;
#define BLANK 0x0700
/* some stuff that will later be used to setup mutliple consoles */
#define NUM_CONSOLES 3
#define CURRENT_CONSOLE consoles[current_console]
typedef struct {
	uint16_t *videoram;
	uint8_t x,y;
	uint16_t attribute;
} console_t;
console_t console0;
console_t console1;
console_t console2;
console_t *consoles[NUM_CONSOLES] = {&console0, &console1, &console2};
console_t *console = &console0;
#define CURSOR_POS (console->x + console->y*80)
static void console_cursor_move(uint16_t pos)
{
	outb(0x3D4, 14); //cursor highbyte register selected
	outb(0x3D5, pos >> 8);
	outb(0x3D4, 15); //cursor lowbyte register selected
	outb(0x3D5, pos & 0xFF);
}
/* rewrite this later with memcpy and memset */
static void console_scroll()
{
	/* move lines up one */
	for(int i = 0; i < 24; i++)
	{
		kmemcpyw(&console->videoram[i*80],&console->videoram[(i + 1)*80], 80);
	}
	/* blank last line */
	kmemsetw(&console->videoram[24*80], BLANK, 80);

	console->y = 24;
}
void console_putc(uint8_t c)
{
	switch(c)
	{
		case 0x08:
			if(console->x != 0)
				console->x--;
			break;
		case 0x29://tab
		case '\t'://tab
			console->x = (console->x + 8) & ~(7);
			break;
		case '\n':
			console->y++;
		case '\r':
			console->x = 0;
			break;
		default:
			console->videoram[CURSOR_POS] = 0x0700 | c;
			console->x++;
	}
	if(console->x >= 80) 
	{
		console->x = 0;
		console->y++;
	}
	if(console->y >= 25)
	{
		console_scroll();
	//	console.y = 0;
	}
	console_cursor_move(CURSOR_POS);
}
int console_puts(char *string)
{
	int cnt = 0;

	while(*string != NULL)
	{
		console_putc(*string++);
		cnt++;
	}

	return cnt;
}

void console_clear()
{
	kmemsetw(console->videoram, BLANK, 80*25);
	console->y=console->x = 0;
	console_cursor_move(0);
}
void console_init()
{
	console->x = console->y = 0;
	console->videoram = videoram;
	console_clear();
}
/*
void console_switch(int num)
{
	//copy curent vram to buffer

	//copy new consoles buffer to vram
	
	console = consoled[num];

	//put cursor at correct postion
	console_cursor_move(CURSOR_POS);
}
*/
