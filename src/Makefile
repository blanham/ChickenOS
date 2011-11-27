#CROSS=/usr/local/cross/bin
#CC=$(CROSS)/i586-elf-gcc
IDIR=./include

LIB_SRCS=./lib/stdio.o ./lib/string.o ./lib/sys.o ./lib/kernel/memory.o 
SOURCES2=  interrupt.o intr-core.o 
SOURCES=kmain.o console.o hw.o kbd.o loader.o syscall.o switch.o thread.o timer.o vm.o $(LIB_SRCS) $(SOURCES2)
#LINKER=i586-elf-ld
LINKER=ld
CFLAGS=-g -Wall -Wextra -Werror -I./include --std=gnu99\
    -nostdlib -nostdinc -fno-builtin -nostartfiles -nodefaultlibs
LDFLAGS=-Tlinker.ld
ASFLAGS=-felf

all: $(SOURCES) link

test: all
	cd .. && ./update_image.sh && qemu -m 512 -fda ./floppy.img && cd src

test2: all
	cd .. && ./update_image.sh && qemu -s -S -m 512 -fda ./floppy.img && cd src

clean:
	rm *.o ./lib/*.o kernel.bin

link:
	$(LINKER) $(LDFLAGS) -o kernel.bin $(SOURCES)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.s.o:
	nasm $(ASFLAGS) $<
