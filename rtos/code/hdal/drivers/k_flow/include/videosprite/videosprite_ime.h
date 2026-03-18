/**
    Image operation module.

    This module supports rendering osds and masks on video frame.

    @file       videosprite.h
    @ingroup    mIAppVideosprite

    Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#ifndef _VIDEO_SPRITE_IME_H_
#define _VIDEO_SPRITE_IME_H_

#include <kwrap/type.h>
#include "kflow_common/type_vdo.h"
#include "videosprite_internal.h"

typedef struct {
	UINT32                    en;
	VDS_INTERNAL_MASK_TYPE    type;
	UINT32                    is_mosaic;
	UINT32                    pos_num;
	UPOINT                    pos[10];
	UINT32                    color[3];
	UINT8                     alpha;
	UINT32                    thickness;
	UINT32                    mosaic_blk_size;
} VDS_INTERNAL_IME_MASK;

typedef struct {
	UINT32                   dirty;
	VDS_INTERNAL_IME_MASK    *data;
} VDS_TO_IME_MASK;

typedef struct {
	UINT32        en;
	VDO_PXLFMT    fmt;
	uintptr_t     addr;
	UPOINT        pos;
	USIZE         size;
	UINT32        ckey_en;
	UINT32        ckey_val;
	UINT16        bweight0;
	UINT16        bweight1;
} VDS_INTERNAL_IME_STAMP;

typedef struct {
	UINT32                    dirty;
	VDS_INTERNAL_IME_STAMP    *data;
} VDS_TO_IME_STAMP;

typedef struct {
	VDS_INTERNAL_EXT_STAMP    *stamp;
} VDS_TO_IME_GRH_STAMP;

typedef struct {
	UINT32                    dirty;
	VDS_TO_IME_GRH_STAMP      data;
} VDS_TO_IME_EXT_STAMP;

typedef struct {
	VDS_INTERNAL_EXT_MASK     *mask;
} VDS_TO_IME_GRH_MASK;

typedef struct {
	UINT32                    dirty;
	VDS_TO_IME_GRH_MASK       data;
} VDS_TO_IME_EXT_MASK;

typedef struct {
	UINT8                    enable;
	UINT32                   cent_x;
	UINT32                   cent_y;
	UINT32                   valid_r;
	UINT8                    decs_r;
} VDS_INTERNAL_IME_FISHEYE;

typedef struct {
	UINT32                   dirty;
	VDS_INTERNAL_IME_FISHEYE *data;
} VDS_TO_IME_FISHEYE;

extern int vds_max_ime_stamp;
extern int vds_max_ime_mask;

extern int vds_render_ime_context(VDS_QUERY_STAGE stage, uintptr_t y, uintptr_t uv, void* p_data, UINT32 w, UINT32 h, UINT32 *palette, UINT32 *loff, VDO_PXLFMT fmt);

#endif //_VIDEO_SPRITE_IME_H_