#!/bin/bash
e2cp build/kernel.bin ext2.img:kernel                                                 
e2cp build/kernel.bin boot.img:chicken                                                 
e2cp build/kernel.bin boot2.img:chicken                                                 
cat partition.img ext2.img > disk.img                                             
#dd if=boot.img of=alpine_ours.img conv=notrunc bs=16384 seek=64 count=30976
dd if=boot2.img of=alpine2.img conv=notrunc bs=16384 seek=64 count=6400
#cat start.img boot.img end.img > disk.img                                             
##mkdir -p ./mnt
##sudo mount -o loop,offset=32256 disk.img ./mnt
##sudo cp src/kernel.bin ./mnt/kernel
#sudo cp grub.cfg ./mnt/boot/grub
#sudo cp init ./mnt/init
#sudo cp dash ./mnt/dash
#sudo cp frotz ./mnt/frotz
#sudo cp zo ./mnt/zo
#sudo cp chicken.bmp ./mnt/chicken.bmp
#sudo cp usrsrc/tests/bin/* ./mnt/tests/
#sudo cp -R disk/* ./mnt/
#ls ./mnt
##sync
##sudo umount ./mnt
