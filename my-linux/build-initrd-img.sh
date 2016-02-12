#!/bin/bash

mkdir tmp_initrd_dir
cd tmp_initrd_dir

# 第一步
mkdir -p bin dev usr/sbin usr/bin sbin
sudo cp -a /dev/{console,tty,tty1} dev/
cp ../busybox ./bin/
sudo chroot ./ /bin/busybox --install -s
echo "#!/bin/sh" > ./init
echo "setsid cttyhack sh" >> ./init
chmod +x ./init

# mount-root
if [ "$1" == "mount-root" ]
then
    sudo cp -a /dev/{tty0,sda,sda1} dev/
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
sudo umount /mnt/
sudo losetup -d /dev/loop1
