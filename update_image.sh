#!/bin/bash

#sudo losetup /dev/loop0 floppy.img
#sudo mount /dev/loop0 /mnt/floppy

#sudo mount -t ext2 -o loop floppy.img /mnt/floppy
#sudo cp src/kernel.bin /mnt/floppy/kernel
#sudo cp initrd /mnt/floppy/initrd
#sudo cp menu.lst /mnt/floppy/grub/menu.lst
#sudo umount /mnt/floppy

mcopy -Do -i floppy.img src/kernel.bin ::kernel
mcopy -Do -i floppy.img initrd ::initrd
mcopy -Do -i floppy.img menu.lst ::boot/grub/menu.lst

