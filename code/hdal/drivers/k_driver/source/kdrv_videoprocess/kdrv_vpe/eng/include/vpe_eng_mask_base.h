#ifndef _VPE_ENG_MASK_BASE_H_
#define _VPE_ENG_MASK_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "kwrap/type.h"
#include "vpe_eng_handle.h"





typedef struct {
	UINT16 line_coeffa;
	UINT16 line_coeffb;
	UINT32 line_coeffc;
	UINT8 line_comp;
} VPE_MASK_LINE_PARAM;

typedef struct {
	UINT8 did;
	UINT8 pal_sel;
	UINT8 line_hit_op;
	UINT16 alpha;
	UINT8 shape;	
} VPE_MASK_WIN_PARAM;


typedef enum {
	VPE_MASK_MOSAIC_8_8      = 0,//mosic blk size = 8x8
	VPE_MASK_MOSAIC_16_16    = 1,//mosic blk size = 16x16
	VPE_MASK_MOSAIC_32_32	 = 2,//mosic blk size = 16x16
	VPE_MASK_MOSAIC_64_64	 = 3,//mosic blk size = 16x16
	VPE_MASK_MOSAIC_MAX
} VPE_MASK_MOSAIC_BLK_SIZE;



extern ER vpe_eng_set_bitmap_mask_mosaic_blk_size_buf_reg(VPE_ENG_HANDLE *p_eng, VPE_MASK_MOSAIC_BLK_SIZE mosic_blk_sz);
extern ER vpe_eng_set_mask_window_param_buf_reg(VPE_ENG_HANDLE *p_eng, UINT8 mask_idx, VPE_MASK_WIN_PARAM win_param);
extern ER vpe_eng_set_mask_line_param_buf_reg(VPE_ENG_HANDLE *p_eng, UINT8 mask_idx, UINT8 line_idx, VPE_MASK_LINE_PARAM line_param);

#ifdef __cplusplus
}
#endif


#endif //
