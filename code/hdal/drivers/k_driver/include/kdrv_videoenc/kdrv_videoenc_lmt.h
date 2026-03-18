/**
 * @file kdrv_videoenc_lmt.h
 * @brief type definition of KDRV API.
 * @author ALG2
 * @date in the year 2019
 */
#ifndef __KDRV_VIDEOENC_LMT_H__
#define __KDRV_VIDEOENC_LMT_H__

#include "comm/drv_lmt.h"

#define H264E_WIDTH_MIN			(320)	//min encode width	(after rotation)
#define H264E_HEIGHT_MIN		(240)	//min encode height	(after rotation)
#define H264E_WIDTH_MAX			(6144)
#define H264E_HEIGHT_MAX		(6144)
#define H264E_WIDTH_ALIGN		(DRV_LIMIT_ALIGN_4WORD)
#define H264E_HEIGHT_ALIGN		(DRV_LIMIT_ALIGN_2WORD)
#define H264E_TILE_MAX			(1)

#define H264E_SCE_WIDTH_ALIGN	(32)
#define H264E_SCE_HEIGHT_ALIGN	(8)

#define H265E_WIDTH_MIN			(320)	//min encode width	(after rotation)
#define H265E_HEIGHT_MIN		(240)	//min encode height	(after rotation)
#define H265E_WIDTH_MAX			(6144)
#define H265E_HEIGHT_MAX		(6144)
#define H265E_WIDTH_ALIGN		(DRV_LIMIT_ALIGN_4WORD)
#define H265E_HEIGHT_ALIGN		(DRV_LIMIT_ALIGN_2WORD)
#ifndef VDOCDC_EMU
#define H265E_TILE_MAX			(2)
#else
#define H265E_TILE_MAX			(4)
#endif

#define H265E_SCE_WIDTH_ALIGN	(32)
#define H265E_SCE_HEIGHT_ALIGN	(8)

#define H265E_MAX_WIDTH_WITHOUT_TILE	(6144)

#define H265E_D2D_MAX_W_1_TILE			(3968)
#define H265E_D2D_MAX_W_2_TILE			(3968)

#define H264E_WIDTH_MIN_528            (240)   //min encode width
#define H264E_HEIGHT_MIN_528           (240)   //min encode height
#define H264E_WIDTH_MAX_528            (5120)
#define H264E_HEIGHT_MAX_528           (5120)

#define H265E_WIDTH_MIN_528            (240)   //min encode width
#define H265E_HEIGHT_MIN_528           (240)   //min encode height
#define H265E_WIDTH_MAX_528            (5120)
#define H265E_HEIGHT_MAX_528           (5120)
#define H265E_TILE_MAX_528             (5)

#define H265E_MAX_WIDTH_WITHOUT_TILE_528       (2176)

#define H264E_WIDTH_MIN_560            (208)   //min encode width
#define H264E_HEIGHT_MIN_560           (208)   //min encode height
#define H264E_WIDTH_MAX_560            (4096)
#define H264E_HEIGHT_MAX_560           (4096)

#define H265E_WIDTH_MIN_560            (208)   //min encode width
#define H265E_HEIGHT_MIN_560           (208)   //min encode height
#define H265E_WIDTH_MAX_560            (4096)
#define H265E_HEIGHT_MAX_560           (4096)
#define H265E_TILE_MAX_560             (5)

#define H265E_GDC_MIN_W_1_TILE_560			(1088)
#define H265E_GDC_MIN_W_2_TILE_560			(1728)
#define H265E_D2D_MIN_W_1_TILE_560			(2688)

#define H265E_MAX_WIDTH_WITHOUT_TILE_560       (2048)

#define H264E_WIDTH_MIN_530            (208)   //min encode width
#define H264E_HEIGHT_MIN_530           (208)   //min encode height
#define H264E_WIDTH_MAX_530            (8192)
#define H264E_HEIGHT_MAX_530           (8192)

