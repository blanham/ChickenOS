#include <chicken/common.h>
#include <chicken/device/video.h>
#include <chicken/device/video/vga.h>
#include <chicken/device/video/bochs_vga.h>

void video_init(struct kernel_boot_info *info)
{
/*	console_t *console = NULL;
	(void)info;
//	console = bochs_vga_init();
	if(console == NULL)
		console = vga_init();*/
	vga_init(info);
}