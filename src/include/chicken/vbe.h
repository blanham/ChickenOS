#ifndef C_OS_VBE_H
#define C_OS_VBE_H

struct vbe_control_info {
	uint8_t signature[4];
	int16_t version;
	int16_t oemstr[2];
	uint8_t capabilities[4];
	int16_t videomodes[2];
	int16_t totalmem;
	int16_t soft_ver;
	int16_t vendstr[2];
	uint32_t prodstr;
	uint32_t prodrevstr;

};
struct vbe_mode_info {

	uint16_t attributes;
	uint8_t winA,winB;
	uint16_t granularity;
	uint16_t winsize;
	uint16_t segmentA, segmentB;
	uintptr_t realFctPtr;
	uint16_t pitch; // chars per scanline
		   
	uint16_t x_res, y_res;
	uint8_t Wchar, Ychar, planes, bpp, banks;
	uint8_t memory_model, bank_size, image_pages;
	uint8_t reserved0;
	
	uint8_t red_mask, red_position;
	uint8_t green_mask, green_position;
	uint8_t blue_mask, blue_position;
	uint8_t rsv_mask, rsv_position;
	uint8_t directcolor_attributes;
	
	uint32_t framebuffer_address;  // your LFB (Linear Framebuffer) address ;)
	uint32_t reserved1;
	uint16_t reserved2;
};

#endif
