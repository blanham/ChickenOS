./compile.sh
./link.sh
cat stage1 stage2 pad kernel.bin pad2 > floppy.img
dd if=floppy.img of=floppy2.img bs=1 count=1474560    
