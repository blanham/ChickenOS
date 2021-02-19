#ifndef C_OS_ARCH_AARCH64_SERIAL_H
#define C_OS_ARCH_AARCH64_SERIAL_H

#define COM1 0x3F8
#define COM2 0x2F8
#define COM3 0x3E8
#define COM4 0x2E8

#define RX_BUF 0
#define TX_BUF 0

#define DLLB 0
#define IER  1

#define DLHB 1
#define IIR 2
#define FCR 2
#define LCR 3
#define MCR 4
#define LSR 5
#define MSR 6
#define SCRATCH 7

#define DLAB 0x80

void serial_init();
#endif
