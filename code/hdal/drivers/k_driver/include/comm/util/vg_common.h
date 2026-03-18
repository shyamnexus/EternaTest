/*
 *   @file   vg_common.h
 *
 *   @brief  vg common header file.
 *
 *   Some common definitions used by vg.
 *
 *   Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#include <linux/version.h>

#define ALIGN128_DOWN(x)    (((x) >> 7) << 7)
#define ALIGN64_DOWN(x)     (((x) >> 6) << 6)
#define ALIGN32_DOWN(x)     (((x) >> 5) << 5)
#define ALIGN16_DOWN(x)     (((x) >> 4) << 4)
#define ALIGN8_DOWN(x)      (((x) >> 3) << 3)
#define ALIGN4_DOWN(x)      (((x) >> 2) << 2)
#define ALIGN2_DOWN(x)      (((x) >> 1) << 1)

#define ALIGN4K_UP(x)       ((((x) + 4095) >> 12) << 12)
#define ALIGN128_UP(x)      ((((x) + 127) >> 7) << 7)
#define ALIGN64_UP(x)       ((((x) + 63) >> 6) << 6)
#define ALIGN32_UP(x)       ((((x) + 31) >> 5) << 5)
#define ALIGN16_UP(x)       ((((x) + 15) >> 4) << 4)
#define ALIGN8_UP(x)        ((((x) + 7) >> 3) << 3)
#define ALIGN4_UP(x)        ((((x) + 3) >> 2) << 2)
#define ALIGN2_UP(x)        ((((x) + 1) >> 1) << 1)

#define IS_ALIGN128(x)      ((x) == (((x) >> 7) << 7))
#define IS_ALIGN64(x)       ((x) == (((x) >> 6) << 6))
#define IS_ALIGN32(x)       ((x) == (((x) >> 5) << 5))
#define IS_ALIGN16(x)       ((x) == (((x) >> 4) << 4))
#define IS_ALIGN8(x)        ((x) == (((x) >> 3) << 3))
#define IS_ALIGN4(x)        ((x) == (((x) >> 2) << 2))
#define IS_ALIGN2(x)        ((x) == (((x) >> 1) << 1))

#define ALIGNX_DOWN(level, x) (              \
		(level == 2) ? ALIGN2_DOWN(x) : (    \
		(level == 4) ? ALIGN4_DOWN(x) : (    \
		(level == 8) ? ALIGN8_DOWN(x) : (    \
		(level == 16) ? ALIGN16_DOWN(x) : (  \
		(level == 32) ? ALIGN32_DOWN(x) : (  \
		(level == 64) ? ALIGN64_DOWN(x) : (x) ))))))

#define ALIGNX_UP(level, x) (                \
		(level == 2) ? ALIGN2_UP(x) : (      \
		(level == 4) ? ALIGN4_UP(x) : (      \
		(level == 8) ? ALIGN8_UP(x) : (      \
		(level == 16) ? ALIGN16_UP(x) : (    \
		(level == 32) ? ALIGN32_UP(x) : (    \
		(level == 64) ? ALIGN64_UP(x) : (x)))))))

#define IS_ALIGNX(level, x) (              \
		(level == 1) ?     1        : (    \
		(level == 2) ? IS_ALIGN2(x) : (    \
		(level == 4) ? IS_ALIGN4(x) : (    \
		(level == 8) ? IS_ALIGN8(x) : (    \
		(level == 16) ? IS_ALIGN16(x) : (  \
		(level == 32) ? IS_ALIGN32(x) : (  \
		(level == 64) ? IS_ALIGN64(x) : 0 )))))))

#define MAX_VAL(a,b)        ((a) > (b) ? (a) : (b))
#define MIN_VAL(a,b)        ((a) < (b) ? (a) : (b))




#ifndef bool
#define bool    int
#endif
#ifndef TRUE
#define TRUE    (1==1)
#endif
#ifndef FALSE
#define FALSE   (1==0)
#endif


//the following input arguments(new_val & old_val) must be the type of "unsigned long"
#define TIME_DIFF(new_val, old_val)     ((long)(new_val) - (long)(old_val))

//the following input arguments(new_val & old_val) must be the type of "unsigned long long"
#define TIME64_DIFF(new_val, old_val)     ((long long)(new_val) - (long long)(old_val))


//find the the first bit in the u32 type
#define VG_FIND_U32_FIRST_BIT(u32_bitmap, number) \
	do { \
		int i;\
		(number) = -1;\
		for(i = 0; i < 32; i++)  \
			if ((u32_bitmap) & (1 << i)) { (number) = i; break; } \
	} while(0)


// Format alignment value
#define	FORMAT_ALIGN_VAL_YUV422_W         8
#define	FORMAT_ALIGN_VAL_YUV422_H         2
#define	FORMAT_ALIGN_VAL_YUV422_MB_W      16
#define	FORMAT_ALIGN_VAL_YUV422_MB_H      16
#define	FORMAT_ALIGN_VAL_YUV422_SCE_W     32
#define	FORMAT_ALIGN_VAL_YUV422_SCE_H     2

#define	FORMAT_ALIGN_VAL_YUV420_W         16
#define	FORMAT_ALIGN_VAL_YUV420_H         2
#define	FORMAT_ALIGN_VAL_YUV420_MB_W      16
#define	FORMAT_ALIGN_VAL_YUV420_MB_H      16
#define	FORMAT_ALIGN_VAL_YUV420_SCE_W     32
#define	FORMAT_ALIGN_VAL_YUV420_SCE_H     2
#define	FORMAT_ALIGN_VAL_YUV420_16X2_W    64
#define	FORMAT_ALIGN_VAL_YUV420_16X2_H    64
#define	FORMAT_ALIGN_VAL_YUV420_SP8_W     16
#define	FORMAT_ALIGN_VAL_YUV420_SP8_H     16

#define	FORMAT_ALIGN_VAL_ARGB1555_W       1
#define	FORMAT_ALIGN_VAL_ARGB1555_H       1
#define	FORMAT_ALIGN_VAL_ARGB8888_W       1
#define	FORMAT_ALIGN_VAL_ARGB8888_H       1
#define	FORMAT_ALIGN_VAL_RGB565_W         1
#define	FORMAT_ALIGN_VAL_RGB565_H         1
#define	FORMAT_ALIGN_VAL_RGB_CV_W         1
#define	FORMAT_ALIGN_VAL_RGB_CV_H         1

// Format size calculation
#define FORMAT_SIZE_YUV422(w, h)          ((ALIGN8_UP(w)) * (ALIGN2_UP(h)) * 2)
#define FORMAT_SIZE_YUV422_MB(w, h)       ((ALIGN16_UP(w)) * (ALIGN16_UP(h)) * 2)
#define FORMAT_SIZE_YUV422_SCE(w, h)      (((ALIGN32_UP(w) / 32) * 3 * 16) * ALIGN2_UP(h))
#define FORMAT_SIZE_YUV420_LLC_8x4(w, h)  (ALIGN4K_UP((ALIGN64_UP(w) * ALIGN64_UP(h) * 3 / 2))) // refer to macro VDODEC_SIZE_YUV420_LLC_8x4 in kflow_videodec

#define FORMAT_SIZE_YUV420_SP(w, h)       (ALIGN16_UP(w) * ALIGN2_UP(h) * 3 / 2)
#define FORMAT_SIZE_YUV420_MB(w, h)       (ALIGN16_UP(w) * ALIGN16_UP(h) * 3 / 2)
#define FORMAT_SIZE_YUV420_SCE(w, h)      (((((ALIGN32_UP(w) / 32) * 3 + 1) / 2) * 16) * ALIGN2_UP(h) * 3 / 2)
#define FORMAT_SIZE_YUV420_16X2(w, h)     (ALIGN64_UP(w) * ALIGN64_UP(h) * 3 / 2)
#define FORMAT_SIZE_YUV420_SP8(w, h)      (ALIGN64_UP(w) * ALIGN64_UP(h) * 3 / 2)

#define FORMAT_SIZE_ARGB1555(w, h)        ((w) * (h) * 2)
#define FORMAT_SIZE_ARGB8888(w, h)        ((w) * (h) * 4)
#define FORMAT_SIZE_RGB565(w, h)          ((w) * (h) * 2)
#define FORMAT_SIZE_RGB_CV(w, h)          ((w) * (h) * 3)

// Size calculation for specific data format
#define GET_DATA_SIZE_CODEC_MB_INFO(w, h, p_size)                       \
	do {                                                                \
		*(p_size) = (ALIGN64_UP(w) * ALIGN64_UP(h) / 16) + 4;           \
	} while(0)

#define	GET_DATA_SIZE_MD_EVENT_DATA(w, h, col_num, p_size)              \
	do {                                                                \
		int mb_size, mb_w_cnt, mb_h_cnt;                                \
		mb_size = ((w) <= 1920 && (h) <= 1080) ? 16 : 32;               \
		if ((mb_w_cnt = (w) / mb_size) < 256)                           \
			mb_w_cnt = 256;                                             \
		mb_h_cnt = (h) / mb_size;                                       \
		*(p_size) = ALIGN8_UP(mb_w_cnt * mb_h_cnt * (col_num + 1)) / 8; \
	} while(0)

#define	GET_DATA_SIZE_CAP_MD_EVENT_DATA(w, h, p_size)           \
	GET_DATA_SIZE_MD_EVENT_DATA(w, h, 1, p_size)

#define	GET_DATA_SIZE_VPE_MD_EVENT_DATA(w, h, p_size)           \
	GET_DATA_SIZE_MD_EVENT_DATA(w, h, 1, p_size)


#endif /* __COMMON_H__ */

