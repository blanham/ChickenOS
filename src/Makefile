ARCH = i386
ARMPRE	=		arm-linux-gnueabi-
CROSS	=		/usr/local/cross/bin
CC		=		$(CROSS)/i586-elf-gcc
CC		=		gcc -m32
CC		=		clang -m32
#LINKER	=		$(CROSS)/i586-elf-ld
CFLAGS	=		-g -Wall -Werror $(INCLUDES) \
				-std=gnu99	\
				-fno-stack-protector \
				-ffreestanding\
				-nostdlib -nostdinc -fno-builtin -DARCH_I386
CFLAGS += 		-Wextra
LINKER	=		ld -melf_i386 
LDFLAGS	=		-T arch/$(ARCH)/linker.ld 
ARMAS	=		$(ARMPRE)as	
AS		=		nasm
ASFLAGS	=		-felf
#FIXME: this should only be one path
INCLUDES=		-I./include -I./fs -I./include/kernel -I./rme2

ARCHSRCS=		$(patsubst %.c,%.o,$(wildcard arch/$(ARCH)/*.c))\
				$(patsubst %.s,%.o,$(wildcard arch/$(ARCH)/*.s))


KLIBSRCS=		$(patsubst %.c,%.o,$(wildcard lib/*.c))

VFS		= 		$(patsubst %.c,%.o,$(wildcard fs/*.c))
#needs cleanup before chaging to patsubst
EXT2	= 		fs/ext2/ext2.o fs/ext2/ops.o

FSSRCS	=		$(VFS) $(EXT2)

MEMSRCS = 		$(patsubst %.c,%.o,$(wildcard mm/*.c))

NETCORESRCS=	$(patsubst %.c,%.o,$(wildcard net/*.c))
NETDEVSRCS=		$(patsubst %.c,%.o,$(wildcard device/net/*.c))

NETSRCS=		$(NETCORESRCS) $(NETDEVSRCS)

USBSRCS 	=	device/usb/usb.o device/usb/uhci.o

AUDIOSRCS	=	#device/audio/ac97.o device/audio/intel_ac97.o

VIDEOSRCS	=	device/video/vga.o device/video/bochs_vga.o

DEVCORE		=	$(patsubst %.c,%.o,$(wildcard device/*.c))

DEVSRCS		=	$(DEVCORE) $(VIDEOSRCS) $(AUDIOSRCS) $(USBSRCS)

THREAD	=		$(patsubst %.c,%.o,$(wildcard thread/*.c))

CORE	=		init/kmain.o init/multiboot.o 
				
SOURCES	=		$(CORE)  $(ARCHSRCS) $(THREAD) $(DEVSRCS) $(NETSRCS) $(MEMSRCS) \
				$(FSSRCS) $(KLIBSRCS) 

all: $(SOURCES) link
tftp: all
	cp kernel.bin /tftpboot/chicken
lib: $(LIBSRCS)
test: all
	cd .. && ./new_update.sh &&  qemu-system-i386  -vga std -soundhw ac97  -rtc base=localtime \
	-m 64 -usb  -hda disk.img -serial file:log && cd src 
bochs: all
	cd .. && ./new_update.sh &&  ./run_bochs.sh && cd src 
test3: all
	cd .. &&  qemu -rtc base=localtime \
	-m 64 -hda disk.img -serial file:log && cd src 

network: all
	cd .. && ./new_update.sh &&  qemu -rtc base=localtime \
	-m 64 -hda disk.img -serial file:log -net nic,vlan=0,model=e1000,macaddr=00:00:00:00:00:00 -net tap,vlan=0,ifname=tap0,script=no  && cd src 
debug: all
	cd .. && ./new_update.sh &&  /usr/local/bin/qemu-system-i386 -rtc base=localtime \
	-m 64 -usb -hda disk.img -s -S -serial file:log && cd src 

clean:
	rm -f kernel.bin $(SOURCES)

link:
	$(LINKER) $(LDFLAGS) -o kernel.bin $(SOURCES) ./libgcc.a



%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.s.o:
ifeq ($(ARCH), i386)
	$(AS) $(ASFLAGS) $<
else
	$(ARMAS) $<
endif
