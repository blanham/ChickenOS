#!/bin/bash
mkdir -p ./mnt
sudo mount -o loop,offset=32256 disk.img ./mnt
sudo cp src/kernel.bin ./mnt/kernel
sudo cp grub.cfg ./mnt/boot/grub
sync
sudo umount ./mnt
