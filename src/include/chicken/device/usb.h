#ifndef C_OS_DEV_USB_H
#define C_OS_DEV_USB_H

#include <chicken/interrupt.h>

/* http://www.beyondlogic.org/usbnutshell/usb3.shtml:
 * There are 4 bits to the PID, however to insure it is received correctly, 
 * the 4 bits are complemented and repeated, making an 8 bit PID in total. 
 *	The resulting format is shown below.
 *
 * PID0 	PID1 	PID2 	PID3 	nPID0 	nPID1 	nPID2 	nPID3
*/
#define usb_pid_8bit(x) (((~(x)) << 4) | (x))
#define USB_PID_TOKEN_OUT 	0x1
#define USB_PID_TOKEN_IN  	0x9
#define USB_PID_TOKEN_SOF 	0x5
#define USB_PID_TOKEN_SETUP	0xD
#define USB_PID_DATA_DATA0	0x3
#define USB_PID_DATA_DATA1	0xB
#define USB_PID_DATA_DATA2	0x7
#define USB_PID_DATA_MDATA	0xF
#define USB_PID_HAND_ACK	0x2
#define USB_PID_HAND_NAK	0xA
#define USB_PID_HAND_STALL	0xE
#define USB_PID_HAND_NYET	0x6
#define USB_PID_SPCL_PRE	0xC
#define USB_PID_SPCL_ERR	0xC
#define USB_PID_SPCL_SPLIT	0x8
#define USB_PID_SPCL_PING	0x4

struct usb_packet_token_full {
	uint8_t sync;
	uint8_t pid;
	uint32_t addr :7;
	uint32_t endp :4;
	uint32_t crc :5;
	uint32_t eop: 3;
} __attribute__((packed));
struct usb_packet_token_high {
	uint32_t sync;
	uint8_t pid;
	uint32_t addr :7;
	uint32_t endp :4;
	uint32_t crc :5;
	uint32_t eop: 3;
} __attribute__((packed));
struct usb_packet_data_full {
	uint8_t sync;
	uint8_t pid;
	uint32_t addr :7;
	uint32_t endp :4;
	uint32_t crc :5;
	uint32_t eop: 3;
} __attribute__((packed));

void usb_init();
#endif
