#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include "mandelbrot.h"

void mandelbrot(uint8_t *buffer, uint32_t width, uint32_t height, uint32_t bits_per_pixel)
{
    struct timeval start, stop;
    gettimeofday(&start, NULL);

    int iters = 1;
    int i, j, c;
    float a, b, tmp, x, y;
    int bytes_per_pixel = bits_per_pixel / 8;
    while (iters <= 48) {
        for (i = 0; i < height; i++) {
            for (j = 0; j < width; j++) {
                x = 0;
                y = 0;
                c = 0;

                a = 3. * j / width - 2;
                b = 1 - 2. * i / height;

                do {
                    tmp = x;
                    x = x * x - y * y + a;
                    y = 2 * tmp * y + b;
                    if (x * x + y * y > 4) {
                        break;
                    }
                } while (++c < iters);

                int start = i * width * bytes_per_pixel + j * bytes_per_pixel;
                int color = c * 255 / iters;
                buffer[start]     = color;
                buffer[start + 1] = color;
                buffer[start + 2] = color;
            }
        }
        iters++;
    }

    gettimeofday(&stop, NULL);
    printf(
        "mandelbrot 48 iters: %.2f seconds\n",
        (stop.tv_sec - start.tv_sec) + (stop.tv_usec - start.tv_usec) / 1000000.
    );
}
