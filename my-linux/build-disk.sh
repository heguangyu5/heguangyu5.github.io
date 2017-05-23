#!/bin/bash

bximage -mode=create -hd=500M -q my-linux.img
sudo losetup /dev/loop0 my-linux.img

sudo gparted /dev/loop0
# create an MS-DOS partition table
# create primary partition ext3
sudo fdisk -l /dev/loop0

sudo losetup -o $((512*2048)) /dev/loop1 my-linux.img
sudo mount /dev/loop1 /mnt

sudo mkdir -p /mnt/boot/grub/
sudo cp device.map /mnt/boot/grub/
sudo grub-install --root-directory=/mnt --grub-mkdevicemap=/mnt/boot/grub/device.map --no-floppy /dev/loop0
sudo cp grub.cfg /mnt/boot/grub/

sudo cp bzImage /mnt/
sudo cp initrd.img.gz /mnt/

sudo mkdir /mnt/bin /mnt/sbin /mnt/usr /mnt/usr/bin /mnt/usr/sbin /mnt/lib64 /mnt/lib /mnt/dev/ /mnt/proc /mnt/sys
sudo cp /lib64/ld-linux-x86-64.so.2 /mnt/lib64/
sudo cp /lib/x86_64-linux-gnu/{libm.so.6,libc.so.6} /mnt/lib/
sudo cp busybox.dyn /mnt/bin/busybox

sudo mknod /mnt/dev/tty c 5 0
sudo mknod /mnt/dev/tty0 c 4 0
sudo mknod /mnt/dev/tty1 c 4 1
sudo mknod /mnt/dev/tty2 c 4 2
sudo mknod /mnt/dev/tty3 c 4 3
sudo mknod /mnt/dev/tty4 c 4 4
sudo mknod /mnt/dev/sda b 8 0
sudo mknod /mnt/dev/sda1 b 8 1

sudo chroot /mnt /bin/busybox --install -s

sync

sudo umount /mnt/
sudo losetup -d /dev/loop0 /dev/loop1
