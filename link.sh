nasm -f elf -o loader.o src/loader.s
/usr/local/cross/bin/i586-elf-ld -T src/linker.ld -o kernel.bin src/loader.o src/kernel.o
