#ifndef C_OS_ARCH_I386_GDT_H
#define C_OS_ARCH_I386_GDT_H
struct segment_descriptor {
	uint16_t limit0;
	uint16_t base0;
	uint8_t base16;
	uint8_t access;
	uint8_t flags;
	uint8_t base24;	

} __attribute__((packed));
typedef struct segment_descriptor gdt_seg_t;
struct gdt_descriptor
{
	uint16_t size;
	uint32_t location;
} __attribute__((packed));

#define GDTA_P		 0x80	//present
#define GDTA_RING0 	 0x00	
#define GDTA_RING3 	 0x60
#define GDTA_ALWAYS	 0x10	//always 1
#define GDTA_EXE	 0x08
#define GDTA_CANJUMP 0x04	//can jump to lower ring levels
#define GDTA_RW		 0x02
#define GDTA_ACCESS  0x01
#define GDTF_32BIT 	 0x40
#define GDTF_4KB 	 0x80 

#define GDTA_KERNEL 	 (GDTA_P | GDTA_RING0 | GDTA_ALWAYS | GDTA_EXE  | GDTA_RW)
#define GDTA_KERNEL_DATA (GDTA_P | GDTA_RING0 | GDTA_ALWAYS | GDTA_RW)
#define GDTA_USER 		 (GDTA_P | GDTA_RING3 | GDTA_ALWAYS | GDTA_EXE |  GDTA_RW)
#define GDTA_USER_DATA 	 (GDTA_P | GDTA_RING3 | GDTA_ALWAYS | GDTA_RW)
#define GDTA_TSS 	 	 (GDTA_P | GDTA_RING3 | GDTA_EXE | GDTA_ACCESS)
#define GDTF_BOTH 		 (GDTF_32BIT | GDTF_4KB)

extern void gdt_flush(struct gdt_descriptor *ptr);
void gdt_install();
#endif
