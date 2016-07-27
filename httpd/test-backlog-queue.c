#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9999);

    bind(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    listen(sock, 2);

    int conn;
    while ((conn = accept(sock, NULL, NULL)) >= 0) {
        char buf[1024];
        char *ptr = buf;
        char c;
        ssize_t count;
        while (1) {
            count = read(conn, &c, 1);
            if (count <= 0) {
                goto close_conn;
            }
            *ptr = c;
            ptr++;
            if (c == '\n') {
                *ptr = 0;
                write(conn, buf, ptr - buf);
                printf("received: %s", buf);
                ptr = buf;
            }
        }
close_conn:
        close(conn);
    }

    close(sock);
}
