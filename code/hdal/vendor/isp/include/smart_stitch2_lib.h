#ifndef _SMART_STITCH_LIB2_
#define _SMART_STITCH_LIB2_

#include "hd_type.h"

//=============================================================================
// define
//=============================================================================

//=============================================================================
// struct & enum definition
//=============================================================================
typedef struct _SMART_STITCH_INFO{
	UINT32 frame_num;		//--- frame number
	size_t y_addr_l;		//--- left image y data addr
	size_t uv_addr_l;		//--- left image uv data addr
	UINT32 y_lofs_l;		//--- left image line offest
	UINT32 uv_lofs_l;		//--- left image uv line offest
	size_t y_addr_r;		//--- right image y data addr
	size_t uv_addr_r;		//--- right image uv data addr
	UINT32 y_lofs_r;		//--- right image y line offest
	UINT32 uv_lofs_r;		//--- right image uv line offest
	UINT32 blend_width;		//--- blend width
	UINT32 blend_height;	//--- blend height
} SMART_STITCH_INFO;

typedef struct _SMART_STITCH_INIT_PARAM
{
	UINT16 overlapID;
	UINT16 blend_width;
	UINT16 blend_height;
	UINT16 output_lut_width;
	UINT16 output_lut_height;
	char* vsp_data_path;
} SMART_STITCH_INIT_PARAM;

enum Y_Alignment_Status
{
	Y_Alignment_Disabled = 0,
	Y_Alignment_Not_Stable = 1,
	Y_Alignment_Half_Anchor_Stable = 2,
	Y_Alignment_ALL_Anchors_Stable = 3
};
//=============================================================================
// extern functions
//=============================================================================
#ifdef __cplusplus
extern "C" {
#endif

extern void* create_smart_stitch2(const SMART_STITCH_INIT_PARAM *init_param);
extern void *create_smart_stitch2_lite(const SMART_STITCH_INIT_PARAM *init_param,
										uint16_t x_search_range, uint16_t y_search_range, uint16_t y_ds_num,
										float overlap_transition_proportion,
										uint16_t preprocess_2dlut_size,
										uint16_t full_image_width_L, uint16_t full_image_width_R);

extern void destroy_smart_stitch2(void ** the_stitcher);
//--- the_stitcher is the pointer of one instance.
extern HD_RESULT smart_stitch2_process(void *the_stitcher, SMART_STITCH_INFO *smart_stitch_info, size_t lut2d_l_addr, size_t lut2d_r_addr);

extern HD_RESULT smart_stitch2_process_input(void *the_stitcher, SMART_STITCH_INFO *smart_stitch_info);
extern HD_RESULT smart_stitch2_process_output(void *the_stitcher, size_t lut2d_l_addr, size_t lut2d_r_addr);
extern HD_RESULT smart_stitch2_process_output_lite(void *the_stitcher, size_t lut2d_l_addr, size_t lut2d_r_addr);
extern HD_RESULT smart_stitch2_process_output_waitfinish(void *the_stitcher, size_t lut2d_l_addr, size_t lut2d_r_addr);

extern void smart_stitch2_lite_redo(void *the_stitcher);

//--- api:smart_stitch2_y_alignment_status
//--- Show whether the vertical alignment process is complete.
//--- Divide the overlapping area image into "6 vertical segments", with the segments numbered sequentially from top to bottom.
//--- The returned array has a size of 6. The 0th element represents the status of the topmost segments.
//--- Returned value - see Y_Alignment_Status
//--- 0: Disabled,				the Y alignment function is disabled.
//--- 1: Not_Stable, 			less than half of the anchors are stable.
//--- 2: Half_Anchor_Stable, 	more than half of the anchors are stable.
//--- 3: ALL_Anchors_Stable,	all of the anchors are stable.
extern const UINT8* smart_stitch2_y_alignment_status(void *the_stitcher);

extern void smart_stitch2_save_yalignment_map(void *the_stitcher);

//--- debug function, save the input source.
extern HD_RESULT smart_stitch2_record_src_img_enable(void *the_stitcher, char* output_dir);
extern void smart_stitch2_record_src_img_disable(void *the_stitcher);

extern void smart_stitch2_set_dbg_out(int on);

#ifdef __cplusplus
}
#endif

#endif

