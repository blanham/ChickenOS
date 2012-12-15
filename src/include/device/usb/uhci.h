#ifndef C_OS_DEVICE_USB_UHCI_H
#define C_OS_DEVICE_USB_UHCI_H


/* We reuse this data structure in 
 * transfer descriptor, since it just
 * adds the depth bit field
 */
struct frame_list_ptr {
	uint32_t terminate :1;
	uint32_t qh_select :1;
	uint32_t depth :1;
	uint32_t reserved :1;
	uint32_t ptr :28;
} __attribute__((packed));

struct td_ctrl_status {
	uint32_t actlen : 11;
	uint32_t reserve0 :5;
	/* status */
	uint32_t reserve1 : 1;
	uint32_t bitstuff_err:1;
	uint32_t crc_timeout: 1;
	uint32_t nak_recv :1;
	uint32_t babble_detected :1;
	uint32_t data_buf_err :1;
	uint32_t stalled :1;
	uint32_t active :1;
	/* config */
	uint32_t ioc :1;
	uint32_t ios :1;
	uint32_t ls :1;
	uint32_t error_cnt :2;
	uint32_t spd :1;
	uint32_t reserve2 :2;
} __attribute__((packed));

struct td_token {
	uint32_t pid :8;
	uint32_t dev_addr :7;
	uint32_t endpt :4;
	uint32_t d:1;
	uint32_t reserved :1;
	uint32_t maxlen : 11;
} __attribute__((packed));

struct transfer_descriptor {
	struct frame_list_ptr flp;
	struct td_ctrl_status ctrl_status;
	struct td_token token;
	uint32_t buf_ptr;
	/* these 4 dwords can be used by software */
	uint32_t reserved[4];
} __attribute__((packed));


struct queue_head {
	struct frame_list_ptr qh_link_ptr;
	struct frame_list_ptr qe_link_ptr;
} __attribute__((packed));

struct uhci_controller {
	struct pci_device *pci;
	uint32_t io_base;
	uint8_t *mem_base;
	int port_cnt;
	struct frame_list_ptr *frames;
	struct transfer_descriptor *tx_descs;
	struct queue_head *q_heads;
};


struct uhci_controller * uhci_init();

#endif
