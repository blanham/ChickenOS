CROSS=/usr/local/cross/bin
#CC=$(CROSS)/i586-elf-gcc
IDIR=./include

LIB_SRCS=./lib/stdio.o ./lib/string.o ./lib/sys.o ./lib/kernel/memory.o 
SOURCES2= 		interrupt.o intr-core.o
EXT2= 			fs/ext2/ext2.o fs/ext2/ops.o
VFS= 			fs/vfs.o fs/ops.o fs/initrd.o fs/device.o
FSSOURCES=		$(VFS) $(EXT2)
MEMSRCS = 		mm/liballoc.o mm/liballoc_funcs.o
SOURCES=		kmain.o console.o hw.o kbd.o loader.o    \
				serial.o sh.o thread/syscall.o thread.o timer.o \
				vm.o $(MEMSRCS) $(FSSOURCES) $(LIB_SRCS) \
				$(SOURCES2)
#LINKER=$(CROSS)/i586-elf-ld
LINKER=ld
CFLAGS=-g -Wall -Wextra -Werror -I./include -I./fs --std=gnu99\
    -nostdlib -nostdinc -fno-builtin -nostartfiles -nodefaultlibs
LDFLAGS=-Tlinker.ld
ASFLAGS=-felf

all: $(SOURCES) link

test: all
	cd .. && ./update_image.sh && qemu -m 512 -fda ./floppy.img && cd src

test2: all
	cd .. && ./update_image.sh && qemu -s -S -m 512 -fda ./floppy.img && cd src

clean:
	rm *.o fs/*.o ./lib/*.o kernel.bin

link:
	$(LINKER) $(LDFLAGS) -o kernel.bin $(SOURCES)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.s.o:
	nasm $(ASFLAGS) $<
