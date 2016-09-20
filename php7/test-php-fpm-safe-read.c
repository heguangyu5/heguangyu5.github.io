#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

typedef struct _fcgi_header {
    unsigned char version;
    unsigned char type;
    unsigned char requestIdB1;
    unsigned char requestIdB0;
    unsigned char contentLengthB1;
    unsigned char contentLengthB0;
    unsigned char paddingLength;
    unsigned char reserved;
} fcgi_header;

int main(void)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(9000);
    if (connect(sock, (struct sockaddr *)&addr, sizeof addr) == -1) {
        perror("connect");
        return 1;
    }

    fcgi_header h;
    memset(&h, 0, sizeof h);
    h.version = 0xAA;
    h.type    = 0xBB;
    h.requestIdB1 = 0xCC;
    h.requestIdB0 = 0xDD;

    printf("sizeof fcgi_header = %zd\n", sizeof h);

    int half_size = sizeof h / 2;
    if (write(sock, &h, half_size) != half_size) {
        perror("write");
        return 1;
    }

    printf("press any key to exit\n");
    pause();

    return 0;
}
