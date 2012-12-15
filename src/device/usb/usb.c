#include <device/usb.h>
#include <device/usb/uhci.h>
#include <device/pci.h>
#include <kernel/hw.h>
#include <stdio.h>
#include <kernel/memory.h>
#include <mm/liballoc.h>



void usb_init()
{
	struct uhci_controller *uhci;
	uhci = uhci_init();	
	(void)uhci;

}
