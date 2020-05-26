#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <drm/drm.h>
#include <drm/drm_mode.h>
#include <unistd.h>

static void print_drm_mode_card_res(struct drm_mode_card_res *res)
{
    printf("--drm_mode_card_res--\n");

    int i;
    uint64_t *p = NULL;

    printf("fb_id_ptr = %#llx\n", res->fb_id_ptr);
    printf("count_fbs = %u\n", res->count_fbs);
    if (res->fb_id_ptr) {
        p = (uint64_t *)res->fb_id_ptr;
        for (i = 0; i < res->count_fbs; i++) {
            printf("  %d: %#lx\n", i, p[i]);
        }
    }

    printf("crtc_id_ptr = %#llx\n", res->crtc_id_ptr);
    printf("count_crtcs = %u\n", res->count_crtcs);
    if (res->crtc_id_ptr) {
        p = (uint64_t *)res->crtc_id_ptr;
        for (i = 0; i < res->count_crtcs; i++) {
            printf("  %d: %#lx\n", i, p[i]);
        }
    }

    printf("connector_id_ptr = %#llx\n", res->connector_id_ptr);
    printf("count_connectors = %u\n", res->count_connectors);
    if (res->connector_id_ptr) {
        p = (uint64_t *)res->connector_id_ptr;
        for (i = 0; i < res->count_connectors; i++) {
            printf("  %d: %#lx\n", i, p[i]);
        }
    }

    printf("encoder_id_ptr = %#llx\n", res->encoder_id_ptr);
    printf("count_encoders = %u\n", res->count_encoders);
    if (res->encoder_id_ptr) {
        p = (uint64_t *)res->encoder_id_ptr;
        for (i = 0;  i < res->count_encoders; i++) {
            printf("  %i: %#lx\n", i, p[i]);
        }
    }

    printf("min_width = %u\n", res->min_width);
    printf("max_width = %u\n", res->max_width);
    printf("min_height = %u\n", res->min_height);
    printf("max_height = %u\n", res->max_height);
    printf("\n");
}

static void print_modeinfo(int idx, struct drm_mode_modeinfo *modeinfo)
{
    printf("  --modeinfo %d--\n", idx);

    printf("  clock = %u\n", modeinfo->clock);
    printf(
        "  hdisplay = %hu, hsync_start = %hu, hsync_end = %hu, htotal = %hu, hskew = %hu\n",
        modeinfo->hdisplay,
        modeinfo->hsync_start,
        modeinfo->hsync_end,
        modeinfo->htotal,
        modeinfo->hskew
    );
    printf(
        "  vdisplay = %hu, vsync_start = %hu, vsync_end = %hu, vtotal = %hu, vscan = %hu\n",
        modeinfo->vdisplay,
        modeinfo->vsync_start,
        modeinfo->vsync_end,
        modeinfo->vtotal,
        modeinfo->vscan
    );
    printf("  vrefresh = %u\n", modeinfo->vrefresh);
    printf("  flags = %x\n", modeinfo->flags);
    printf("  type = %u\n", modeinfo->type);
    printf("  name = ");
    for (int i = 0; i < DRM_DISPLAY_MODE_LEN; i++) {
        printf("%c", modeinfo->name[i]);
    }
    printf("\n");
    printf("\n");
}

static void print_conn(int idx, struct drm_mode_get_connector *conn)
{
    printf("--conn %d--\n", idx);

    int i;

    printf("encoders_ptr = %#llx\n", conn->encoders_ptr);
    printf("count_encoders = %u\n", conn->count_encoders);

    printf("modes_ptr = %#llx\n", conn->modes_ptr);
    printf("count_modes = %u\n", conn->count_modes);
    if (conn->modes_ptr) {
        for (i = 0; i < conn->count_modes; i++) {
            print_modeinfo(i, ((struct drm_mode_modeinfo *)conn->modes_ptr) + i);
        }
    }

    printf("props_ptr = %#llx\n", conn->props_ptr);
    printf("count_props = %u\n", conn->count_props);

    printf("prop_values_ptr = %#llx\n", conn->prop_values_ptr);

    printf("encoder_id = %u\n", conn->encoder_id);
    printf("connector_id = %u\n", conn->connector_id);
    printf("connector_type = %u\n", conn->connector_type);
    printf("connector_type_id = %u\n", conn->connector_type_id);
    printf("connection = %u\n", conn->connection);
    printf("mm_width = %u\n", conn->mm_width);
    printf("mm_height = %u\n", conn->mm_height);
    printf("subpixel = %u\n", conn->subpixel);
    printf("\n");
}

