#ChickenOS Makefile
#Abandon all hope ye who enter here
ARCH = x86/i386
ARMPRE	=		arm-none-gnueabi-
CROSS	=		~/opt/cross/bin
CC		=		$(CROSS)/i686-elf-gcc
#CC		=		gcc -m32
CC		=		clang -m32 -target i686-pc-none-elf -mno-sse -mno-mmx
#LINKER	=		$(CROSS)/i586-elf-ld
CFLAGS	=		-g -Wall -Werror $(INCLUDES) \
				-std=gnu11	\
				-ffreestanding \
				-fno-stack-protector \
				-Wextra\
				-fno-builtin -DARCH_I386
CFLAGS += 		-Wextra
#LINKER	=		ld -melf_i386
LINKER	=		clang -m32 -target i686-pc-none-elf -nostdlib
LDFLAGS	=		-T arch/$(ARCH)/linker.ld
ARMAS	=		$(ARMPRE)as
AS		=		nasm
ASFLAGS	=		-felf
#FIXME: this should only be one path
ARCH_INCLUDES=	-I./arch/$(ARCH)/c
INCLUDES=		-I./include -I./fs -I./include/kernel $(ARCH_INCLUDES)

ARCHSRCS=		$(patsubst %.s,%.o,$(wildcard arch/$(ARCH)/*.s))\
				$(patsubst %.c,%.o,$(wildcard arch/$(ARCH)/*.c))\
				$(patsubst %.S,%.o,$(wildcard arch/$(ARCH)/*.S))

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

VIDEOSRCS	=	$(patsubst %.c,%.o,$(wildcard device/video/*.c))

DEVCORE		=	$(patsubst %.c,%.o,$(wildcard device/*.c))

DEVSRCS		=	$(DEVCORE) $(VIDEOSRCS) $(AUDIOSRCS) $(USBSRCS)

THREAD	=		$(patsubst %.c,%.o,$(wildcard thread/*.c))

CORE	=		init/kmain.o init/init.o

SOURCES	=		$(CORE) $(ARCHSRCS) $(THREAD) $(DEVSRCS) $(NETSRCS) $(MEMSRCS) \
				$(FSSRCS) $(KLIBSRCS)

all: $(SOURCES) link
tftp: all
	cp kernel.bin /tftpboot/chicken
lib: $(LIBSRCS)
test: all
	cd .. && ./new_update.sh &&  DISPLAY=:0.0 qemu-system-i386  -vga std -soundhw ac97  -rtc base=localtime \
	-m 128 -usb  -hda disk.img -serial file:log && cd src
bochs: all
	cd .. && ./new_update.sh &&  ./run_bochs.sh && cd src
test3: all
	cd .. &&  qemu -rtc base=localtime \
	-m 64 -hda disk.img -serial file:log && cd src

network: all
	cd .. && ./new_update.sh &&  qemu -rtc base=localtime \
	-m 64 -hda disk.img -serial file:log -net nic,vlan=0,model=e1000,macaddr=00:00:00:00:00:00 -net tap,vlan=0,ifname=tap0,script=no  && cd src
debug: all
	cd .. && ./new_update.sh &&  qemu-system-i386 -rtc base=localtime \
	-m 64 -usb -hda disk.img -s -S -serial file:log && cd src

clean:
	rm -f kernel.bin $(SOURCES)

link:
	$(LINKER) $(LDFLAGS) -o kernel.bin $(SOURCES) clangrt.a # ./libgcc.a #arch/i386/rme2/src/rme.a ./libgcc.a

library: $(SOURCES)
	$(LINKER) -shared -Wl,-soname,thisistest -o test.bin $(SOURCES) clangrt.a # ./libgcc.a #arch/i386/rme2/src/rme.a ./libgcc.a


%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.s.o:
ifeq ($(ARCH), x86/i386)
	$(AS) $(ASFLAGS) $<
else
	$(ARMAS) $<
endif
.S.o:
ifeq ($(ARCH), x86/i386)
	$(AS) $(ASFLAGS) $<
else
	$(ARMAS) $<
endif
