#!/bin/bash

#sudo losetup /dev/loop0 floppy.img
#sudo mount /dev/loop0 /mnt/floppy
sudo mount -t ext2 -o loop floppy.img /mnt/floppy
sudo cp src/kernel.bin /mnt/floppy/kernel
sudo umount /mnt/floppy
