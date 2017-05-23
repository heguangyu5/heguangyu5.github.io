#!/bin/bash

# @see Documentation/devices.txt

mkdir tmp_initrd_dir
cd tmp_initrd_dir

mkdir bin sbin usr usr/bin usr/sbin proc sys dev sda1
cd dev
sudo mknod tty c 5 0
sudo mknod tty0 c 4 0
sudo mknod tty1 c 4 1
sudo mknod sda b 8 0
sudo mknod sda1 b 8 1
cd ..
cp ../busybox ./bin/
sudo chroot ./ /bin/busybox --install -s

echo "#!/bin/sh" > ./init
echo "mount -t proc none /proc" >> ./init
echo "mount -t sysfs none /sys" >> ./init
echo "mount -t ext3 /dev/sda1 /sda1" >> ./init
echo "mount -t proc none /sda1/proc" >> ./init
echo "mount -t sysfs none /sda1/sys" >> ./init
echo "exec switch_root /sda1 /sbin/init" >> ./init
chmod +x ./init

find . | cpio --quiet -H newc -o | gzip -9 -n > ../busybox.img.gz
cd ../
rm -rf tmp_initrd_dir

sudo losetup -o $((512*2048)) /dev/loop1 my-linux.img
sudo mount /dev/loop1 /mnt
sudo cp busybox.img.gz /mnt/initrd.img.gz
sync
sudo umount /mnt/
sudo losetup -d /dev/loop1

rm my-linux.vdi
VBoxManage convertdd my-linux.img my-linux.vdi --format VDI
