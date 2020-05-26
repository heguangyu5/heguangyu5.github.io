#include <linux/fb.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <unistd.h>

static void print_fb_bitfield(struct fb_bitfield *field)
{
    printf("  offset = %u\n", field->offset);
    printf("  length = %u\n", field->length);
    printf("  msb_right = %u\n", field->msb_right);
}

static void print_vinfo(struct fb_var_screeninfo *vinfo)
{
    printf("--fb_var_screeninfo--\n\n");

    printf("xres = %u\n", vinfo->xres);
    printf("yres = %u\n", vinfo->yres);
    printf("xres_virtual = %u\n", vinfo->xres_virtual);
    printf("yres_virtual = %u\n", vinfo->yres_virtual);
    printf("xoffset = %u\n", vinfo->xoffset);
    printf("yoffset = %u\n", vinfo->yoffset);
    printf("\n");

    printf("bits_per_pixel = %u\n", vinfo->bits_per_pixel);
    printf("grayscale = %u\n", vinfo->grayscale);
    printf("red:\n");
    print_fb_bitfield(&vinfo->red);
    printf("green:\n");
    print_fb_bitfield(&vinfo->green);
    printf("blue:\n");
    print_fb_bitfield(&vinfo->blue);
    printf("transp:\n");
    print_fb_bitfield(&vinfo->transp);
    printf("nonstd = %u\n", vinfo->nonstd);
    printf("\n");

    printf("activate = %u\n", vinfo->activate);
    printf("\n");

    printf("height = %u\n", vinfo->height);
    printf("width = %u\n", vinfo->width);
    printf("\n");

    printf("pixclock = %u\n", vinfo->pixclock);
    printf("left_margin = %u\n", vinfo->left_margin);
    printf("right_margin = %u\n", vinfo->right_margin);
    printf("upper_margin = %u\n", vinfo->upper_margin);
    printf("lower_margin = %u\n", vinfo->lower_margin);
    printf("\n");

    printf("hsync_len = %u\n", vinfo->hsync_len);
    printf("vsync_len = %u\n", vinfo->vsync_len);
    printf("sync = %u\n", vinfo->sync);
    printf("vmode = %u\n", vinfo->vmode);
    printf("rotate = %u\n", vinfo->rotate);
    printf("colorspace = %u\n", vinfo->colorspace);
    printf("\n");
}

static void print_finfo(struct fb_fix_screeninfo *finfo)
{
    printf("--fb_fix_screeninfo--\n\n");

    printf("id: ");
    for (int i = 0; i < 16; i++) {
        printf("%c", finfo->id[i]);
    }
    printf("\n");
    printf("smem_start = %#lx\n", finfo->smem_start);
    printf("smem_len = %u = %u MB\n", finfo->smem_len, finfo->smem_len / 1024 / 1024);
    printf("type = %u\n", finfo->type);
    printf("type_aux = %u\n", finfo->type_aux);
    printf("visual = %u\n", finfo->visual);
    printf("xpanstep = %hu\n", finfo->xpanstep);
    printf("ypanstep = %hu\n", finfo->ypanstep);
    printf("ywrapstep = %hu\n", finfo->ywrapstep);
    printf("line_length = %u bytes\n", finfo->line_length);
    printf("mmio_start = %#lx\n", finfo->mmio_start);
    printf("mmio_len = %u = %u MB\n", finfo->mmio_len, finfo->mmio_len / 1024 / 1024);
    printf("accel = %u\n", finfo->accel);
    printf("capabilities = %hu\n", finfo->capabilities);
    printf("\n");
}

static inline uint32_t pixel_color(uint8_t r, uint8_t g, uint8_t b, struct fb_var_screeninfo *vinfo)
{
    return (r << vinfo->red.offset) | (g << vinfo->green.offset) | (b << vinfo->blue.offset);
}

int main(void)
{
    struct fb_fix_screeninfo finfo = {0};
    struct fb_var_screeninfo vinfo = {0};

    int fb_fd = open("/dev/fb0", O_RDWR);
    if (fb_fd == -1) {
        perror("open /dev/fb0");
        return 1;
    }

    ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
    print_vinfo(&vinfo);

    if (vinfo.grayscale != 0 || vinfo.bits_per_pixel != 32) {
        printf("set grayscale = 0, bits_per_pixel = 32\n");
        vinfo.grayscale = 0;
        vinfo.bits_per_pixel = 32;
        ioctl(fb_fd, FBIOPUT_VSCREENINFO, &vinfo);
        ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
        print_vinfo(&vinfo);
    }

    ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo);
    print_finfo(&finfo);

    long screensize = vinfo.yres_virtual * finfo.line_length;
    printf("screensize = %ld = %ld MB\n", screensize, screensize / 1024 / 1024);

    uint8_t *fbp = mmap(NULL, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
    if (fbp == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    int i;
    for (i = 0; i < 30; i++) {
        int x, y;
        for (x = 0; x < vinfo.xres; x++) {
            for (y = 0; y < vinfo.yres; y++) {
                long location = (x + vinfo.xoffset) * (vinfo.bits_per_pixel/8) + (y + vinfo.yoffset) * finfo.line_length;
                *((uint32_t *)(fbp + location)) = pixel_color(0xFF, 0x00, 0xFF, &vinfo);
            }
        }
        usleep(100000);
    }

    return 0;
}
