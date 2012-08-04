CROSS	=		/usr/local/cross/bin
CC		=		$(CROSS)/i586-elf-gcc
CC		=		gcc -m32 #clang -m32
#LINKER	=		$(CROSS)/i586-elf-ld
CFLAGS	=		-g -Wall -Wextra -Werror $(INCLUDES) \
				-std=gnu99	\
				-fno-stack-protector -nostartfiles \
				-nodefaultlibs -ffreestanding\
				-nostdlib -nostdinc -fno-builtin

LINKER	=		ld -melf_i386
LDFLAGS	=		-Tinit/linker.ld

AS		=		nasm
ASFLAGS	=		-felf

INCLUDES=		-I./include -I./fs -I./include/kernel

KLIBSRCS=		./lib/stdio.o ./lib/string.o ./lib/ctype.o\
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

DEVSRCS	=		device/intr-core.o\
				device/console.o device/hw.o device/kbd.o\
				device/timer.o device/serial.o device/ata.o\
				device/pci.o
THREAD	=		thread/thread.o thread/syscall.o thread/scheduler.o\
				thread/tss.o thread/fork.o thread/exec.o thread/load_elf.o sh.o
				#have to have interrupt.o here at the moment 
CORE	=		init/kmain.o init/loader.o device/interrupt.o
				
SOURCES	=		$(CORE) $(THREAD) $(DEVSRCS) $(NETSRCS) $(MEMSRCS) \
				$(FSSRCS) $(KLIBSRCS)

all: $(SOURCES) link
	cp kernel.bin /tftpboot/chicken
lib: $(LIBSRCS)
test: all
	cd .. && ./new_update.sh &&  qemu -rtc base=localtime \
	-m 64 -hda disk.img -serial file:log&& cd src 

test2: all
	cd .. && ./new_update.sh &&  qemu -rtc base=localtime \
	-m 64 -hda disk.img -serial file:log -net nic,vlan=0,model=e1000,macaddr=00:00:00:00:00:00 -net tap,vlan=0,ifname=tap0,script=no  && cd src 
debug: all
	cd .. && ./update_image.sh && qemu -rtc base=localtime -s -S -m 512 -fda \
	./floppy.img && cd src

clean:
	rm kernel.bin\
		$(SOURCES)

link:
	$(LINKER) $(LDFLAGS) -o kernel.bin $(SOURCES)


%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.s.o:
	$(AS) $(ASFLAGS) $<
