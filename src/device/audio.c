// TODO : Actually implement audio support
/*
#include <common.h>
#include <mm/liballoc.h>
#include <device/pci.h>
#include <device/audio.h>
#include <device/audio/ac97.h>
#include <chicken/device/ioport.h>
#include <sys/types.h>
struct audio_device {

//ioctl_fn
//write_fn
//read_fn
	void *aux;
};

struct intel_ac97_state {
	struct pci_device *pci;
	uint16_t mixer_base;
	uint16_t dma_base;



};

//gonna take a leaf out of OpenBSD's book
struct _ac97_vendors{
	uint32_t id;
	char * name;

}	ac97_vendors[] = {{0x83847600, "SigmaTel"}};


uint16_t intel_ac97_mixer_inw(struct intel_ac97_state *e, uint16_t port)
{
	return inw(e->mixer_base + port);
}

void ac97_reset()
{



}

extern uint32_t pci_get_barn(struct pci_device *dev, uint8_t type, uint8_t num);
int audio_read(uint16_t dev UNUSED, void *buf UNUSED, off_t off UNUSED, size_t count UNUSED)
{

	return 0;
}

int audio_write(uint16_t dev UNUSED, void *buf UNUSED, off_t off UNUSED, size_t count UNUSED)
{

	return 0;
}

int audio_ioctl(uint16_t dev UNUSED, int request UNUSED, va_list args UNUSED)
{
	return 0;
}

void intel_ac97_handler(void *aux)
{
	struct intel_ac97_state *e = aux;
	(void)e;
	printf("Audio IRQ\n");


}

int audio_init()
{
	struct intel_ac97_state *e = (struct intel_ac97_state *)kcalloc(sizeof(*e), 1);
	uint16_t temp;
	uint32_t vendor_id;
	e->pci = pci_get_device(INTEL_VEND,0x2415);

	if(e->pci != NULL)
	{
		//e->pci_hdr = e->pci->header;
		//printf("Intel Pro/1000 Ethernet adapter Rev %i found at ", e->pci_hdr->rev);
		e->mixer_base = pci_get_barn(e->pci, PCI_BAR_IO, 0) & ~1;
		e->dma_base = pci_get_barn(e->pci, PCI_BAR_IO, 1) & ~1;
	//	printf("Mixer base address %x DMA base address %x\n", e->mixer_base, e->dma_base);
		temp = intel_ac97_mixer_inw(e,AC97_REG_VENDOR_ID1);
		vendor_id = temp << 16;
		temp = intel_ac97_mixer_inw(e,AC97_REG_VENDOR_ID2);
		vendor_id |= temp;
		if(vendor_id == ac97_vendors[0].id)
		{
			printf("Found %s AC97 codec\n", ac97_vendors[0].name);

		}
	//	printf("Thingy %x\n",vendor_id);



		pci_register_irq(e->pci, &intel_ac97_handler, e);
	}

	return 0;
}
*/