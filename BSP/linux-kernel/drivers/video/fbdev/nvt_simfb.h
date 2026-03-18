#ifndef __NVT_SIM_FB_H__
#define __NVT_SIM_FB_H__

#include <drm/drm_fourcc.h>
#include <linux/fb.h>
#include <linux/types.h>

#define NVT_SIMFB_VERSION "1.00.000"
#define SIMPLE_FORMATS \
{ \
	{ "r5g6b5", 16, {11, 5}, {5, 6}, {0, 5}, {0, 0}, DRM_FORMAT_RGB565 }, \
	{ "r5g5b5a1", 16, {11, 5}, {6, 5}, {1, 5}, {0, 1}, DRM_FORMAT_RGBA5551 }, \
	{ "x1r5g5b5", 16, {10, 5}, {5, 5}, {0, 5}, {0, 0}, DRM_FORMAT_XRGB1555 }, \
	{ "a1r5g5b5", 16, {10, 5}, {5, 5}, {0, 5}, {15, 1}, DRM_FORMAT_ARGB1555 }, \
	{ "r5g5b5a1", 16, {11, 5}, {6, 5}, {1, 5}, {0, 1}, DRM_FORMAT_RGBA5551 }, \
	{ "r4g4b4a4", 16, {12, 4}, {8, 4}, {4, 4}, {0, 4}, DRM_FORMAT_RGBA4444 }, \
	{ "r8g8b8", 24, {16, 8}, {8, 8}, {0, 8}, {0, 0}, DRM_FORMAT_RGB888 }, \
	{ "x8r8g8b8", 32, {16, 8}, {8, 8}, {0, 8}, {0, 0}, DRM_FORMAT_XRGB8888 }, \
	{ "a8r8g8b8", 32, {16, 8}, {8, 8}, {0, 8}, {24, 8}, DRM_FORMAT_ARGB8888 }, \
	{ "r8g8b8a8", 32, {24, 8}, {16, 8}, {8, 8}, {0, 8}, DRM_FORMAT_RGBA8888 }, \
	{ "a8b8g8r8", 32, {0, 8}, {8, 8}, {16, 8}, {24, 8}, DRM_FORMAT_ABGR8888 }, \
	{ "x2r10g10b10", 32, {20, 10}, {10, 10}, {0, 10}, {0, 0}, DRM_FORMAT_XRGB2101010 }, \
	{ "a2r10g10b10", 32, {20, 10}, {10, 10}, {0, 10}, {30, 2}, DRM_FORMAT_ARGB2101010 }, \
	{ "r10g10b10a2", 32, {22, 10}, {12, 10}, {2, 10}, {0, 2}, DRM_FORMAT_RGBA1010102 }, \
}

struct nvtsimfb_format {
	const char *name;
	u32 bits_per_pixel;
	struct fb_bitfield red;
	struct fb_bitfield green;
	struct fb_bitfield blue;
	struct fb_bitfield transp;
	u32 fourcc;
};

struct nvtsimfb_platform_data {
	u32 width;
	u32 height;
	u32 stride;
	const char *format;
};

#endif /* __NVT_SIM_FB_H__ */
