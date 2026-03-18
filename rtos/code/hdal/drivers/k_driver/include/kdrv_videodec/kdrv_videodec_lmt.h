/**
 * @file kdrv_videodec_lmt.h
 * @brief type definition of KDRV API.
 * @author ALG2
 * @date in the year 2019
 */
#ifndef __KDRV_VIDEODEC_LMT_H__
#define __KDRV_VIDEODEC_LMT_H__

#include "kdrv_type.h"

#define H264D_WIDTH_MIN                     (64)
#define H264D_HEIGHT_MIN                    (64)
#define H264D_WIDTH_MAX                     (8192)
#define H264D_HEIGHT_MAX                    (8192)
#define H264D_FIRST_WIDTH_ALIGN             (64)
#define H264D_FIRST_HEIGHT_ALIGN            (64)
#define H264D_EXTRA_TRUE_WIDTH_ALIGN        (16)
#define H264D_EXTRA_SCE_WIDTH_ALIGN         (32)
#define H264D_EXTRA_HEIGHT_ALIGN            (2)

#define H265D_WIDTH_MIN                     (64)
#define H265D_HEIGHT_MIN                    (64)
#define H265D_WIDTH_MAX                     (8192)
#define H265D_HEIGHT_MAX                    (8192)
#define H265D_FIRST_WIDTH_ALIGN             (64)
#define H265D_FIRST_HEIGHT_ALIGN            (64)
#define H265D_EXTRA_TRUE_WIDTH_ALIGN        (16)
#define H265D_EXTRA_SCE_WIDTH_ALIGN         (32)
#define H265D_EXTRA_HEIGHT_ALIGN            (2)

#define H264D_FIRST_REF_BUF_ADDR_ALIGN      (128)
#define H264D_EXTRA_REF_BUF_ADDR_ALIGN      (16)
#define H264D_MBINFO_BUF_ADDR_ALIGN         (128)

#define H265D_FIRST_REF_BUF_ADDR_ALIGN      (128)
#define H265D_EXTRA_REF_BUF_ADDR_ALIGN      (16)
#define H265D_MBINFO_BUF_ADDR_ALIGN         (128)

#define H26XD_BS_BUF_ADDR_ALIGN             (128)

#define H264D_FIRST_TRUE_OUTPUT_FORMAT      (KDRV_BUFTYPE_YUV420_SP)
#define H264D_FIRST_LLC_OUTPUT_FORMAT       (KDRV_BUFTYPE_UNKNOWN)
#define H264D_EXTRA_TRUE_OUTPUT_FORMAT      (KDRV_BUFTYPE_YUV420_SP)
#define H264D_EXTRA_SCE_OUTPUT_FORMAT       (KDRV_BUFTYPE_YUV420_SCE)

#define H265D_FIRST_TRUE_OUTPUT_FORMAT      (KDRV_BUFTYPE_YUV420_16x2)
#define H265D_FIRST_LLC_OUTPUT_FORMAT       (KDRV_BUFTYPE_UNKNOWN)
#define H265D_EXTRA_TRUE_OUTPUT_FORMAT      (KDRV_BUFTYPE_YUV420_SP)
#define H265D_EXTRA_SCE_OUTPUT_FORMAT       (KDRV_BUFTYPE_YUV420_SCE)

//#define H264D_MBINFO_SIZE(w, h)	((((w+15) >> 4) << 4) * (((h+15) >> 4) << 4) * 5 >> 4)
/* sdk mode is not support B frame */
#define H264D_MBINFO_SIZE(w, h)             (0)
#define H265D_MBINFO_SIZE(w, h)             ((((w+63) >> 6) << 6) * (((h+63) >> 6) << 6) >> 6)


#endif	// __KDRV_VIDEODEC_LMT_H__

