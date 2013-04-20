#include <common.h>
#include <device/video.h>
#include <device/video/vga.h>
#include <device/video/bochs_vga.h>


int video_init()
{
	console_t *console = NULL;

//	console = bochs_vga_init();
	if(console == NULL)
		console = vga_init();
	(void)console;

	return 0;

}
