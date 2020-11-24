#ifndef C_OS_ARCH_I386_PIC_H
#define C_OS_ARCH_I386_PIC_H

/* PIC defines */
#define PIC1_CMD  0x0020
#define PIC1_DATA 0x0021
#define PIC2_CMD  0x00A0
#define PIC2_DATA 0x00A1
#define PIC_EOI   0x20

void pic_init();
void pic_mask(int irq);
void pic_unmask(int irq);
void pic_send_end(int irq);
#endif
