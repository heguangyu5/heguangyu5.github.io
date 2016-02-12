#!/bin/bash

bximage -mode=create -hd=500M -q my-linux.img
sudo losetup /dev/loop0 my-linux.img
sudo gparted /dev/loop0
# create an MS-DOS partition table
# create primary partition ext4
sudo fdisk -l /dev/loop0
sudo losetup -o $((512*2048)) /dev/loop1 my-linux.img
sudo mount /dev/loop1 /mnt
sudo mkdir -p /mnt/boot/grub/
sudo cp device.map /mnt/boot/grub/
sudo grub-install --root-directory=/mnt --grub-mkdevicemap=/mnt/boot/grub/device.map --no-floppy /dev/loop0
sudo cp grub.cfg /mnt/boot/grub/
sudo cp bzImage /mnt/
sudo cp initrd.img.gz /mnt/
sync
sudo umount /mnt/
sudo losetup -d /dev/loop0 /dev/loop1
