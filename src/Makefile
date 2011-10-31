CROSS=/usr/local/cross/bin
CC=$(CROSS)/i586-elf-gcc
IDIR=./include

LIB_SRCS=./lib/stdio.o ./lib/string.o ./lib/sys.o ./lib/kernel/memory.o 
SOURCES=kmain.o console.o hw.o loader.o $(LIB_SRCS) 
LINKER=i586-elf-ld
CFLAGS=-Wall -Wextra -Werror -I./include --std=gnu99\
    -nostdlib -nostdinc -fno-builtin -nostartfiles -nodefaultlibs
LDFLAGS=linker.ld
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
