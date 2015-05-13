#!/bin/bash
mkdir -p ./mnt
sudo mount -o loop,offset=32256 disk.img ./mnt
sudo cp src/kernel.bin ./mnt/kernel
sudo cp grub.cfg ./mnt/boot/grub
#sudo cp init ./mnt/init
sudo cp dash ./mnt/dash
sudo cp frotz ./mnt/frotz
sudo cp zo ./mnt/zo
sudo cp chicken.bmp ./mnt/chicken.bmp
#sudo cp usrsrc/tests/bin/* ./mnt/tests/
sudo cp -R disk/* ./mnt/
ls ./mnt
sync
sudo umount ./mnt
