/**
    Image operation module.

    This module supports rendering osds and masks on video frame.

    @file       videosprite.h
    @ingroup    mIAppVideosprite

    Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#ifndef _VIDEO_SPRITE_ENC_H_
#define _VIDEO_SPRITE_ENC_H_

#include <kwrap/type.h>
#include "kflow_common/type_vdo.h"
#include "videosprite_internal.h"

typedef struct {
	VDO_PXLFMT    fmt;
	uintptr_t     vaddr;
	uintptr_t     paddr;
	UPOINT        pos;
	USIZE         size;
	UINT32        layer;
	UINT32        region;
	UINT32        ckey_en;
	UINT32        ckey_val;
	UINT32        qp_en;
	UINT32        qp_fix;
	UINT8         qp_val;
	UINT8         qp_lpm_mode;
	UINT8         qp_tnr_mode;
	UINT8         qp_fro_mode;
	UINT8         fg_alpha;
	UINT8         bg_alpha;
	UINT8         gcac_enable;
	UINT8         gcac_blk_width;
	UINT8         gcac_blk_height;
	UINT8         gcac_blk_num;
	UINT8         gcac_org_color_level;
	UINT8         gcac_inv_color_level;
	UINT8         gcac_nor_diff_th;
	UINT8         gcac_inv_diff_th;
	UINT8         gcac_sta_only_mode;
	UINT8         gcac_full_eval_mode;
	UINT8         gcac_eval_lum_targ;
} VDS_INTERNAL_COE_STAMP;

typedef struct {
	VDS_INTERNAL_MASK_TYPE    type;
	UINT32                    color;
	UINT32                    alpha;
	UINT32                    x;
	UINT32                    y;
	UINT32                    w;
	UINT32                    h;
	UINT32                    thickness;
	UINT32                    layer;
	UINT32                    region;
} VDS_INTERNAL_COE_MASK;

typedef struct {
	UINT32                 en;
	VDS_INTERNAL_MASK_TYPE type;
	UINT32                 is_mosaic;
	UINT32                 color;
	UPOINT                 pos[4];
	UINT32                 alpha;
	UINT32                 thickness;
	UINT32                 mosaic_blk_w;
	UINT32                 mosaic_blk_h;
} VDS_INTERNAL_COE_MASK2;

typedef struct {
	UINT32                    x;
	UINT32                    y;
	UINT32                    w;
	UINT32                    h;
	UINT32                    layer;
	UINT32                    region;
	UINT32                    mosaic_blk_w;
	UINT32                    mosaic_blk_h;
} VDS_INTERNAL_COE_MOSAIC;

typedef struct _VDS_INTERNAL_COE_STAMP_MASK {
	UINT32        en;
	UINT32        is_mask;
	UINT32        is_mosaic;
	union {
		VDS_INTERNAL_COE_STAMP     stamp;
		VDS_INTERNAL_COE_MASK      mask;
		VDS_INTERNAL_COE_MOSAIC    mosaic;
	} data;  
} VDS_INTERNAL_COE_STAMP_MASK;

typedef struct {
	UINT32                         dirty;
	VDS_INTERNAL_COE_STAMP_MASK    *stamp;
} VDS_TO_ENC_COE_STAMP;

typedef struct {
	UINT32                    dirty;
	VDS_INTERNAL_EXT_STAMP    *stamp;
} VDS_TO_ENC_EXT_STAMP;

typedef struct {
	UINT32                    dirty;
	VDS_INTERNAL_EXT_MASK     *mask;
} VDS_TO_ENC_EXT_MASK;

typedef struct {
	UINT32                    dirty;
	VDS_INTERNAL_COE_MASK2    *mask;
} VDS_TO_ENC_COE_MASK2;

extern int vds_max_coe_stamp;
extern int vds_max_coe_mask;

extern int vds_render_enc_ext(VDS_QUERY_STAGE stage, uintptr_t y, uintptr_t uv, void* p_data, UINT32 w, UINT32 h, UINT32 *palette, UINT32 *loff, VDO_PXLFMT fmt);
extern int vds_render_coe_grh(uintptr_t y, uintptr_t uv, VDS_INTERNAL_COE_STAMP_MASK *stamp, UINT32 w, UINT32 h, UINT32 *palette, UINT32 *loff, VDO_PXLFMT fmt);
extern int vds_get_venc_qp_mosaic(int vid);

#endif //_VIDEO_SPRITE_ENC_H_