#define H264E_MIN_WIDTH_SRH256         (640)
#define H264E_MAX_WIDTH_WITHOUT_TILE_530       (5120)

#define H264E_MIN_LTW_SRV76_SRH256			(1024)
#define H264E_MAX_LTW_SRV76_SRH256			(2048)
#define H264E_MAX_RTW_SRV76_SRH256			(2176)

#define H264E_MAX_2_TW_SRV76_SRH256			(H264E_MAX_LTW_SRV76_SRH256+H264E_MAX_RTW_SRV76_SRH256)

#define H264E_MIN_LTW_SRV76_SRH128			(768)
#define H264E_MAX_LTW_SRV76_SRH128			(2304)
#define H264E_MAX_RTW_SRV76_SRH128			(2304)

#define H264E_MAX_2_TW_SRV76_SRH128			(H264E_MAX_LTW_SRV76_SRH128+H264E_MAX_RTW_SRV76_SRH128)

#define H264E_MIN_LTW_SRV36_SRH256			(1024)
#define H264E_MAX_LTW_SRV36_SRH256			(4096)
#define H264E_MAX_RTW_SRV36_SRH256			(4096)

#define H264E_MAX_2_TW_SRV36_SRH256			(H264E_MAX_LTW_SRV36_SRH256+H264E_MAX_RTW_SRV36_SRH256)
#define H264E_MAX_W_WITHOUT_TILE_V36_530	(4096)

#ifdef VDOCDC_LL
#define H264E_SUPPORT_FRAME_PIPE		(0)
#else
#define H264E_SUPPORT_FRAME_PIPE		(0)
#endif

#define H265E_WIDTH_MIN_530            (208)   //min encode width
#define H265E_HEIGHT_MIN_530           (208)   //min encode height
#define H265E_WIDTH_MAX_530            (8192)
#define H265E_HEIGHT_MAX_530           (8192)
#define H265E_TILE_MAX_530             (4)

// jira-id: NA51102-266
#define H26XE_GDC_MAX_W_1_TILE_530			(1920)
#define H26XE_GDC_MAX_W_2_TILE_530			(2880)
#define H265E_GDC_MAX_W_3_TILE_530			(4096)
#define H26XE_D2D_MAX_W_1_TILE_530			(2432)
#define H26XE_D2D_MAX_W_2_TILE_530			(7680)
#define H26XE_D2D_W_2_TILE_SRH_128_SRV_76_530			(4608)

#define H265E_MAX_WIDTH_WITHOUT_TILE_530       (5120)

#define H264E_WIDTH_MIN_538            (208)   //min encode width
#define H264E_HEIGHT_MIN_538           (144)   //min encode height
#define H264E_WIDTH_MAX_538            (6144)
#define H264E_HEIGHT_MAX_538           (6144)
#define H264E_WIDTH_MIN_539            (208)   //min encode width
#define H264E_HEIGHT_MIN_539           (144)   //min encode height
#define H264E_WIDTH_MAX_539            (8192)
#define H264E_HEIGHT_MAX_539           (8192)

#define H265E_WIDTH_MIN_538            (208)   //min encode width
#define H265E_HEIGHT_MIN_538           (144)   //min encode height
#define H265E_WIDTH_MAX_538            (6144)
#define H265E_HEIGHT_MAX_538           (6144)
#define H265E_WIDTH_MIN_539            (208)   //min encode width
#define H265E_HEIGHT_MIN_539           (144)   //min encode height
#define H265E_WIDTH_MAX_539            (8192)
#define H265E_HEIGHT_MAX_539           (8192)
#define H265E_TILE_MAX_538             (2)

#define H265E_MAX_LTW_SRV76_SRH256			(2048)
#define H265E_MAX_RTW_SRV76_SRH256			(2176)
#define H265E_MAX_MTW_SRV76_SRH256			(1792)