int main(void)
{
    int dri_fd = open("/dev/dri/card0", O_RDWR);
    if (dri_fd == -1) {
        perror("open /dev/dri/card0");
        return 1;
    }

    ioctl(dri_fd, DRM_IOCTL_SET_MASTER, 0);

    uint64_t res_fb_buf[10]   = {0},
             res_crtc_buf[10] = {0},
             res_conn_buf[10] = {0},
             res_enc_buf[10]  = {0};

    struct drm_mode_card_res res = {0};
    ioctl(dri_fd, DRM_IOCTL_MODE_GETRESOURCES, &res);
    res.fb_id_ptr        = (uint64_t)res_fb_buf;
    res.crtc_id_ptr      = (uint64_t)res_crtc_buf;
    res.connector_id_ptr = (uint64_t)res_conn_buf;
    res.encoder_id_ptr   = (uint64_t)res_enc_buf;
    ioctl(dri_fd, DRM_IOCTL_MODE_GETRESOURCES, &res);
    print_drm_mode_card_res(&res);

    void *fb_base[10] = {0};
    long fb_w[10]     = {0};
    long fb_h[10]     = {0};

    int i;
    for (i = 0; i < res.count_connectors; i++) {
        if (res_conn_buf[i] == 0) {
            continue;
        }

        struct drm_mode_modeinfo conn_mode_buf[20] = {0};
        uint64_t conn_prop_buf[20]    = {0},
                 conn_propval_buf[20] = {0},
                 conn_enc_buf[20]     = {0};
        struct drm_mode_get_connector conn = {0};

        conn.connector_id = res_conn_buf[i];

        ioctl(dri_fd, DRM_IOCTL_MODE_GETCONNECTOR, &conn);
        conn.modes_ptr = (uint64_t)conn_mode_buf;
        conn.props_ptr = (uint64_t)conn_prop_buf;
        conn.prop_values_ptr = (uint64_t)conn_propval_buf;
        conn.encoders_ptr = (uint64_t)conn_enc_buf;
        ioctl(dri_fd, DRM_IOCTL_MODE_GETCONNECTOR, &conn);
        print_conn(i, &conn);

        if (   conn.count_encoders < 1
            || conn.count_modes < 1
            || !conn.encoder_id
            || !conn.connection
        ) {
            printf("Not connected\n");
            continue;
        }

        struct drm_mode_create_dumb create_dumb = {0};
        struct drm_mode_map_dumb    map_dumb    = {0};
        struct drm_mode_fb_cmd      cmd_dumb    = {0};

        create_dumb.width  = conn_mode_buf[0].hdisplay;
        create_dumb.height = conn_mode_buf[0].vdisplay;
        create_dumb.bpp    = 32;
        create_dumb.flags  = 0;
        create_dumb.pitch  = 0;
        create_dumb.size   = 0;
        create_dumb.handle = 0;
        ioctl(dri_fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_dumb);

        cmd_dumb.width  = create_dumb.width;
        cmd_dumb.height = create_dumb.height;
        cmd_dumb.bpp    = create_dumb.bpp;
        cmd_dumb.pitch  = create_dumb.pitch;
        cmd_dumb.depth  = 24;
        cmd_dumb.handle = create_dumb.handle;
        ioctl(dri_fd, DRM_IOCTL_MODE_ADDFB, &cmd_dumb);

        map_dumb.handle = create_dumb.handle;
        ioctl(dri_fd, DRM_IOCTL_MODE_MAP_DUMB, &map_dumb);

        printf(
            "fb size = %#llx = %llu MB, width = %u, height = %u\n",
            create_dumb.size,
            create_dumb.size / 1024 / 1024,
            create_dumb.width,
            create_dumb.height
        );

        fb_base[i] = mmap(NULL, create_dumb.size, PROT_READ | PROT_WRITE, MAP_SHARED, dri_fd, map_dumb.offset);
        if (fb_base[i] == MAP_FAILED) {
            perror("mmap");
            return 1;
        }
        fb_w[i] = create_dumb.width;
        fb_h[i] = create_dumb.height;

        struct drm_mode_get_encoder enc = {0};
        enc.encoder_id = conn.encoder_id;
        ioctl(dri_fd, DRM_IOCTL_MODE_GETENCODER, &enc);

        struct drm_mode_crtc crtc = {0};
        crtc.crtc_id = enc.crtc_id;
        ioctl(dri_fd, DRM_IOCTL_MODE_GETCRTC, &crtc);

        crtc.fb_id = cmd_dumb.fb_id;
        crtc.set_connectors_ptr = (uint64_t)&res_conn_buf[i];
        crtc.count_connectors = 1;
        crtc.mode = conn_mode_buf[0];
        crtc.mode_valid = 1;
        ioctl(dri_fd, DRM_IOCTL_MODE_SETCRTC, &crtc);
    }

    ioctl(dri_fd, DRM_IOCTL_DROP_MASTER, 0);

    for (i = 0; i < 30; i++) {
        int j;
        for (j = 0; j < res.count_connectors; j++) {
            int x, y;
            for (y = 0; y < fb_h[j]; y++) {
                for (x = 0; x < fb_w[j]; x++) {
                    int location = y * fb_w[j] + x;
                    *(((uint32_t *)fb_base[j]) + location) = 0x00ff00ff;
                }
            }
        }
        usleep(100000);
    }

    return 0;
}
