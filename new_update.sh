#!/bin/bash
mkdir -p ./mnt
sudo mount -o loop,offset=32256 disk.img ./mnt
sudo cp src/kernel.bin ./mnt/kernel
sudo cp grub.cfg ./mnt/boot/grub
sudo cp init ./mnt/init
sudo cp frotz ./mnt/frotz
sudo cp zo ./mnt/zo
sudo cp chicken.bmp ./mnt/chicken.bmp
sync
sudo umount ./mnt
