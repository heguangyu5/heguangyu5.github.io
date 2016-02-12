<?php

if ($argc != 5)
{
	echo "Usage: php build.php setup.bin vmlinux.bin minor_root major_root\n";
	exit;
}

$setupBin = file_get_contents($argv[1]);
$vmlinuxBin = file_get_contents($argv[2]);

$minor_root = $argv[3];
$major_root = $argv[4];

$setup_bin_size = strlen($setupBin);
$setup_sectors  = (int)(($setup_bin_size + 511) / 512);
$setup_sects    = $setup_sectors - 1;

$vmlinux_bin_size = strlen($vmlinuxBin);
$sys_size   = (int)(($vmlinux_bin_size + 15 + 4) / 16);
$sys_size_1 = $sys_size & 0xFF;
$sys_size_2 = ($sys_size >> 8) & 0xFF;
$sys_size_3 = ($sys_size >> 16) & 0xFF;
$sys_size_4 = ($sys_size >> 24) & 0xFF;

$setupBinPadLen = $setup_sectors * 512 - $setup_bin_size;
$vmlinuxBinPadLen = $sys_size * 16 - 4 - $vmlinux_bin_size;

echo "setup.bin padded zeros: $setupBinPadLen\n";
echo "vmlinux.bin padded zeros: $vmlinuxBinPadLen\n";

/**
 * why chr() here?
 *
 * try:
 *  $s = 'ab';
 *  $s[0] = 33;
 *  $s[1] = chr(33);
 *  file_put_contents('/tmp/php-int', $s);
 *
 * then
 *  hexdump -C /tmp/php-int
 *  0x00 33 21
 */
$setupBin[508] = chr($minor_root);
$setupBin[509] = chr($major_root);
$setupBin[0x1f1] = chr($setup_sects);
$setupBin[0x1f4] = chr($sys_size_1);
$setupBin[0x1f5] = chr($sys_size_2);
$setupBin[0x1f6] = chr($sys_size_3);
$setupBin[0x1f7] = chr($sys_size_4);

$fp = fopen('/tmp/bzImage.pad', 'w');
fwrite($fp, $setupBin);
fwrite($fp, pack('@' . $setupBinPadLen));
fwrite($fp, $vmlinuxBin);
fwrite($fp, pack('@' . $vmlinuxBinPadLen));
fclose($fp);
