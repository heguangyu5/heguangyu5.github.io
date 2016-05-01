#!/bin/bash

mkdir tmp_initrd_dir
cd tmp_initrd_dir

mkdir -p bin dev usr/sbin usr/bin sbin proc sys
cp ../busybox.dyn ./bin/busybox
mkdir lib64 lib
cp /lib64/ld-linux-x86-64.so.2 lib64/
cp /lib/x86_64-linux-gnu/{libm.so.6,libc.so.6} lib/
sudo chroot ./ /bin/busybox --install -s
cp ../tools/my-simple-shell ./init

find . | cpio --quiet -H newc -o | gzip -9 -n > ../busybox.img.gz
cd ../
rm -rf tmp_initrd_dir

sudo losetup -o $((512*2048)) /dev/loop1 my-linux.img
sudo mount /dev/loop1 /mnt
sudo cp busybox.img.gz /mnt/initrd.img.gz
sync
sudo umount /mnt/
sudo losetup -d /dev/loop1
