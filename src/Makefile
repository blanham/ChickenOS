CROSS	=		/usr/local/cross/bin
CC		=		$(CROSS)/i586-elf-gcc
CC		=		gcc -m32
CC		=		clang -m32
#LINKER	=		$(CROSS)/i586-elf-ld
CFLAGS	=		-g -Wall -Werror $(INCLUDES) \
				-std=gnu99	\
				-fno-stack-protector \
			-ffreestanding\
				-nostdlib -nostdinc -fno-builtin
CFLAGS += 		-Wextra
LINKER	=		ld -melf_i386 
LDFLAGS	=		-Tinit/linker.ld 

AS		=		nasm
ASFLAGS	=		-felf

INCLUDES=		-I./include -I./fs -I./include/kernel -I./rme2

KLIBSRCS=		./lib/stdio.o ./lib/string.o ./lib/stdlib.o ./lib/ctype.o\
				./lib/kernel/memory.o ./lib/vsprintf.o

EXT2	= 		fs/ext2/ext2.o fs/ext2/ops.o
VFS		= 		fs/vfs.o fs/ops.o fs/initrd.o fs/device.o
FSSRCS	=		$(VFS) $(EXT2)

MEMSRCS = 		mm/gdt.o mm/liballoc.o mm/liballoc_funcs.o\
				mm/paging.o mm/palloc.o mm/vm.o

NETSRCS =		net/net_core.o net/arp.o net/dhcp.o net/ethernet.o net/ip.o\
				net/udp.o net/icmp.o net/tcp.o net/dns.o net/socket.o\
				device/net/pcnet.o device/net/ne2000.o device/net/rtl8139.o\
				device/net/e1000.o

USBSOURCES 	=	device/usb/usb.o device/usb/uhci.o

AUDIOSRCS	=	device/audio.o #device/audio/ac97.o device/audio/intel_ac97.o

VIDEOSOURCES=	device/video.o device/video/vga.o device/video/bochs_vga.o

DEVSRCS		=	device/intr-core.o  device/input.o\
				device/console.o device/hw.o device/kbd.o\
				device/timer.o device/serial.o device/ata.o\
				device/pci.o $(VIDEOSOURCES) $(AUDIOSRCS)\
				$(USBSOURCES)
RME_OBJ =		rme.o prefixes.o ops_alu.o ops_stack.o ops_io.o ops_mov.o\
				ops_string.o ops_jump.o ops_call.o ops_flags.o ops_misc.o
RME_OBJ := 		$(addprefix rme2/,$(RME_OBJ))

THREAD	=		thread/thread.o thread/thread_ops.o thread/syscall.o\
				thread/scheduler.o	thread/tss.o thread/fork.o thread/exec.o\
				thread/signal.o thread/load_elf.o sh.o
				#have to have interrupt.o here at the moment 
CORE	=		init/kmain.o init/loader.o init/multiboot.o device/interrupt.o
				
SOURCES	=		$(CORE)  $(THREAD) $(DEVSRCS) $(NETSRCS) $(MEMSRCS) \
				$(FSSRCS) $(KLIBSRCS) 
#SOURCES = $(CORE) device/intr-core.o device/kbd.o device/serial.o thread/thread.o mm/palloc.o mm/liballoc.o mm/liballoc_funcs.o thread/syscall.o thread/tss.o device/console.o device/hw.o mm/gdt.o mm/paging.o mm/vm.o $(KLIBSRCS)
#SOURCES = init/kmain.o init/loader.o 
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

test2: all
	cd .. && ./new_update.sh &&  qemu -rtc base=localtime \
	-m 64 -hda disk.img -serial file:log -net nic,vlan=0,model=e1000,macaddr=00:00:00:00:00:00 -net tap,vlan=0,ifname=tap0,script=no  && cd src 
debug: all
	cd .. && ./new_update.sh &&  /usr/local/bin/qemu-system-i386 -rtc base=localtime \
	-m 64 -usb -hda disk.img -s -S -serial file:log && cd src 

clean:
	rm kernel.bin\
		$(SOURCES)

link:
	$(LINKER) $(LDFLAGS) -o kernel.bin $(SOURCES) ./libgcc.a



%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.s.o:
	$(AS) $(ASFLAGS) $<