#define H265E_MAX_2_TW_SRV76_SRH256			(H265E_MAX_LTW_SRV76_SRH256+H265E_MAX_RTW_SRV76_SRH256)
#define H265E_MAX_3_TW_SRV76_SRH256			(H265E_MAX_LTW_SRV76_SRH256+H265E_MAX_RTW_SRV76_SRH256+H265E_MAX_MTW_SRV76_SRH256)
#define H265E_MAX_4_TW_SRV76_SRH256			(H265E_MAX_LTW_SRV76_SRH256+H265E_MAX_RTW_SRV76_SRH256+H265E_MAX_MTW_SRV76_SRH256+H265E_MAX_MTW_SRV76_SRH256)
#define H265E_MAX_WIDTH_SRV76_SRH256        (H265E_MAX_4_TW_SRV76_SRH256)

#define H265E_MAX_LTW_SRV36_SRH256			(4096)
#define H265E_MAX_RTW_SRV36_SRH256			(4096)
#define H265E_MAX_MTW_SRV36_SRH256			(3072)

#define H265E_MAX_2_TW_SRV36_SRH256			(H265E_MAX_LTW_SRV36_SRH256+H265E_MAX_RTW_SRV36_SRH256)

#define H26X_MAX_W_WITHOUT_TILE_V76_530 (2432)
#define H26X_MAX_W_WITHOUT_TILE_V36_530 (5120)

#define H26XE_BUF_ADDR_ALIGN	(DRV_LIMIT_ALIGN_WORD)
#define H26XE_BS_BUF_ADDR_ALIGN	(DRV_LIMIT_ALIGN_WORD)

#define H26XE_ROTATE_90			(1)
#define H26XE_ROTATE_180		(1)
#define H26XE_ROTATE_270		(1)


#define H264E_ROTATE_MAX_HEIGHT		(1984)
#define H264E_ROTATE_MAX_HEIGHT_528    (2176)
#define H264E_ROTATE_MAX_HEIGHT_560    (2176)
#define H264E_ROTATE_MAX_HEIGHT_530    (4096)
#define H264E_ROTATE_MAX_HEIGHT_538    (2176)

#define H26XE_QP_MAP_UNIT		(16)

#define H26XE_ROI_WIN_MAX_NUM	(10)

#define H26XE_OSG_WIN_MAX_NUM	(32)
#define H26XE_OSG_PAL_MAX_NUM	(16)
#define H26XE_OSG_WIDTH_ALIGN	(2)		//should be multiple of 2
#define H26XE_OSG_HEIGHT_ALIGN	(2)		//should be multiple of 2
#define H26XE_OSG_POSITION_X_ALIGN	(2)		//should be multiple of 2
#define H26XE_OSG_POSITION_Y_ALIGN	(2)		//should be multiple of 2
#define H26XE_OSG_X_ALIGN		(2)
#define H26XE_OSG_Y_ALIGN		(2)
#define H26XE_OSG_LOFS_ALIGN	(DRV_LIMIT_ALIGN_WORD)
#define H26XE_OSG_ADR_ALIGN	(DRV_LIMIT_ALIGN_2WORD)
#define H26XE_OSG_TYPE_MASK		0x7D	// bit0: argb1555, bit1: argb8888, bit2: argb4444, bit3: rgb565
										// bit4: 1 bit palette, bit5: 2 bit palette, bit6: 4 bit palette

#define H26XE_MASK_WIN_MAX_NUM		(8)
#define H26XE_MASK_WIDTH_ALIGN      (2)
#define H26XE_MASK_HEIGHT_ALIGN     (2)
#define H26XE_MASK_X_ALIGN          (2)
#define H26XE_MASK_Y_ALIGN          (2)

#define H26XE_MOT_BUF_MAX_NUM	(3)

// H265E_HEIGHT_MAX_530/64*H265E_TILE_MAX_530
#define H26XE_SLICE_MAX_NUM		(512)

#endif	// __KDRV_VIDEOENC_LMT_H__
