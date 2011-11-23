CROSS=/usr/local/cross/bin
CC=$(CROSS)/i586-elf-gcc
IDIR=./include

LIB_SRCS=./lib/stdio.o ./lib/string.o ./lib/sys.o ./lib/kernel/memory.o 
SOURCES2=  interrupt.o intr-core.o 
SOURCES=kmain.o console.o hw.o kbd.o loader.o syscall.o switch.o thread.o timer.o vm.o $(LIB_SRCS) $(SOURCES2)
LINKER=i586-elf-ld
CFLAGS=-g -Wall -Wextra -Werror -I./include --std=gnu99\
    -nostdlib -nostdinc -fno-builtin -nostartfiles -nodefaultlibs
LDFLAGS=-Tlinker.ld
ASFLAGS=-felf

all: $(SOURCES) link

clean:
	rm *.o ./lib/*.o kernel.bin

link:
	/usr/local/cross/bin/i586-elf-ld $(LDFLAGS) -o kernel.bin $(SOURCES)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.s.o:
	nasm $(ASFLAGS) $<
