<?php
$fp = stream_socket_client("tcp://a.example.com:8888", $errno, $errstr, 30);
if (!$fp) {
    echo $errstr, "\n";
    exit(1);
}

while (!feof($fp)) {
    fwrite($fp, "GET / HTTP/1.1\r\nHost: a.example.com:8888\r\nConnection: keep-alive\r\n\r\n");

    $findStart = false;
    while (true) {
        $str = fgets($fp, 1024);
        if ($str == "<!DOCTYPE html>\n") {
            $findStart = true;
        }
        if (!$findStart) {
            echo $str;
        }
        if ($str == "</html>\n") {
            break;
        }
    }

    sleep(3);
}
