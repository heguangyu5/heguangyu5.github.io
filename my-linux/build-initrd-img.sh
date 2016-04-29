#!/bin/bash

# @see Documentation/devices.txt

mkdir tmp_initrd_dir
cd tmp_initrd_dir

# 第一步
mkdir -p bin dev usr/sbin usr/bin sbin
cd dev
sudo mknod tty c 5 0
sudo mknod tty1 c 4 1
cd ..
cp ../busybox ./bin/
sudo chroot ./ /bin/busybox --install -s
echo "#!/bin/sh" > ./init
echo "setsid cttyhack sh" >> ./init
chmod +x ./init

# mount-root
if [ "$1" == "mount-root" ]
then
    cd dev
    sudo mknod tty0 c 4 0
    sudo mknod sda b 8 0
    sudo mknod sda1 b 8 1
    cd ..
    mkdir -p mnt/root proc sys
    echo "#!/bin/sh" > ./init
    echo "mount -t proc none /proc" >> ./init
    echo "mount -t sysfs none /sys" >> ./init
    echo "mount -o ro /dev/sda1 /mnt/root" >> ./init
    echo "setsid cttyhack sh" >> ./init
    echo "exec switch_root /mnt/root /sbin/init" >> ./init
fi


find . | cpio --quiet -H newc -o | gzip -9 -n > ../busybox.img.gz
cd ../
rm -rf tmp_initrd_dir

sudo losetup -o $((512*2048)) /dev/loop1 my-linux.img
sudo mount /dev/loop1 /mnt
sudo cp busybox.img.gz /mnt/initrd.img.gz
sync
sudo umount /mnt/
sudo losetup -d /dev/loop1
