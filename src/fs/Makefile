CROSS=/usr/local/cross/bin
#CC=$(CROSS)/i586-elf-gcc
IDIR=../include

LIB_SRCS=../lib/stdio.o ../lib/string.o ../lib/sys.o ../lib/kernel/memory.o 
SOURCES=vfs.o fat.o ext2fs.o
#LINKER=i586-elf-ld
CFLAGS=-Wall -Wextra -Werror --std=gnu99\
#CFLAGS=-Wall -Wextra -Werror -I../include --std=gnu99\
#    -nostdlib -nostdinc -fno-builtin -nostartfiles -nodefaultlibs
#LDFLAGS=linker.ld
ASFLAGS=-felf

all: $(SOURCES) link

clean:
	rm *.o ./lib/*.o kernel.bin

link:
	gcc $(LDFLAGS) -o vfs $(SOURCES)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.s.o:
	nasm $(ASFLAGS) $<
