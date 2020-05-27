# framebuffer (/dev/fb0)

open /dev/fb0, 通过 ioctl() 取得 screeninfo.

然后 mmap() 得到 framebuffer.

然后通过CPU画像素显示到显示器上.

@see fb-drm.pdf
@see fb.c

# drm-dumb-buffer (/dev/dri/card0)

open /dev/card0, 通过 KMS ioctl() 最终得到一个 dumb-buffer.

然后和 framebuffer 一样,通过CPU画像素显示到显示器上.

@see fb-drm.pdf
@see drm.c
@see modeset.c
@see modeset-double-buffered.c

# GPU?

`man drm`
`man drm-memory`

## GEM

Gem-buffers cannot be created with a generic API. Each driver provides its own API to create gem-buffers. See for example `DRM_I915_GEM_CREATE`, `DRM_NOUVEAU_GEM_NEW` or `DRM_RADEON_GEM_CREATE`.

Each of these ioctls returns a gem-handle that can be passed to different **generic** ioctls. 

The **libgbm** library from the **mesa3D** distribution tries to provide a driver-independent API to create gbm buffers and retrieve a gbm-handle to them.

Besides generic buffer management, the GEM API **does not** provide any generic access. Each driver implements its own functionality on top of this API.

The next higher-level API is **OpenGL**. So if you want to use more GPU features, you should use the mesa3D library to create OpenGL contexts on DRM devices. You may have a look at other mesa3D manpages, including **libgbm** and **libEGL**.

2D **software-rendering** (rendering with the CPU) can be achieved with the **dumb-buffer-API** in a driver-**independent** fashion, however, for **hardware-accelerated** 2D or 3D rendering you must use OpenGL. Any other API that tries to abstract the driver-internals to access GEM-execution-buffers and other GPU internals, would simply reinvent OpenGL so it is not provided.

## Summary

`GEM -> drm-intel / drm-radeon / drm-nouveau -> OpenGL (mesa3D)`
