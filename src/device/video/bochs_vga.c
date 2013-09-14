/*	ChickenOS - device/video/bochs_vga.c
 *	Driver for the Bochs VBE device include in Bochs/QEMU
 */

#include <common.h>
#include <hw.h>
#include <device/video/bochs_vga.h>
#include <device/video/font.h>
#include <device/pci.h>
#include <stdio.h>
#include <mm/vm.h>
#include <kernel/thread.h>
#include <memory.h>
#include <mm/paging.h>
#include <mm/liballoc.h>
#include <device/console.h>
#include <vincent.h>
#include <font2.h>

//FIXME: move this somewhere better
uint32_t *background_image = NULL;

struct bochs_vga_state {
	struct pci_device *pci;
	int x, y;
	int h, w;
	int num;
	uint32_t bg;
	uint32_t fg;
};

uint32_t *framebuffer;

uint16_t bochs_vga_read(uint16_t port)
{
	outw(VBE_DISPI_IOPORT_INDEX, port);

	return inw(VBE_DISPI_IOPORT_DATA);
}

void bochs_vga_write(uint16_t port, uint16_t value)
{
	outw(VBE_DISPI_IOPORT_INDEX, port);
	outw(VBE_DISPI_IOPORT_DATA, value);
}

void bochs_vga_setmode(uint32_t w, uint32_t h, uint32_t bpp, bool linearfb, bool clearmem)
{
	bochs_vga_write(VBE_DISPI_INDEX_ID, 0xB0c4);
	bochs_vga_read(VBE_DISPI_INDEX_ID);
	bochs_vga_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
	bochs_vga_write(VBE_DISPI_INDEX_XRES, w);
	bochs_vga_write(VBE_DISPI_INDEX_YRES, h);
	bochs_vga_write(VBE_DISPI_INDEX_BPP,  bpp);
	
	bochs_vga_write(0x07, 4096);
	bochs_vga_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED | 
		(linearfb ? VBE_DISPI_LFB_ENABLED : 0) |
		(clearmem ? VBE_DISPI_NOCLEARMEM : 0));

}
unsigned char bochs_buf[256];


void bochs_vga_putc_internal(console_t *con, int c)
{
	unsigned int d = c;// - 32;
	struct bochs_vga_state *state = (struct bochs_vga_state *)con->aux;
	uint32_t x = state->x;
	uint32_t y = state->y;	
	uint8_t temp;
	uint32_t *ptr = framebuffer;// +640*480*4*con->num;
	//FIXME: Perhaps unroll these loops?
	for(int i =0; i < 8; i++)
	{
		temp = vincent_font[d][i];;
		 for(int j = 0; j < 8; j++)
		{
			if(0x80 & (temp << j)){
				ptr[x*8 + j + (y*8 +i)*con->w] = 0x00ffffff;//0x00ff0000 ;
			}else{
				ptr[j + i*con->w] |= 0x00;//0x00ff0000 ;
			}
		}
	}	
}

void bochs_vga_scroll(console_t *con)
{
	(void)con;
}

void bochs_vga_putc(console_t *con, int c)
{

	struct bochs_vga_state *state = (struct bochs_vga_state *)con->aux;

	switch(c)
	{
		case 0x08:
			if(state->x > 0)
				state->x--;
			else if(state->y > 0)
				state->y--;
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
		default:
			//bochs_vga_putc(c, con->x, con->y);
		//	state->videoram[CURSOR_POS] = state->attribute | c;
			bochs_vga_putc_internal(con, c);
			state->x++;
	}
	if(state->x >= state->w) 
	{
		state->x = 0;
		state->y++;
	}
	if(state->y >= state->h)
	{
		bochs_vga_scroll(con);
		
	//	console.y = 0;
	}
	
//	interrupt_set(old);


}


uint32_t * bochs_vga_init_internal(int w, int h, int bpp)
{
	struct pci_device *pci;
	uint32_t *ptr;
	uint32_t bar;
	thread_t *cur = thread_current();
	if((pci = pci_get_device(0x1234, 0x1111)) != NULL)
	{
			bar = pci_get_bar(pci, 0);
			ptr = framebuffer = (uint32_t *)(bar & ~0xfff);
	}else{

			ptr = framebuffer = (uint32_t *)(0xe0000000 & ~0xfff);
	//	return NULL;
	}
	uint32_t size = w*h*(bpp / 4)*10 / 4096;
	pagedir_insert_pagen_physical(cur->pd, (uintptr_t)framebuffer, (uintptr_t)framebuffer, 0x7, size);
	pagedir_install(cur->pd);
	bochs_vga_setmode(w, h, bpp, true, true);
	kmemset(framebuffer, 0x0, h*w*(bpp/4));

	return ptr;
}

void bochs_vga_switch(console_t *con)
{
//	struct bochs_vga_state *state = (struct bochs_vga_state *)con->aux;
	bochs_vga_write(VBE_DISPI_INDEX_X_OFFSET, 0);
	bochs_vga_write(VBE_DISPI_INDEX_Y_OFFSET, con->num*480);
}

console_t * bochs_vga_init()
{
	console_t *new = (console_t *)kcalloc(sizeof(*new), NUM_CONSOLES);
	struct bochs_vga_state *state;// = kcalloc(sizeof(*state), 1);
	uint16_t test;
	uint32_t *ptr;
	uint8_t * ptr2;
	uint8_t *test_ptr;
	test = bochs_vga_read(VBE_DISPI_INDEX_ID);


	if(test == 0xFFFF)
		return NULL;
	ptr = (uint32_t *)bochs_vga_init_internal(640, 480, 32);
	if(ptr == NULL)
		return NULL;
	
	if(background_image != NULL)
	{
		test_ptr = (uint8_t *)background_image;
		uint32_t offset = *(uint32_t *)(test_ptr + 0xa);
		test_ptr = test_ptr + offset;
		kmemcpy(framebuffer, test_ptr, 640*480*4);
		ptr = (void *)test_ptr;
		for(int i = 0; i < 640*480; i++)
		{
			ptr2 = (uint8_t *)ptr;
			ptr2[0] /= 3;
			ptr2[1] /= 3;
			ptr2[2] /= 3;
			ptr2[3] /= 3;
			ptr++;
		}



	}
	for(int i = 0; i < NUM_CONSOLES; i++)
	{
		new->aux = kcalloc(sizeof(struct bochs_vga_state), 1);
		state = new->aux;
		state->w = 640/8;
		state->h = 480/8;
		new->h = 480;
		new->w = 640;



		new[i].putc = &bochs_vga_putc;
		new[i].switch_fn = &bochs_vga_switch;	
		kmemcpy(framebuffer + 640*480*i, test_ptr, 640*480*4);

		console_register(&new[i]);	


	}


	return new;
}


