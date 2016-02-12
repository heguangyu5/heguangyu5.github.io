<?php

if ($argc != 2) {
	echo "Usage: php range2freearea.php ranges.php\n";
	exit;
}

$rangesFile = $argv[1];
if (!file_exists($rangesFile)) {
	echo 'No such file: ', $rangesFile, "\n";
	exit;
}

$ranges = include $rangesFile;
$totalPages  = 0;
foreach ($ranges as $range) {
	$split = array();
	list($start, $end) = $range;
	$totalPages += $end - $start;

	$mode = $start % 64;
	$part1End = $mode ? $start + (64 - $mode) : $start;
	if ($part1End >= $end) {
		$split[0] = array($start, $end - 1);
		$split[1] = null;
		$split[2] = null;
		$rangesSplit[] = $split;
		continue;
	}

	$part2End = $end - ($end % 64);
	if ($part2End > $part1End) {
		if ($part1End == $start) {
			$split[0] = null;
		} else {
			$split[0] = array($start, $part1End - 1);
		}
		$split[1] = array($part1End, $part2End - 1);
		if ($part2End == $end) {
			$split[2] = null;
		} else {
			$split[2] = array($part2End, $end - 1);
		}
		$rangesSplit[] = $split;
		continue;
	}

	// $part2End == $part1End
	$split[0] = array($start, $part1End - 1);
	$split[1] = null;
	$split[2] = array($part2End, $end - 1);
	$rangesSplit[] = $split;
}

printf("Total pages: %x\n", $totalPages);

foreach ($rangesSplit as $idx => $split) {
	printf("%x-%x(%x) split", $ranges[$idx][0], $ranges[$idx][1], $ranges[$idx][1] - $ranges[$idx][0]);
	echo ' part1: ';
	if ($split[0]) {
		printf("%x-%x(%d, %x-%x)", $split[0][0], $split[0][1], $split[0][1] - $split[0][0] + 1, $split[0][0] * 0x38, $split[0][1] * 0x38);
	} else {
		echo 'none';
	}

	echo ' part2: ';
	if ($split[1]) {
		printf("%x-%x(%s, %x-%x)", $split[1][0], $split[1][1], "64 * " . ($split[1][1] - $split[1][0] + 1) / 64, $split[1][0] * 0x38, $split[1][1] * 0x38);
	} else {
		echo 'none';
	}

	echo ' part3: ';
	if ($split[2]) {
		printf("%x-%x(%d, %x-%x)", $split[2][0], $split[2][1], $split[2][1] - $split[2][0] + 1, $split[2][0] * 0x38, $split[2][1] * 0x38);
	} else {
		echo 'none';
	}
	echo "\n";
}

$free_area = array();
for ($i = 0; $i < 11; $i++) {
	$free_area[] = array();
}

function mergeFreeArea($order)
{
	global $free_area;
	while ($order < 10) {
		$count = count($free_area[$order]);
		if ($count < 2) {
			break;
		}
		$count--;
		if ($free_area[$order][$count] - pow(2, $order) != $free_area[$order][$count-1]) {
			break;
		}
		$nextOrder = $order + 1;
		if ($free_area[$order][$count-1] % pow(2, $nextOrder) != 0) {
			break;
		}
		$nextCount = count($free_area[$nextOrder]);
		$free_area[$nextOrder][$nextCount] = $free_area[$order][$count-1];
		unset($free_area[$order][$count], $free_area[$order][$count-1]);
		$order++;
	}
}

function addToFreeArea($pfn)
{
	global $free_area;
	$count = count($free_area[0]);
	if ($count == 0) {
		$free_area[0][0] = $pfn;
		return;
	}
	$free_area[0][$count] = $pfn;
	mergeFreeArea(0);
}

function addToFreeArea6($pfn)
{
	global $free_area;
	$count = count($free_area[6]);
	if ($count == 0) {
		$free_area[6][0] = $pfn;
		return;
	}
	$free_area[6][$count] = $pfn;
	mergeFreeArea(6);
}

foreach ($rangesSplit as $split) {
	// part1
	if ($split[0]) {
		$start = $split[0][0];
		$end = $split[0][1];
		while ($start <= $end) {
			addToFreeArea($start);
			$start++;
		}
	}
	// part2
	if ($split[1]) {
		$start = $split[1][0];
		$end = $split[1][1];
		while ($start < $end) {
			addToFreeArea6($start);
			$start += 64;
		}
	}

	// part3
	if ($split[2]) {
		$start = $split[2][0];
		$end = $split[2][1];
		while ($start <= $end) {
			addToFreeArea($start);
			$start++;
		}
	}
}

foreach ($free_area as $order => $area) {
	$pages = pow(2, $order);
	printf("order = %d (%d pages, count = 0x%x):\n", $order, $pages, count($area));
	foreach ($area as $pfn) {
		printf("\t%x-%x\n", $pfn, $pfn + $pages - 1);
	}
}
