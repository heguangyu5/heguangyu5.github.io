<?php

$children = array();

for ($i = 0; $i < 300; $i++) {
    $pid = pcntl_fork();
    if ($pid == -1) {
        die("fork failed");
    }
    if ($pid) {
        // parent
        $children[$pid] = array(
            'start' => time()
        );
    } else {
        // child
        echo "child started\n";
        $fp = stream_socket_client("tcp://a.example.com:8888", $errno, $errstr, 30);
        if (!$fp) {
            echo $errstr, "\n";
            exit(1);
        }
        fwrite($fp, "GET / HTTP/1.1\r\nHost: a.example.com:8888\r\nConnection: keep-alive\r\n\r\n");
        while (!feof($fp)) {
            fgets($fp, 1024);
        }
        fclose($fp);
        echo "child exit\n";
        exit;
    }
}

$exitCount = 0;
while (true) {
    $pid = pcntl_wait($status);
    if (pcntl_wifexited($status)) {
        $children[$pid]['end'] = time();
        $children[$pid]['time'] = $children[$pid]['end'] - $children[$pid]['start'];
        $exitCount++;
        if ($exitCount == 300) {
            break;
        }
    }
}

uasort($children, function ($a, $b) {
    if ($a['start'] == $b['start']) {
        if ($a['time'] == $b['time']) {
            return 0;
        }
        return $a['time'] > $b['time'] ? 1 : -1;
    }
    return $a['start'] > $b['start'] ? 1 : -1;
});

$i = 1;
foreach ($children as $pid => $data) {
    echo $i, '-', $pid, ': start = ', $data['start'], ', end = ', $data['end'], ', time = ', $data['time'], "\n";
    $i++;
}
