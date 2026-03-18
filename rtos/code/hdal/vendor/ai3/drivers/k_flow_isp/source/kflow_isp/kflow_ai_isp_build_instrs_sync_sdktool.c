/**
    @brief Source file of kflow_isp.

    @file kflow_ai_isp_build_instrs_sync_sdktool.c

    @ingroup kflow_isp

    @note Nothing.

    Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/

#include "kwrap/type.h"
#include "kwrap/error_no.h"
#include "kwrap/file.h"
#include "kwrap/platform.h"
#include "kwrap/semaphore.h"
#include "kwrap/cpu.h"

#include "kdrv_ai.h"
#include "kflow_ai_isp_build_instrs_sync_sdktool.h"
#include <linux/string.h>
#include "kflow_isp/kflow_isp_platform.h"

//=============================================================
#define __CLASS__ 				"[ai][kflow_isp][isp]"
#include "kflow_ai_isp_debug.h"
//=============================================================


#if 1

#define ASSERT_MSG(cond, msg)        	 \
	do {                                 \
	  if (!(cond)) {                     \
		DBG_DUMP("%s", msg);				 \
	  }                                  \
	} while (false)

#else
#endif


#define NULL_MODE               0x0
#define UPD_MODE                0x1
#define NEXTLL_MODE             0x3

#define PPU_CONTROL_REG_OFS     0x0C
#define PPU_NR_PRE_REG_OFS      0x10
#define PPU_LOFS_EN_REG_OFS     0x88
#define PPU_IN0_LOFS_REG_OFS    0x8C
#define PPU_IN1_LOFS_REG_OFS    0x90
#define PPU_IN2_LOFS_REG_OFS    0x94
#define PPU_OUT0_LOFS_REG_OFS   0x98
#define PPU_OUT1_LOFS_REG_OFS   0x9C
#define PPU_OUT2_LOFS_REG_OFS   0xA0
#define PPU_WIDTH_REG_OFS       0xA4
#define PPU_HEIGHT_REG_OFS      0xA8
#define PPU_PAD_0_REG_OFS       0x118
#define PPU_PAD_1_REG_OFS       0x11C
#define PPU_CROP_0_REG_OFS      0x12C
#define PPU_CROP_1_REG_OFS      0x130
#define PPU_SAI0_REG_OFS        0x50
#define PPU_SAO0_REG_OFS        0x68
#define PPU_QUAN_OUT0_REG_OFS   0x22C
#define PPU_QUAN_OUT1_REG_OFS   0x230
#define PPU_QUAN_OUT2_REG_OFS   0x234

#define POU_TUNING_REG_OFS      0x14
#define POU_LOFS_EN_REG_OFS     0x88
#define POU_OUT0_LOFS_REG_OFS   0x98
#define POU_CROP_0_REG_OFS      0x12C
#define POU_CROP_1_REG_OFS      0x130

const uint64_t g_instr_mask = 0xffffffff;


typedef struct _JMISP_INSTR_CASE {
    JMISP_INSTR_STRIPE_CASE_TYPE stripe_case;
    JMISP_INSTR_BLOCK_CASE_TYPE block_case;
} JMISP_INSTR_CASE;


int32_t gen_block_case_table(JMISP_INSTR_CASE* p_table, int32_t table_sz, JMISPPPU_HEAD* p_ppu_head, AI_JMISP_INFO* p_jmisp_info);


e_AI_JMISP_IOBUF_CONST get_benchmark_iobuf_type(VOID) {
    return e_AI_JMISP_IOBUF_RING_IN0;
}

AI_JMISP_IOBUF_INFO* get_benchmark_iobuf(AI_JMISP_INFO* p_jmisp_info) {
    AI_JMISP_IOBUF_INFO* p_benchmark_iobuf = NULL;
    if (p_jmisp_info == NULL) {
        return p_benchmark_iobuf;
    }
    p_benchmark_iobuf = &p_jmisp_info->iobuf[get_benchmark_iobuf_type()];
    if (p_benchmark_iobuf->buffer_sz == 0) {
        ASSERT_MSG(0, "Benchmark buffer should not be empty\n");
    }
    return p_benchmark_iobuf;
}

e_AI_JMISP_REG_IO_CONST convert_iobuf2regbuf_type(AI_JMISP_IOBUF_INFO* p_iobuf_info, e_AI_JMISP_IOBUF_CONST iobuf_type) {
    switch (iobuf_type) {
    case e_AI_JMISP_IOBUF_RING_IN0:
        return e_AI_JMISP_REG_IO_RING_BUF0;

    case e_AI_JMISP_IOBUF_RING_IN1:
        return e_AI_JMISP_REG_IO_RING_BUF1;

    case e_AI_JMISP_IOBUF_REF_IN0:
    case e_AI_JMISP_IOBUF_FUSION_WEIGHT_IN:
        return e_AI_JMISP_REG_IO_FRAME_BUF0;

    case e_AI_JMISP_IOBUF_REF_IN1:
    case e_AI_JMISP_IOBUF_SIGMA_IN:
        return e_AI_JMISP_REG_IO_FRAME_BUF1;

    case e_AI_JMISP_IOBUF_REF_OUT0:
    case e_AI_JMISP_IOBUF_GAMMA_IN:
        return e_AI_JMISP_REG_IO_FRAME_BUF2;

    case e_AI_JMISP_IOBUF_REF_OUT1:
    case e_AI_JMISP_IOBUF_GAMMA_OUT:
        return e_AI_JMISP_REG_IO_FRAME_BUF3;

    case e_AI_JMISP_IOBUF_PINGPONG_OUT0:
        return e_AI_JMISP_REG_IO_PINGPONG_BUF0;

    case e_AI_JMISP_IOBUF_PINGPONG_OUT1:
        return e_AI_JMISP_REG_IO_PINGPONG_BUF1;

    case e_AI_JMISP_IOBUF_PINGPONG_OUT2:
        return e_AI_JMISP_REG_IO_PINGPONG_BUF2;

    case e_AI_JMISP_IOBUF_PPU_PIPEBUF0:
        return e_AI_JMISP_REG_IO_CUSTPIPE_BUF0;

    case e_AI_JMISP_IOBUF_PPU_PIPEBUF1:
        return e_AI_JMISP_REG_IO_CUSTPIPE_BUF1;

    default:
        return e_AI_JMISP_REG_IO_UNKNOWN;
    }
}

BOOL is_in_buf(AI_JMISP_IOBUF_INFO* p_iobuf_info, e_AI_JMISP_IOBUF_CONST type) {
    if (p_iobuf_info == NULL) {
        return FALSE;
    }
    switch (type) {
    case e_AI_JMISP_IOBUF_RING_IN0:
    case e_AI_JMISP_IOBUF_RING_IN1:
    case e_AI_JMISP_IOBUF_REF_IN0:
    case e_AI_JMISP_IOBUF_FUSION_WEIGHT_IN:
    case e_AI_JMISP_IOBUF_REF_IN1:
    case e_AI_JMISP_IOBUF_SIGMA_IN:
    case e_AI_JMISP_IOBUF_GAMMA_IN:
        return TRUE;
    default:
        return FALSE;
    }
}

BOOL is_out_buf(AI_JMISP_IOBUF_INFO* p_iobuf_info, e_AI_JMISP_IOBUF_CONST type) {
    if (p_iobuf_info == NULL) {
        return FALSE;
    }
    switch (type) {
    case e_AI_JMISP_IOBUF_REF_OUT0:
    case e_AI_JMISP_IOBUF_REF_OUT1:
    case e_AI_JMISP_IOBUF_PINGPONG_OUT0:
    case e_AI_JMISP_IOBUF_PINGPONG_OUT1:
    case e_AI_JMISP_IOBUF_PINGPONG_OUT2:
    case e_AI_JMISP_IOBUF_GAMMA_OUT:
        return TRUE;
    default:
        return FALSE;
    }
}

BOOL is_ring_in_buf(AI_JMISP_IOBUF_INFO* p_iobuf_info, e_AI_JMISP_IOBUF_CONST type) {
    e_AI_JMISP_REG_IO_CONST reg_io_type;
    if (!is_in_buf(p_iobuf_info, type)) {
        return FALSE;
    }

    reg_io_type = convert_iobuf2regbuf_type(p_iobuf_info, type);
    switch (reg_io_type) {
    case e_AI_JMISP_REG_IO_RING_BUF0:
    case e_AI_JMISP_REG_IO_RING_BUF1:
        return TRUE;
    default:
        return FALSE;
    }
}

BOOL is_ring_out_buf(AI_JMISP_IOBUF_INFO* p_iobuf_info, e_AI_JMISP_IOBUF_CONST type) {
    return FALSE;
}

BOOL is_pingpong_in_buf(AI_JMISP_IOBUF_INFO* p_iobuf_info, e_AI_JMISP_IOBUF_CONST type) {
    return FALSE;
}

BOOL is_pingpong_out_buf(AI_JMISP_IOBUF_INFO* p_iobuf_info, e_AI_JMISP_IOBUF_CONST type) {
    e_AI_JMISP_REG_IO_CONST reg_io_type;
    if (!is_out_buf(p_iobuf_info, type)) {
        return FALSE;
    }

    reg_io_type = convert_iobuf2regbuf_type(p_iobuf_info, type);
    switch (reg_io_type) {
    case e_AI_JMISP_REG_IO_PINGPONG_BUF0:
    case e_AI_JMISP_REG_IO_PINGPONG_BUF1:
    case e_AI_JMISP_REG_IO_PINGPONG_BUF2:
        return TRUE;
    default:
        return FALSE;
    }
}

BOOL is_frame_in_buf(AI_JMISP_IOBUF_INFO* p_iobuf_info, e_AI_JMISP_IOBUF_CONST type) {
    e_AI_JMISP_REG_IO_CONST reg_io_type;
    if (!is_in_buf(p_iobuf_info, type)) {
        return FALSE;
    }

    reg_io_type = convert_iobuf2regbuf_type(p_iobuf_info, type);
    switch (reg_io_type) {
    case e_AI_JMISP_REG_IO_FRAME_BUF0:
    case e_AI_JMISP_REG_IO_FRAME_BUF1:
    case e_AI_JMISP_REG_IO_FRAME_BUF2:
    case e_AI_JMISP_REG_IO_FRAME_BUF3:
        return TRUE;
    default:
        return FALSE;
    }
}

BOOL is_frame_out_buf(AI_JMISP_IOBUF_INFO* p_iobuf_info, e_AI_JMISP_IOBUF_CONST type) {
    e_AI_JMISP_REG_IO_CONST reg_io_type;
    if (!is_out_buf(p_iobuf_info, type)) {
        return FALSE;
    }

    reg_io_type = convert_iobuf2regbuf_type(p_iobuf_info, type);
    switch (reg_io_type) {
    case e_AI_JMISP_REG_IO_FRAME_BUF0:
    case e_AI_JMISP_REG_IO_FRAME_BUF1:
    case e_AI_JMISP_REG_IO_FRAME_BUF2:
    case e_AI_JMISP_REG_IO_FRAME_BUF3:
        return TRUE;
    default:
        return FALSE;
    }
}

BOOL is_ring_buf(AI_JMISP_IOBUF_INFO* p_iobuf_info, e_AI_JMISP_IOBUF_CONST type) {
        if (is_ring_out_buf(p_iobuf_info, type) || is_ring_in_buf(p_iobuf_info, type)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL is_pingpong_buf(AI_JMISP_IOBUF_INFO* p_iobuf_info, e_AI_JMISP_IOBUF_CONST type) {
    if (is_pingpong_out_buf(p_iobuf_info, type) || is_pingpong_in_buf(p_iobuf_info, type)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL is_frame_buf(AI_JMISP_IOBUF_INFO* p_iobuf_info, e_AI_JMISP_IOBUF_CONST type) {
    if (is_frame_in_buf(p_iobuf_info, type) || is_frame_out_buf(p_iobuf_info, type)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL is_cust_pipe_buf(AI_JMISP_IOBUF_INFO* p_iobuf_info, e_AI_JMISP_IOBUF_CONST type) {
    e_AI_JMISP_REG_IO_CONST reg_io_type = convert_iobuf2regbuf_type(p_iobuf_info, type);
    switch (reg_io_type) {
    case e_AI_JMISP_REG_IO_CUSTPIPE_BUF0:
    case e_AI_JMISP_REG_IO_CUSTPIPE_BUF1:
        return TRUE;
    default:
        return FALSE;
    }
}

BOOL is_jmisp_sup_buf(AI_JMISP_IOBUF_INFO* p_iobuf_info, e_AI_JMISP_IOBUF_CONST type) {
    if (is_ring_buf(p_iobuf_info, type) || is_pingpong_buf(p_iobuf_info, type) || is_frame_buf(p_iobuf_info, type)) {
        return TRUE;
    } else {
        return FALSE;
    }
}


uint32_t gen_jmisp_stripe_num(AI_JMISP_INFO* p_jmisp_info) {
    AI_JMISP_IOBUF_INFO* p_temp_iobuf;
    uint32_t stripe_num = 0;
    p_temp_iobuf = get_benchmark_iobuf(p_jmisp_info);
    if (p_temp_iobuf == NULL) {
        return stripe_num;
    }

    if (p_temp_iobuf->stripeInfo.last_stripe_width == 0) {
        stripe_num = 1;
    }
    else if (p_temp_iobuf->stripeInfo.mid_stripe_width == 0) {
        stripe_num = 2;
    }
    else {
        stripe_num = (p_temp_iobuf->frameInfo.frame_width - p_temp_iobuf->stripeInfo.first_stripe_width - p_temp_iobuf->stripeInfo.last_stripe_width);
        stripe_num += p_temp_iobuf->stripeInfo.stripe_overlap_width;
        stripe_num /= (p_temp_iobuf->stripeInfo.mid_stripe_width - p_temp_iobuf->stripeInfo.stripe_overlap_width);
        stripe_num += 2;
    }
    return stripe_num;
}

uint32_t gen_jmisp_slice_num(AI_JMISP_INFO* p_jmisp_info) {
    AI_JMISP_IOBUF_INFO* p_temp_iobuf;
    uint32_t slice_num = 0;
    p_temp_iobuf = get_benchmark_iobuf(p_jmisp_info);
    if (p_temp_iobuf == NULL) {
        return slice_num;
    }

    slice_num = p_temp_iobuf->frameInfo.frame_height / p_temp_iobuf->stripeInfo.output_slice_height;
    if ((p_temp_iobuf->frameInfo.frame_height % p_temp_iobuf->stripeInfo.output_slice_height) != 0) {
        ++slice_num;
    }
    return slice_num;
}

int32_t jmisp_align_pos_ofs(int32_t ori_ofs, int32_t bitdepth) {
    int32_t new_ofs = ori_ofs * bitdepth;
    if (new_ofs & 0x7) {
        ASSERT_MSG(0, "JMISP Stripe/Slice/Block offset should be byte aligned\n");
    }
    new_ofs = new_ofs >> 3;
    return new_ofs;
}

int32_t jmisp_gen_loop_instr_input_blocks(const e_AI_JMISP_IOBUF_CONST iobuf_info_type, AI_JMISP_IOBUF_INFO* p_iobuf_info, JMISP_REG_IOBUF* p_reg_buf, JMISP_INSTR_BLOCK_CASE_INFO* p_blks_lhs) {

    int32_t block_width, block_height, block_overlap_width, block_overlap_height;
    int32_t block_width_without_overlap, block_height_without_overlap;
    int32_t first_stripe_block_x_num, mid_stripe_block_x_num, last_stripe_block_x_num;
    int32_t first_slice_block_y_num, last_slice_block_y_num;
	int32_t stripe_idx, block_idx, bound_idx;
    int32_t half_overlap_slice_h;
    int32_t exist_last_slice;
	
	if ((p_iobuf_info == NULL) || (p_reg_buf == NULL) || (p_blks_lhs == NULL)) {
		return -1;
	}

    exist_last_slice = (p_reg_buf->last_slice_h == 0) ? 0 : 1;
	
    block_width = p_iobuf_info->blockInfo.block_width;
    block_height = p_iobuf_info->blockInfo.block_height;
    block_overlap_width = p_iobuf_info->blockInfo.block_overlap_width;
    block_overlap_height = p_iobuf_info->blockInfo.block_overlap_height;
    block_width_without_overlap = block_width - block_overlap_width;
    block_height_without_overlap = block_height - block_overlap_height;
    first_stripe_block_x_num = ALIGN_CEIL_DIV(p_reg_buf->first_stripe_w, block_width_without_overlap);
    mid_stripe_block_x_num = ALIGN_CEIL_DIV(p_reg_buf->mid_stripe_w, block_width_without_overlap);
    last_stripe_block_x_num = ALIGN_CEIL_DIV(p_reg_buf->last_stripe_w, block_width_without_overlap);
    
    half_overlap_slice_h = p_iobuf_info->stripeInfo.slice_overlap_height >> 1;
    first_slice_block_y_num = ALIGN_CEIL_DIV(p_reg_buf->first_slice_h - half_overlap_slice_h, block_height_without_overlap);
    //mid_slice_block_y_num = ALIGN_CEIL_DIV(mid_slice_height - p_iobuf_info->stripeInfo.slice_overlap_height, block_height_without_overlap);
    last_slice_block_y_num = ALIGN_CEIL_DIV(p_reg_buf->last_slice_h - half_overlap_slice_h, block_height_without_overlap);
	

    for (stripe_idx = 0; stripe_idx < e_STRIPECASE_NUM; ++stripe_idx) {
        const JMISP_INSTR_STRIPE_CASE_TYPE stripe_case = (JMISP_INSTR_STRIPE_CASE_TYPE)stripe_idx;
        for (block_idx = 0; block_idx < e_BLOCKCASE_NUM; ++block_idx) {
            const JMISP_INSTR_BLOCK_CASE_TYPE block_case = (JMISP_INSTR_BLOCK_CASE_TYPE)block_idx;
            JMISP_INSTR_BLOCK_INFO* p_in_blk = &p_blks_lhs->blocks[stripe_case][block_case];
			
			//init
			p_in_blk->width = 0;
			p_in_blk->height = 0;
			for (bound_idx=0; bound_idx<e_BOUND_NUM; ++bound_idx) {
				p_in_blk->pad[bound_idx] = 0;
				p_in_blk->crop[bound_idx] = 0;
			}

            if ((block_case == e_BLOCKCASE_LEFT_TOP) || (block_case == e_BLOCKCASE_LEFT) || (block_case == e_BLOCKCASE_BOTTOM_LEFT)) {
                p_in_blk->width = block_width - (block_overlap_width >> 1);
                p_in_blk->pad[e_BOUND_LEFT] = (block_overlap_width >> 1);
            }
            else if ((block_case == e_BLOCKCASE_TOP_RIGHT) || (block_case == e_BLOCKCASE_RIGHT) || (block_case == e_BLOCKCASE_RIGHT_BOTTOM)) {
                int32_t last_block_width;
                if (stripe_case == e_STRIPECASE_LEFT) {
                    last_block_width = p_reg_buf->first_stripe_w - first_stripe_block_x_num * block_width_without_overlap;
                }
                else if (stripe_case == e_STRIPECASE_RIGHT) {
                    last_block_width = p_reg_buf->last_stripe_w - last_stripe_block_x_num * block_width_without_overlap;
                }
                else {
                    last_block_width = p_reg_buf->mid_stripe_w - mid_stripe_block_x_num * block_width_without_overlap;
                }

                if (last_block_width == 0) {
                    p_in_blk->width = block_width - (block_overlap_width >> 1);
                }
                else if (last_block_width > 0) {
                    p_in_blk->width = last_block_width + (block_overlap_width >> 1);
                }
                else if (last_block_width < 0) {
                    p_in_blk->width = block_width - (block_overlap_width >> 1) + last_block_width;
                }
                p_in_blk->pad[e_BOUND_RIGHT] = block_width - p_in_blk->width;
            }
            else {
                p_in_blk->width = block_width;
            }


            if (((block_case == e_BLOCKCASE_LEFT_TOP) || (block_case == e_BLOCKCASE_TOP) || (block_case == e_BLOCKCASE_TOP_RIGHT))) {
                int32_t first_block_height;
                if (exist_last_slice) {
                    first_block_height = p_reg_buf->first_slice_h - half_overlap_slice_h - first_slice_block_y_num * block_height_without_overlap;
                } else {
                    first_block_height = p_reg_buf->first_slice_h - first_slice_block_y_num * block_height_without_overlap;
                }

                if (first_block_height == 0) {
                    p_in_blk->height = block_height - (block_overlap_height >> 1);
                }
                else if (first_block_height > 0) {
                    p_in_blk->height = first_block_height + (block_overlap_height >> 1);
                }
                else if (first_block_height < 0) {
                    p_in_blk->height = block_height - (block_overlap_height >> 1) + first_block_height;
                }
                p_in_blk->pad[e_BOUND_TOP] = block_height - p_in_blk->height;
            }
            else if ( ((block_case == e_BLOCKCASE_BOTTOM_LEFT) || (block_case == e_BLOCKCASE_BOTTOM) || (block_case == e_BLOCKCASE_RIGHT_BOTTOM)) ) {
                int32_t last_block_height;
                if (exist_last_slice) {
                    last_block_height = p_reg_buf->last_slice_h - half_overlap_slice_h - last_slice_block_y_num * block_height_without_overlap;
                } else {
                    last_block_height = block_height_without_overlap;
                }

                if (last_block_height == 0) {
                    p_in_blk->height = block_height - (block_overlap_height >> 1);
                }
                else if (last_block_height > 0) {
                    p_in_blk->height = last_block_height + (block_overlap_height >> 1);
                }
                else if (last_block_height < 0) {
                    p_in_blk->height = block_height - (block_overlap_height >> 1) + last_block_height;
                }
                p_in_blk->pad[e_BOUND_BOTTOM] = block_height - p_in_blk->height;
            }
            else {
                p_in_blk->height = block_height;
            }
#if (0)
            ASSERT_MSG((p_in_blk->pad[e_BOUND_TOP] & 0x1) == 0, "Top pad should be even\n");
            ASSERT_MSG((p_in_blk->pad[e_BOUND_RIGHT] & 0x1) == 0, "Right pad should be even\n");
            ASSERT_MSG((p_in_blk->pad[e_BOUND_BOTTOM] & 0x1) == 0, "Bottom pad should be even\n");
            ASSERT_MSG((p_in_blk->pad[e_BOUND_LEFT] & 0x1) == 0, "Left pad should be even\n");
#endif
        }
    }


    return 0;
}

int32_t jmisp_gen_loop_instr_output_blocks(const e_AI_JMISP_IOBUF_CONST iobuf_info_type, AI_JMISP_IOBUF_INFO* p_iobuf_info, JMISP_REG_IOBUF* p_reg_buf, JMISP_INSTR_BLOCK_CASE_INFO* p_blks_lhs) {

    int32_t block_width_without_overlap, block_height_without_overlap;
    int32_t in_block_overlap_width, in_block_overlap_height, in_block_width, in_block_height;
    int32_t first_stripe_block_x_num, mid_stripe_block_x_num, last_stripe_block_x_num;
    int32_t first_slice_block_y_num, last_slice_block_y_num;
	int32_t stripe_idx, block_idx, bound_idx;
    int32_t half_overlap_slice_h;
    int32_t exist_last_slice;
	
	if ((p_iobuf_info == NULL) || (p_reg_buf == NULL) || (p_blks_lhs == NULL)) {
		return -1;
	}

    exist_last_slice = (p_reg_buf->last_slice_h == 0) ? 0 : 1;
	
    in_block_width = p_iobuf_info->blockInfo.block_width;
    in_block_height = p_iobuf_info->blockInfo.block_height;
    in_block_overlap_width =  p_iobuf_info->blockInfo.block_overlap_width;
    in_block_overlap_height = p_iobuf_info->blockInfo.block_overlap_height;
    block_width_without_overlap = in_block_width - in_block_overlap_width;
    block_height_without_overlap = in_block_height - in_block_overlap_height;
    first_stripe_block_x_num = ALIGN_CEIL_DIV(p_reg_buf->first_stripe_w, block_width_without_overlap);
    mid_stripe_block_x_num = ALIGN_CEIL_DIV(p_reg_buf->mid_stripe_w, block_width_without_overlap);
    last_stripe_block_x_num = ALIGN_CEIL_DIV(p_reg_buf->last_stripe_w, block_width_without_overlap);

    half_overlap_slice_h = p_iobuf_info->stripeInfo.slice_overlap_height >> 1;
    first_slice_block_y_num = ALIGN_CEIL_DIV(p_reg_buf->first_slice_h - half_overlap_slice_h, block_height_without_overlap);
    //mid_slice_block_y_num = ALIGN_CEIL_DIV(mid_slice_height - p_iobuf_info->stripeInfo.slice_overlap_height, block_height_without_overlap);
    last_slice_block_y_num = ALIGN_CEIL_DIV(p_reg_buf->last_slice_h - half_overlap_slice_h, block_height_without_overlap);


    for (stripe_idx = 0; stripe_idx < e_STRIPECASE_NUM; ++stripe_idx) {
        const JMISP_INSTR_STRIPE_CASE_TYPE stripe_case = (JMISP_INSTR_STRIPE_CASE_TYPE)stripe_idx;
        for (block_idx = 0; block_idx < e_BLOCKCASE_NUM; ++block_idx) {
            const JMISP_INSTR_BLOCK_CASE_TYPE block_case = (JMISP_INSTR_BLOCK_CASE_TYPE)block_idx;
            JMISP_INSTR_BLOCK_INFO* p_out_blk = &p_blks_lhs->blocks[stripe_case][block_case];
			
			//init
			p_out_blk->width = 0;
			p_out_blk->height = 0;
			for (bound_idx=0; bound_idx<e_BOUND_NUM; ++bound_idx) {
				p_out_blk->pad[bound_idx] = 0;
				p_out_blk->crop[bound_idx] = 0;
			}

            p_out_blk->width = in_block_width;
            if ((block_case == e_BLOCKCASE_LEFT_TOP) || (block_case == e_BLOCKCASE_LEFT) || (block_case == e_BLOCKCASE_BOTTOM_LEFT)) {
                p_out_blk->crop[e_BOUND_LEFT] = (in_block_overlap_width >> 1);
                p_out_blk->crop[e_BOUND_RIGHT] = (in_block_overlap_width >> 1);
#if REF_OUT_STRIPE_CROP_WITH_PPU
                if (is_frame_out_buf(p_iobuf_info, iobuf_info_type)
                && ((stripe_case == e_STRIPECASE_MID) || (stripe_case == e_STRIPECASE_RIGHT))) {
                    p_out_blk->crop[e_BOUND_LEFT] += (p_iobuf_info->stripeInfo.stripe_overlap_width >> 1);
                }
#endif
            }
            else if ((block_case == e_BLOCKCASE_TOP_RIGHT) || (block_case == e_BLOCKCASE_RIGHT) || (block_case == e_BLOCKCASE_RIGHT_BOTTOM)) {
                int32_t last_out_block_width, last_block_width_with_left_pad;
                if (stripe_case == e_STRIPECASE_LEFT) {
                    last_out_block_width = p_reg_buf->first_stripe_w - first_stripe_block_x_num * block_width_without_overlap;
                }
                else if (stripe_case == e_STRIPECASE_RIGHT) {
                    last_out_block_width = p_reg_buf->last_stripe_w - last_stripe_block_x_num * block_width_without_overlap;
                }
                else {
                    last_out_block_width = p_reg_buf->mid_stripe_w - mid_stripe_block_x_num * block_width_without_overlap;
                }

                if (last_out_block_width == 0) {
                    last_block_width_with_left_pad = block_width_without_overlap + (in_block_overlap_width >> 1);
                }
                else if (last_out_block_width > 0) {
                    last_block_width_with_left_pad = last_out_block_width + (in_block_overlap_width >> 1);
                }
                else if (last_out_block_width < 0) {
                    last_block_width_with_left_pad = block_width_without_overlap + last_out_block_width + (in_block_overlap_width >> 1);
                }
                p_out_blk->crop[e_BOUND_LEFT] = (in_block_overlap_width >> 1);
                p_out_blk->crop[e_BOUND_RIGHT] = block_width_without_overlap + in_block_overlap_width - last_block_width_with_left_pad;
#if REF_OUT_STRIPE_CROP_WITH_PPU
                if (is_frame_out_buf(p_iobuf_info, iobuf_info_type)
                && ((stripe_case == e_STRIPECASE_LEFT) || (stripe_case == e_STRIPECASE_MID))) {
                    p_out_blk->crop[e_BOUND_RIGHT] += (p_iobuf_info->stripeInfo.stripe_overlap_width >> 1);
                }
#endif
            }
            else {
                p_out_blk->crop[e_BOUND_LEFT] = (in_block_overlap_width >> 1);
                p_out_blk->crop[e_BOUND_RIGHT] = (in_block_overlap_width >> 1);
            }

            if ((p_out_blk->crop[e_BOUND_LEFT] + p_out_blk->crop[e_BOUND_RIGHT]) >= in_block_width) {
                ASSERT_MSG(0, "JMISP block crop size > block width\n");
            }
            if ((p_out_blk->crop[e_BOUND_TOP] + p_out_blk->crop[e_BOUND_BOTTOM]) >= in_block_height) {
                ASSERT_MSG(0, "JMISP block crop size > block height\n");
            }


            p_out_blk->height = in_block_height;
            if (((block_case == e_BLOCKCASE_LEFT_TOP) || (block_case == e_BLOCKCASE_TOP) || (block_case == e_BLOCKCASE_TOP_RIGHT))) {
                int32_t first_out_block_height, first_block_height_with_bottom_pad;
                if (exist_last_slice) {
                    first_out_block_height = p_reg_buf->first_slice_h - half_overlap_slice_h - first_slice_block_y_num * block_height_without_overlap;
                } else {
                    first_out_block_height = p_reg_buf->first_slice_h - first_slice_block_y_num * block_height_without_overlap;
                }

                if (first_out_block_height == 0) {
                    first_block_height_with_bottom_pad = block_height_without_overlap + (in_block_overlap_height >> 1);
                }
                else if (first_out_block_height > 0) {
                    first_block_height_with_bottom_pad = first_out_block_height + (in_block_overlap_height >> 1);
                }
                else if (first_out_block_height < 0) {
                    first_block_height_with_bottom_pad = (block_height_without_overlap + first_out_block_height) + (in_block_overlap_height >> 1);
                }
                p_out_blk->crop[e_BOUND_TOP] = block_height_without_overlap + in_block_overlap_height - first_block_height_with_bottom_pad;
                p_out_blk->crop[e_BOUND_BOTTOM] = (in_block_overlap_height >> 1);
            }
            else if ( ((block_case == e_BLOCKCASE_BOTTOM_LEFT) || (block_case == e_BLOCKCASE_BOTTOM) || (block_case == e_BLOCKCASE_RIGHT_BOTTOM)) ) {
                int32_t last_out_block_height, last_block_height_with_top_pad;
                if (exist_last_slice) {
                    last_out_block_height = p_reg_buf->last_slice_h - half_overlap_slice_h - last_slice_block_y_num * block_height_without_overlap;
                } else {
                    last_out_block_height = block_height_without_overlap;
                }

                if (last_out_block_height == 0) {
                    last_block_height_with_top_pad = block_height_without_overlap + (in_block_overlap_height >> 1);
                }
                else if (last_out_block_height > 0) {
                    last_block_height_with_top_pad = last_out_block_height + (in_block_overlap_height >> 1);
                }
                else if (last_out_block_height < 0) {
                    last_block_height_with_top_pad = (block_height_without_overlap + last_out_block_height) + (in_block_overlap_height >> 1);
                }
                p_out_blk->crop[e_BOUND_TOP] = (in_block_overlap_height >> 1);
                p_out_blk->crop[e_BOUND_BOTTOM] = block_height_without_overlap + in_block_overlap_height - last_block_height_with_top_pad;
            }
            else {
                p_out_blk->crop[e_BOUND_TOP] = (in_block_overlap_height >> 1);
                p_out_blk->crop[e_BOUND_BOTTOM] = (in_block_overlap_height >> 1);
            }

        }
    }

    return 0;
}


INT32 gen_jmisp_reg_para(AI_JMISP_INFO* p_jmisp_info, JMISP_REG_INFO* p_reg_info) {
    int32_t idx;
    int32_t exist_mid_slice, exist_last_slice, exist_last_stripe;

    if ((p_jmisp_info == NULL) || (p_reg_info == NULL)) {
        ASSERT_MSG(0, "Null jmisp info, please check\n");
        return -1;
    }
    memset(p_reg_info, 0, sizeof(JMISP_REG_INFO));

    p_reg_info->stripe_x_num = gen_jmisp_stripe_num(p_jmisp_info);
    p_reg_info->slice_y_num = gen_jmisp_slice_num(p_jmisp_info);
    exist_mid_slice = (p_reg_info->slice_y_num < 3) ? 0 : 1;
    exist_last_slice = (p_reg_info->slice_y_num < 2) ? 0 : 1;
    exist_last_stripe = (p_reg_info->stripe_x_num < 2) ? 0 : 1;

    for (idx=0; idx<e_AI_IOBUF_NUM; ++idx) {
        const e_AI_JMISP_IOBUF_CONST iobuf_info_type = (e_AI_JMISP_IOBUF_CONST)idx;
        AI_JMISP_IOBUF_INFO* p_iobuf_info = (AI_JMISP_IOBUF_INFO*)&p_jmisp_info->iobuf[iobuf_info_type];
        const e_AI_JMISP_REG_IO_CONST reg_iobuf_type = convert_iobuf2regbuf_type(p_iobuf_info, iobuf_info_type);
        JMISP_REG_IOBUF* p_reg_buf;
        const int32_t block_width_without_overlap = p_iobuf_info->blockInfo.block_width - p_iobuf_info->blockInfo.block_overlap_width;
        const int32_t block_height_without_overlap = p_iobuf_info->blockInfo.block_height - p_iobuf_info->blockInfo.block_overlap_height;
        const int32_t bitdepth = p_iobuf_info->bitdepth;
        const int32_t half_overlap_slice_h = p_iobuf_info->stripeInfo.slice_overlap_height >> 1;
        int32_t tmp_last_slice_h, first_slice_block_y_num, tmp_y_ofs;

        if (reg_iobuf_type < 0) {
            continue;
        } else if ((!is_jmisp_sup_buf(p_iobuf_info, iobuf_info_type)) || (p_iobuf_info->buffer_sz <= 0)) {
            continue;
        }

        p_reg_buf = &p_reg_info->iobuf_reg[reg_iobuf_type];
        p_reg_buf->first_stripe_w = p_iobuf_info->stripeInfo.first_stripe_width;
        p_reg_buf->mid_stripe_w = p_iobuf_info->stripeInfo.mid_stripe_width;
        p_reg_buf->last_stripe_w = p_iobuf_info->stripeInfo.last_stripe_width;
        p_reg_buf->first_slice_h = p_iobuf_info->stripeInfo.output_slice_height + half_overlap_slice_h;
        if (exist_mid_slice) {
            p_reg_buf->mid_slice_h = p_iobuf_info->stripeInfo.output_slice_height + p_iobuf_info->stripeInfo.slice_overlap_height;
        } else {
            p_reg_buf->mid_slice_h = 0;
        }
        tmp_last_slice_h = (p_iobuf_info->frameInfo.frame_height - p_reg_info->slice_y_num * p_iobuf_info->stripeInfo.output_slice_height);
        if (!exist_last_slice) {
            p_reg_buf->last_slice_h = 0;
        }
        else if (tmp_last_slice_h == 0) {
            p_reg_buf->last_slice_h = p_iobuf_info->stripeInfo.output_slice_height + half_overlap_slice_h;
        }
        else if (tmp_last_slice_h > 0) {
            p_reg_buf->last_slice_h = tmp_last_slice_h + half_overlap_slice_h;
        }
        else if (tmp_last_slice_h < 0) {
            p_reg_buf->last_slice_h = p_iobuf_info->stripeInfo.output_slice_height + tmp_last_slice_h + half_overlap_slice_h;
        }

        
        if (is_ring_buf(p_iobuf_info, iobuf_info_type)) {
            p_reg_buf->is_from_ringbuf = TRUE;
        } else {
            p_reg_buf->is_from_ringbuf = FALSE;
        }

        if (is_frame_buf(p_iobuf_info, iobuf_info_type))  {
            p_reg_buf->block_lofs = p_iobuf_info->frameInfo.frame_width;
        } else {
            p_reg_buf->block_lofs = max(max(p_iobuf_info->stripeInfo.first_stripe_width, p_iobuf_info->stripeInfo.mid_stripe_width), p_iobuf_info->stripeInfo.last_stripe_width);
        }
        
        if (p_reg_buf->is_from_ringbuf) {
            p_reg_buf->first_stripe_x_ofs = 0;
            p_reg_buf->mid_stripe_x_ofs = 0;
        }
#if REF_OUT_STRIPE_CROP_WITH_PPU
        else if (is_frame_out_buf(p_iobuf_info, iobuf_info_type)) {
            p_reg_buf->first_stripe_x_ofs = p_iobuf_info->stripeInfo.first_stripe_width - (p_iobuf_info->stripeInfo.stripe_overlap_width >> 1);
            p_reg_buf->mid_stripe_x_ofs = p_iobuf_info->stripeInfo.mid_stripe_width - p_iobuf_info->stripeInfo.stripe_overlap_width;
        }
#endif
        else {
            p_reg_buf->first_stripe_x_ofs = p_iobuf_info->stripeInfo.first_stripe_width - p_iobuf_info->stripeInfo.stripe_overlap_width;
            p_reg_buf->mid_stripe_x_ofs = p_iobuf_info->stripeInfo.mid_stripe_width - p_iobuf_info->stripeInfo.stripe_overlap_width;
        }


        if (is_out_buf(p_iobuf_info, iobuf_info_type)) {

            p_reg_buf->first_slice_y_ofs = (p_reg_buf->first_slice_h - half_overlap_slice_h) * p_reg_buf->block_lofs;
            p_reg_buf->mid_slice_y_ofs = (p_reg_buf->mid_slice_h - p_iobuf_info->stripeInfo.slice_overlap_height) * p_reg_buf->block_lofs;

            p_reg_buf->first_block_x_ofs = block_width_without_overlap;
            p_reg_buf->mid_block_x_ofs = block_width_without_overlap;

            if (block_height_without_overlap > 0) {
                first_slice_block_y_num = ALIGN_CEIL_DIV(p_reg_buf->first_slice_h - half_overlap_slice_h, block_height_without_overlap);
                tmp_y_ofs = (p_reg_buf->first_slice_h - half_overlap_slice_h) - (block_height_without_overlap * (first_slice_block_y_num - 1));
                p_reg_buf->first_block_y_ofs = tmp_y_ofs * p_reg_buf->block_lofs;
            } else {
                p_reg_buf->first_block_y_ofs = block_height_without_overlap * p_reg_buf->block_lofs;
            }
            p_reg_buf->mid_block_y_ofs = block_height_without_overlap * p_reg_buf->block_lofs;

        } else {

            const int32_t half_overlap_blk_h = p_iobuf_info->blockInfo.block_overlap_height >> 1;
            p_reg_buf->first_slice_y_ofs = (p_reg_buf->first_slice_h - p_iobuf_info->stripeInfo.slice_overlap_height) * p_reg_buf->block_lofs;
            p_reg_buf->mid_slice_y_ofs = (p_reg_buf->mid_slice_h - p_iobuf_info->stripeInfo.slice_overlap_height) * p_reg_buf->block_lofs;

            p_reg_buf->first_block_x_ofs = block_width_without_overlap - (p_iobuf_info->blockInfo.block_overlap_width >> 1);
            p_reg_buf->mid_block_x_ofs = block_width_without_overlap;

            if (block_height_without_overlap > 0) {
                first_slice_block_y_num = ALIGN_CEIL_DIV(p_reg_buf->first_slice_h - half_overlap_slice_h, block_height_without_overlap);
                tmp_y_ofs = (p_reg_buf->first_slice_h - half_overlap_slice_h) - (block_height_without_overlap * (first_slice_block_y_num - 1)) - half_overlap_blk_h;
                p_reg_buf->first_block_y_ofs = tmp_y_ofs * p_reg_buf->block_lofs;
            } else {
                p_reg_buf->first_block_y_ofs = (block_height_without_overlap - half_overlap_blk_h) * p_reg_buf->block_lofs;
            }
            p_reg_buf->mid_block_y_ofs = block_height_without_overlap * p_reg_buf->block_lofs;

        }

        if (!exist_last_stripe) {
            p_reg_buf->mid_stripe_x_ofs = 0;
        }
        if (!exist_last_slice) {
            p_reg_buf->mid_slice_y_ofs = 0;
        }


        // for bit depth > 8
        p_reg_buf->block_lofs = jmisp_align_pos_ofs(p_reg_buf->block_lofs, bitdepth);
        p_reg_buf->first_stripe_x_ofs = jmisp_align_pos_ofs(p_reg_buf->first_stripe_x_ofs, bitdepth);
        p_reg_buf->mid_stripe_x_ofs = jmisp_align_pos_ofs(p_reg_buf->mid_stripe_x_ofs, bitdepth);
        p_reg_buf->first_slice_y_ofs = jmisp_align_pos_ofs(p_reg_buf->first_slice_y_ofs, bitdepth);
        p_reg_buf->mid_slice_y_ofs = jmisp_align_pos_ofs(p_reg_buf->mid_slice_y_ofs, bitdepth);
        p_reg_buf->first_block_x_ofs = jmisp_align_pos_ofs(p_reg_buf->first_block_x_ofs, bitdepth);
        p_reg_buf->mid_block_x_ofs = jmisp_align_pos_ofs(p_reg_buf->mid_block_x_ofs, bitdepth);
        p_reg_buf->first_block_y_ofs = jmisp_align_pos_ofs(p_reg_buf->first_block_y_ofs, bitdepth);
        p_reg_buf->mid_block_y_ofs = jmisp_align_pos_ofs(p_reg_buf->mid_block_y_ofs, bitdepth);
    }

    {
        const e_AI_JMISP_IOBUF_CONST iobuf_info_type = get_benchmark_iobuf_type();
        AI_JMISP_IOBUF_INFO* p_temp_iobuf = &p_jmisp_info->iobuf[iobuf_info_type];
        const e_AI_JMISP_REG_IO_CONST reg_iobuf_type = convert_iobuf2regbuf_type(p_temp_iobuf, iobuf_info_type);
        JMISP_REG_IOBUF* p_temp_reg_buf;
        const int32_t block_width_without_overlap = p_temp_iobuf->blockInfo.block_width - p_temp_iobuf->blockInfo.block_overlap_width;
        const int32_t block_height_without_overlap = p_temp_iobuf->blockInfo.block_height - p_temp_iobuf->blockInfo.block_overlap_height;
        if (reg_iobuf_type < 0) {
            ASSERT_MSG(0, "Unknown regbuf type in reg.\n");
            return -1;
        } else if (!is_jmisp_sup_buf(p_temp_iobuf, iobuf_info_type)) {
            ASSERT_MSG(0, "Unknown iobuf type in reg.\n");
            return -1;
        }
        p_temp_reg_buf = &p_reg_info->iobuf_reg[reg_iobuf_type];
        p_reg_info->first_stripe_block_x_num = ALIGN_CEIL_DIV(p_temp_reg_buf->first_stripe_w, block_width_without_overlap);
        p_reg_info->mid_stripe_block_x_num = ALIGN_CEIL_DIV(p_temp_reg_buf->mid_stripe_w, block_width_without_overlap);
        p_reg_info->last_stripe_block_x_num = ALIGN_CEIL_DIV(p_temp_reg_buf->last_stripe_w, block_width_without_overlap);
        p_reg_info->first_slice_block_y_num = ALIGN_CEIL_DIV(p_temp_reg_buf->first_slice_h - (p_temp_iobuf->stripeInfo.slice_overlap_height >> 1), block_height_without_overlap);
        if (exist_mid_slice) {
        p_reg_info->mid_slice_block_y_num = (p_temp_reg_buf->mid_slice_h - p_temp_iobuf->blockInfo.block_overlap_height) / (block_height_without_overlap);
        } else {
            p_reg_info->mid_slice_block_y_num = 0;
        }
        if (exist_last_slice) {
        p_reg_info->last_slice_block_y_num = ALIGN_CEIL_DIV(p_temp_reg_buf->last_slice_h - (p_temp_iobuf->stripeInfo.slice_overlap_height >> 1), block_height_without_overlap);
        } else {
            p_reg_info->last_slice_block_y_num = 0;
        }
                
        if ((exist_mid_slice) && ((p_temp_reg_buf->mid_slice_h - p_temp_iobuf->blockInfo.block_overlap_height) != (block_height_without_overlap * p_reg_info->mid_slice_block_y_num))) {
            ASSERT_MSG(0, "ERR: middle slice height - block overlap should be devided with block height\n");
        }
        if (exist_last_slice) {
            if ((p_temp_iobuf->stripeInfo.slice_overlap_height != p_temp_iobuf->blockInfo.block_overlap_height)
                        && (p_reg_info->slice_y_num > 1)) {
                ASSERT_MSG(0, "ERR: slice overlap hegiht should equal with block overlap height\n");
            } else if ((p_temp_iobuf->stripeInfo.slice_overlap_height % 4) != 0) {
                ASSERT_MSG(0, "ERR: slice overlap hegiht should be devided with 4\n");
            }
        } else if (p_temp_iobuf->stripeInfo.slice_overlap_height != 0) {
            ASSERT_MSG(0, "ERR: slice overlap hegiht should be 0 if only 1 slice\n");
        }

        if (((p_temp_iobuf->frameInfo.frame_height - (p_temp_iobuf->stripeInfo.slice_overlap_height >> 1)) % (p_temp_iobuf->stripeInfo.output_slice_height) == 0)
            && (p_temp_iobuf->stripeInfo.slice_overlap_height > 0)) {
            p_reg_info->signal_mode = TRUE;
        }
        else {
            p_reg_info->signal_mode = FALSE;
        }
    }

    return 0;
}

INT32 gen_ppu_blockcase_info(AI_JMISP_INFO* p_jmisp_info, JMISP_REG_INFO* p_reg_info, JMISPPPU_HEAD* p_ppu_head, JMISP_INSTR_BLOCK_CASE_INFO* p_ppu_blkcase_info) {
    e_AI_JMISP_IOBUF_CONST iobuf_info_type;
    e_AI_JMISP_REG_IO_CONST reg_iobuf_type;
    AI_JMISP_IOBUF_INFO* p_iobuf_info;
    JMISP_REG_IOBUF* p_reg_buf;

    if ((p_jmisp_info == NULL) || (p_ppu_head == NULL) || (p_ppu_blkcase_info == NULL)) {
        return -1;
    }
    memset(p_ppu_blkcase_info, 0, sizeof(JMISP_INSTR_BLOCK_CASE_INFO));

    iobuf_info_type = (e_AI_JMISP_IOBUF_CONST)p_ppu_head->match_iobuf_ID[0];
    p_iobuf_info = &p_jmisp_info->iobuf[iobuf_info_type];
    reg_iobuf_type = convert_iobuf2regbuf_type(p_iobuf_info, iobuf_info_type);;
    if (reg_iobuf_type < 0) {
        ASSERT_MSG(0, "Unknown regbuf type in reg.\n");
        return -1;
    } else if (!is_jmisp_sup_buf(p_iobuf_info, iobuf_info_type)) {
        ASSERT_MSG(0, "Unknown iobuf type in reg.\n");
        return -1;
    }
    p_reg_buf = &p_reg_info->iobuf_reg[reg_iobuf_type];

    if (is_in_buf(p_iobuf_info, iobuf_info_type)) {
        jmisp_gen_loop_instr_input_blocks(iobuf_info_type, p_iobuf_info, p_reg_buf, p_ppu_blkcase_info);
    } else if (is_out_buf(p_iobuf_info, iobuf_info_type)) {
        jmisp_gen_loop_instr_output_blocks(iobuf_info_type, p_iobuf_info, p_reg_buf, p_ppu_blkcase_info);
    }

#if 0
    {
        uint32_t x=0, y=0;
        DBG_DUMP("jmisp iobuf type = %d\n", iobuf_info_type);
        for (y=0; y<e_STRIPECASE_NUM; ++y) {
            for (x=0; x<e_BLOCKCASE_NUM; ++x) {
                JMISP_INSTR_BLOCK_INFO* p_info = &p_ppu_blkcase_info->blocks[y][x];
                DBG_DUMP("[%d][%d]; w=%d; h=%d; p0=%d; p1=%d; p2=%d; p3=%d; c0=%d; c1=%d; c2=%d; c3=%d;\r\n"
                , y, x, p_info->width, p_info->height
                , p_info->pad[0], p_info->pad[1], p_info->pad[2], p_info->pad[3]
                , p_info->crop[0], p_info->crop[1], p_info->crop[2], p_info->crop[3]);
            }
        }
    }
#endif

    return 0;
}

uint32_t get_cmd_mode(uint64_t cmd) {
    return (cmd & 0xf);
}

uint32_t get_upd_cmd_regofs(uint64_t cmd) {
    return ((cmd >> 8) & 0xfff);
}

uint32_t get_upd_cmd_byte_en(uint64_t cmd) {
    return ((cmd >> 4) & 0xf);
}

uint32_t get_upd_cmd_value(uint64_t cmd) {
    return (cmd >> 32);
}

bool is_bayer_from_upd_value(uint32_t value) {
    const uint32_t in_fmt = (value & 0xf);
    if ((in_fmt == 2) || (in_fmt == 3)) {
        return true;
    } else {
        return false;
    }
}

bool is_nr_mode_from_upd_value(uint32_t value) {
    const uint32_t path_sel = (value & 0xf);
    if (path_sel == 0) {
        return false;
    } else {
        return true;
    }
}


int32_t upd_ppu_blockcase_instr(JMISP_INSTR_BLOCK_INFO* p_blk_case, JMISP_REG_INFO* p_reg_info, JMISPPPU_HEAD* p_ppu_head, uintptr_t cmd_addr, uint32_t cmd_lines, AI_JMISP_INFO* p_jmisp_info) {
    const uint64_t instr_mask = g_instr_mask;
    uint32_t crop_w, crop_h;
    uint32_t extra_pad_r = 0;
    uint64_t* p_cmd;
    bool is_nr = false;
    if ((p_blk_case == NULL) || (p_reg_info == NULL) || (p_ppu_head == NULL) || (cmd_addr == 0) || (p_jmisp_info == NULL)) {
        return -1;
    }

    p_cmd = (uint64_t*)cmd_addr;
    {
        const e_AI_JMISP_IOBUF_CONST iobuf_info_type = (e_AI_JMISP_IOBUF_CONST)p_ppu_head->match_iobuf_ID[0];
        AI_JMISP_IOBUF_INFO* p_info = &p_jmisp_info->iobuf[iobuf_info_type];
        const uint32_t ch_addr_ofs = p_info->frameInfo.frame_height * ((p_info->frameInfo.frame_width * p_info->bitdepth + 7) >> 3);
        uint32_t upd_ch_addr_cnt = 0, ch_addr=0;
        bool is_bayer = false;
        int32_t idx=0;

        for (idx=0; idx<cmd_lines; ++idx) {
            const uint32_t mode = get_cmd_mode(p_cmd[idx]);
            const uint32_t reg_ofs = get_upd_cmd_regofs(p_cmd[idx]);
            const uint32_t is_upd = get_upd_cmd_byte_en(p_cmd[idx]);
            const uint32_t value = get_upd_cmd_value(p_cmd[idx]);
            if (mode == NULL_MODE) {
                break;
            } else if ((mode == NEXTLL_MODE) && (p_info->channel > 1)) {
                ++upd_ch_addr_cnt;
            }
            if ((mode != UPD_MODE) || (0 == is_upd)) {
                continue;
            }

            if (upd_ch_addr_cnt > 0) {
                ch_addr += ch_addr_ofs;
                if (((idx == PPU_SAI0_REG_OFS) && is_in_buf(p_info, iobuf_info_type))
                    || ((idx == PPU_SAO0_REG_OFS) && is_out_buf(p_info, iobuf_info_type))) {
                    p_cmd[idx] = (p_cmd[idx] & instr_mask) + ((uint64_t)ch_addr << (uint64_t)32);
                }
            } else if (reg_ofs == PPU_CONTROL_REG_OFS) {
                is_bayer = is_bayer_from_upd_value(value);
            } else if (reg_ofs == PPU_NR_PRE_REG_OFS) {
                is_nr = is_nr_mode_from_upd_value(value);
            }
        }

        crop_w = p_blk_case->width - p_blk_case->crop[e_BOUND_LEFT] - p_blk_case->crop[e_BOUND_RIGHT];
        crop_h = p_blk_case->height - p_blk_case->crop[e_BOUND_TOP] - p_blk_case->crop[e_BOUND_BOTTOM];
        if (((p_blk_case->pad[e_BOUND_TOP] > 0) || (p_blk_case->pad[e_BOUND_BOTTOM] > 0))
            && (p_blk_case->pad[e_BOUND_RIGHT] < 4) && (!is_bayer) && (!is_nr)) {
            extra_pad_r =  4 - p_blk_case->pad[e_BOUND_RIGHT];
        } else {
            extra_pad_r = 0;
        }

        if ((crop_w == 0) || (crop_h == 0)) {
            ASSERT_MSG(0, "ppu out w or h = 0 after crop.\n");
        }
    }

    {
        e_AI_JMISP_IOBUF_CONST iobuf_info_type;
        e_AI_JMISP_REG_IO_CONST reg_iobuf_type;
        AI_JMISP_IOBUF_INFO* p_iobuf_info;
        JMISP_REG_IOBUF* p_reg_buf;
        int32_t idx=0;

        for (idx=0; idx<cmd_lines; ++idx) {
            const uint32_t mode = get_cmd_mode(p_cmd[idx]);
            const uint32_t reg_ofs = get_upd_cmd_regofs(p_cmd[idx]);
            const uint32_t is_upd = get_upd_cmd_byte_en(p_cmd[idx]);
            if ((mode == NULL_MODE) || (mode == NEXTLL_MODE)) {
                break;
            } else if ((mode != UPD_MODE) || (0 == is_upd)) {
                continue;
            }

            switch (reg_ofs) {
            case PPU_WIDTH_REG_OFS:
                iobuf_info_type = (e_AI_JMISP_IOBUF_CONST)p_ppu_head->match_iobuf_ID[0];
                p_iobuf_info = &p_jmisp_info->iobuf[iobuf_info_type];
                if (is_in_buf(p_iobuf_info, iobuf_info_type)) {
                    p_cmd[idx] = (p_cmd[idx] & instr_mask) + ((uint64_t)p_blk_case->width << (uint64_t)32);
                }
                break;
            case PPU_HEIGHT_REG_OFS:
                iobuf_info_type = (e_AI_JMISP_IOBUF_CONST)p_ppu_head->match_iobuf_ID[0];
                p_iobuf_info = &p_jmisp_info->iobuf[iobuf_info_type];
                if (is_in_buf(p_iobuf_info, iobuf_info_type)) {
                    p_cmd[idx] = (p_cmd[idx] & instr_mask) + ((uint64_t)p_blk_case->height << (uint64_t)32);
                }
                break;
            case PPU_PAD_0_REG_OFS:
                p_cmd[idx] = (p_cmd[idx] & instr_mask)
                                + ((uint64_t)(((p_blk_case->pad[e_BOUND_LEFT] & 0xffff) << 0)
                                                + (((p_blk_case->pad[e_BOUND_RIGHT] + extra_pad_r) & 0xffff) << 16)) << (uint64_t)32);
                break;
            case PPU_PAD_1_REG_OFS:
                p_cmd[idx] = (p_cmd[idx] & instr_mask)
                                + ((uint64_t)(((p_blk_case->pad[e_BOUND_TOP] & 0xffff) << 0)
                                                + ((p_blk_case->pad[e_BOUND_BOTTOM] & 0xffff) << 16)) << (uint64_t)32);
                break;
            case PPU_CROP_0_REG_OFS:
                p_cmd[idx] = (p_cmd[idx] & instr_mask)
                                + ((uint64_t)(((p_blk_case->crop[e_BOUND_LEFT] & 0xffff) << 0)
                                                + ((p_blk_case->crop[e_BOUND_TOP] & 0xffff) << 16)) << (uint64_t)32);
                break;
            case PPU_CROP_1_REG_OFS:
                p_cmd[idx] = (p_cmd[idx] & instr_mask)
                                + ((uint64_t)(((crop_w & 0xffff) << 0)
                                                + ((crop_h & 0xffff) << 16)) << (uint64_t)32);
                break;
            case PPU_LOFS_EN_REG_OFS:
                p_cmd[idx] = p_cmd[idx] & instr_mask; //init
                iobuf_info_type = (e_AI_JMISP_IOBUF_CONST)p_ppu_head->match_iobuf_ID[0];
                p_iobuf_info = &p_jmisp_info->iobuf[iobuf_info_type];
                if (is_in_buf(p_iobuf_info, iobuf_info_type)) {
                    p_cmd[idx] = p_cmd[idx] | ((uint64_t)0x1 << 32);
                    if (extra_pad_r > 0) {
                        p_cmd[idx] = p_cmd[idx] | ((uint64_t)0x10 << 32);
                    } else if (is_nr) {
                        p_cmd[idx] = p_cmd[idx] | ((uint64_t)0x1010 << 32);
                    }
                } else if (is_out_buf(p_iobuf_info, iobuf_info_type)) {
                    p_cmd[idx] = p_cmd[idx] | ((uint64_t)0x10 << 32);
                }

                iobuf_info_type = (e_AI_JMISP_IOBUF_CONST)p_ppu_head->match_iobuf_ID[1];
                p_iobuf_info = &p_jmisp_info->iobuf[iobuf_info_type];
                if (is_in_buf(p_iobuf_info, iobuf_info_type)) {
                    p_cmd[idx] = p_cmd[idx] | ((uint64_t)0x2 << 32);
                    if (extra_pad_r > 0) {
                        p_cmd[idx] = p_cmd[idx] | ((uint64_t)0x20 << 32);
                    }
                } else if (is_out_buf(p_iobuf_info, iobuf_info_type)) {
                    p_cmd[idx] = p_cmd[idx] | ((uint64_t)0x20 << 32);
                }
                break;
            case PPU_IN0_LOFS_REG_OFS:
                iobuf_info_type = (e_AI_JMISP_IOBUF_CONST)p_ppu_head->match_iobuf_ID[0];
                p_iobuf_info = &p_jmisp_info->iobuf[iobuf_info_type];
                if (is_in_buf(p_iobuf_info, iobuf_info_type)) {
                    reg_iobuf_type = convert_iobuf2regbuf_type(p_iobuf_info, iobuf_info_type);
                    if (reg_iobuf_type >= 0) {
                        uint64_t block_lofs;
                        p_reg_buf = &p_reg_info->iobuf_reg[reg_iobuf_type];
                        block_lofs = p_reg_buf->block_lofs;
                        p_cmd[idx] = (p_cmd[idx] & instr_mask) + (block_lofs << (uint64_t)32);
                    }
                }
                break;
            case PPU_OUT0_LOFS_REG_OFS:
                iobuf_info_type = (e_AI_JMISP_IOBUF_CONST)p_ppu_head->match_iobuf_ID[0];
                p_iobuf_info = &p_jmisp_info->iobuf[iobuf_info_type];
                if ((is_in_buf(p_iobuf_info, iobuf_info_type)) && (extra_pad_r > 0)) {
                    int32_t crop_pad_lofs = p_jmisp_info->iobuf[iobuf_info_type].blockInfo.block_width * p_jmisp_info->iobuf[iobuf_info_type].bitdepth >> 3;
                    p_cmd[idx] = (p_cmd[idx] & instr_mask) + ((uint64_t)crop_pad_lofs << (uint64_t)32);
                } else if (is_out_buf(p_iobuf_info, iobuf_info_type)) {
                    reg_iobuf_type = convert_iobuf2regbuf_type(p_iobuf_info, iobuf_info_type);
                    if (reg_iobuf_type >= 0) {
                        uint64_t block_lofs;
                        p_reg_buf = &p_reg_info->iobuf_reg[reg_iobuf_type];
                        block_lofs = p_reg_buf->block_lofs;
                        p_cmd[idx] = (p_cmd[idx] & instr_mask) + (block_lofs << (uint64_t)32);
                    }
                }
                break;
            case PPU_IN1_LOFS_REG_OFS:
                iobuf_info_type = (e_AI_JMISP_IOBUF_CONST)p_ppu_head->match_iobuf_ID[1];
                p_iobuf_info = &p_jmisp_info->iobuf[iobuf_info_type];
                if (is_in_buf(p_iobuf_info, iobuf_info_type)) {
                    reg_iobuf_type = convert_iobuf2regbuf_type(p_iobuf_info, iobuf_info_type);
                    if (reg_iobuf_type >= 0) {
                        uint64_t block_lofs;
                        p_reg_buf = &p_reg_info->iobuf_reg[reg_iobuf_type];
                        block_lofs = p_reg_buf->block_lofs;
                        p_cmd[idx] = (p_cmd[idx] & instr_mask) + (block_lofs << (uint64_t)32);
                    }
                }
                break;
            case PPU_OUT1_LOFS_REG_OFS:
                iobuf_info_type = (e_AI_JMISP_IOBUF_CONST)p_ppu_head->match_iobuf_ID[1];
                p_iobuf_info = &p_jmisp_info->iobuf[iobuf_info_type];
                if ((is_in_buf(p_iobuf_info, iobuf_info_type)) && (extra_pad_r > 0)) {
                    int32_t crop_pad_lofs = p_jmisp_info->iobuf[iobuf_info_type].blockInfo.block_width * p_jmisp_info->iobuf[iobuf_info_type].bitdepth >> 3;
                    p_cmd[idx] = (p_cmd[idx] & instr_mask) + ((uint64_t)crop_pad_lofs << (uint64_t)32);
                } else if (is_out_buf(p_iobuf_info, iobuf_info_type)) {
                    reg_iobuf_type = convert_iobuf2regbuf_type(p_iobuf_info, iobuf_info_type);
                    if (reg_iobuf_type >= 0) {
                        uint64_t block_lofs;
                        p_reg_buf = &p_reg_info->iobuf_reg[reg_iobuf_type];
                        block_lofs = p_reg_buf->block_lofs;
                        p_cmd[idx] = (p_cmd[idx] & instr_mask) + (block_lofs << (uint64_t)32);
                    }
                }
                break;
            default:
                break;
            }
        }
    }
    return 0;
}

int32_t upd_pou_blockcase_instr(JMISP_INSTR_BLOCK_INFO* p_blk_case, JMISP_REG_INFO* p_reg_info, JMISPPPU_HEAD* p_ppu_head, uintptr_t cmd_addr, uint32_t cmd_lines, AI_JMISP_INFO* p_jmisp_info) {
    const uint64_t instr_mask = g_instr_mask;
    uint32_t crop_w, crop_h;
    uint64_t* p_cmd;
    if ((p_blk_case == NULL) || (p_reg_info == NULL) || (p_ppu_head == NULL) || (cmd_addr == 0) || (p_jmisp_info == NULL)) {
        return -1;
    }

    p_cmd = (uint64_t*)cmd_addr;
    crop_w = p_blk_case->width - p_blk_case->crop[e_BOUND_LEFT] - p_blk_case->crop[e_BOUND_RIGHT];
    crop_h = p_blk_case->height - p_blk_case->crop[e_BOUND_TOP] - p_blk_case->crop[e_BOUND_BOTTOM];
    if ((crop_w == 0) || (crop_h == 0)) {
        ASSERT_MSG(0, "ppu out w or h = 0 after crop.\n");
    }

    {
        const e_AI_JMISP_IOBUF_CONST iobuf_info_type = (e_AI_JMISP_IOBUF_CONST)p_ppu_head->match_iobuf_ID[0];
        const e_AI_JMISP_REG_IO_CONST reg_iobuf_type = convert_iobuf2regbuf_type(&p_jmisp_info->iobuf[iobuf_info_type], iobuf_info_type);
        const JMISP_REG_IOBUF* p_reg_iobuf;
        int32_t idx=0;

        if (reg_iobuf_type < 0) {
            ASSERT_MSG(0, "Unknown regbuf type in reg.\n");
            return -1;
        } else if (!is_jmisp_sup_buf(&p_jmisp_info->iobuf[iobuf_info_type], iobuf_info_type)) {
            ASSERT_MSG(0, "Unknown iobuf type in reg.\n");
            return -1;
        }
        p_reg_iobuf = &p_reg_info->iobuf_reg[reg_iobuf_type];

        for (idx=0; idx<cmd_lines; ++idx) {
            const uint32_t mode = get_cmd_mode(p_cmd[idx]);
            const uint32_t reg_ofs = get_upd_cmd_regofs(p_cmd[idx]);
            const uint32_t is_upd = get_upd_cmd_byte_en(p_cmd[idx]);
            if (mode == NULL_MODE) {
                break;
            }
            if ((mode != UPD_MODE) || (0 == is_upd)) {
                continue;
            }

            switch (reg_ofs) {
            case POU_CROP_0_REG_OFS:
                p_cmd[idx] = (p_cmd[idx] & instr_mask)
                                + ((uint64_t)(((p_blk_case->crop[e_BOUND_LEFT] & 0xffff) << 0)
                                                + ((p_blk_case->crop[e_BOUND_TOP] & 0xffff) << 16)) << (uint64_t)32);
                break;
            case POU_CROP_1_REG_OFS:
                p_cmd[idx] = (p_cmd[idx] & instr_mask)
                                + ((uint64_t)(((crop_w & 0xffff) << 0)
                                                + ((crop_h & 0xffff) << 16)) << (uint64_t)32);
                break;
            case POU_LOFS_EN_REG_OFS:
                p_cmd[idx] = p_cmd[idx] & instr_mask; //init
                // enable in lofs 0, 1, 2
                p_cmd[idx] = p_cmd[idx] | ((uint64_t)0x707 << 32);
                // enable out lofs 0
                p_cmd[idx] = p_cmd[idx] | ((uint64_t)0x10 << 32);
                break;
            case POU_OUT0_LOFS_REG_OFS:
                p_cmd[idx] = (p_cmd[idx] & instr_mask) + ((uint64_t)(p_reg_iobuf->block_lofs) << (uint64_t)32);
                break;
            default:
                break;
            }
        }
    }
    return 0;
}

int32_t upd_pou_tuning_instr(JMISP_INSTR_TUNING_INFO* p_tuning, JMISP_REG_INFO* p_reg_info, JMISPPPU_HEAD* p_ppu_head, uintptr_t cmd_addr, uint32_t cmd_lines, AI_JMISP_INFO* p_jmisp_info) {
    const uint64_t instr_mask = g_instr_mask;
    uint64_t* p_cmd;
    if ((p_tuning == NULL) || (p_reg_info == NULL) || (p_ppu_head == NULL) || (cmd_addr == 0) || (p_jmisp_info == NULL)) {
        return -1;
    }

    p_cmd = (uint64_t*)cmd_addr;
    {
        const e_AI_JMISP_IOBUF_CONST iobuf_info_type = (e_AI_JMISP_IOBUF_CONST)p_ppu_head->match_iobuf_ID[0];
        const e_AI_JMISP_REG_IO_CONST reg_iobuf_type = convert_iobuf2regbuf_type(&p_jmisp_info->iobuf[iobuf_info_type], iobuf_info_type);
        const JMISP_REG_IOBUF* p_reg_iobuf;
        int32_t idx=0;

        if (reg_iobuf_type < 0) {
            ASSERT_MSG(0, "Unknown regbuf type in reg.\n");
            return -1;
        } else if (!is_jmisp_sup_buf(&p_jmisp_info->iobuf[iobuf_info_type], iobuf_info_type)) {
            ASSERT_MSG(0, "Unknown iobuf type in reg.\n");
            return -1;
        }
        p_reg_iobuf = &p_reg_info->iobuf_reg[reg_iobuf_type];

        for (idx=0; idx<cmd_lines; ++idx) {
            const uint32_t mode = get_cmd_mode(p_cmd[idx]);
            const uint32_t reg_ofs = get_upd_cmd_regofs(p_cmd[idx]);
            const uint32_t is_upd = get_upd_cmd_byte_en(p_cmd[idx]);
            if (mode == NULL_MODE) {
                break;
            }
            if ((mode != UPD_MODE) || (0 == is_upd)) {
                continue;
            }

            switch (reg_ofs) {
            case POU_TUNING_REG_OFS:
                p_cmd[idx] = (p_cmd[idx] & instr_mask) + ((uint64_t)p_tuning->str << (uint64_t)32);
                break;
            default:
                break;
            }
        }
    }
    return 0;
}

int32_t set_ppu_gain_instr(JMISP_INSTR_GAIN_INFO* p_gain, JMISP_REG_INFO* p_reg_info, JMISPPPU_HEAD* p_ppu_head, uintptr_t cmd_addr, uint32_t cmd_lines, AI_JMISP_INFO* p_jmisp_info) {
    const uint64_t instr_mask = g_instr_mask;
    uint64_t* p_cmd;
    if ((p_gain == NULL) || (p_reg_info == NULL) || (p_ppu_head == NULL) || (cmd_addr == 0) || (p_jmisp_info == NULL)) {
        return -1;
    }

    p_cmd = (uint64_t*)cmd_addr;
    {
        const e_AI_JMISP_IOBUF_CONST iobuf_info_type = (e_AI_JMISP_IOBUF_CONST)p_ppu_head->match_iobuf_ID[0];
        const e_AI_JMISP_REG_IO_CONST reg_iobuf_type = convert_iobuf2regbuf_type(&p_jmisp_info->iobuf[iobuf_info_type], iobuf_info_type);
        const JMISP_REG_IOBUF* p_reg_iobuf;
        int32_t idx=0;

        if (reg_iobuf_type < 0) {
            ASSERT_MSG(0, "Unknown regbuf type in reg.\n");
            return -1;
        } else if (!is_jmisp_sup_buf(&p_jmisp_info->iobuf[iobuf_info_type], iobuf_info_type)) {
            ASSERT_MSG(0, "Unknown iobuf type in reg.\n");
            return -1;
        }
        p_reg_iobuf = &p_reg_info->iobuf_reg[reg_iobuf_type];

        for (idx=0; idx<cmd_lines; ++idx) {
            const uint32_t mode = get_cmd_mode(p_cmd[idx]);
            const uint32_t reg_ofs = get_upd_cmd_regofs(p_cmd[idx]);
            const uint32_t is_upd = get_upd_cmd_byte_en(p_cmd[idx]);
            uint64_t tmp_val;
            if (mode == NULL_MODE) {
                break;
            }
            if ((mode != UPD_MODE) || (0 == is_upd)) {
                continue;
            }

            switch (reg_ofs) {
            case PPU_QUAN_OUT0_REG_OFS:
                tmp_val = ((p_gain->quan_out[0].dir & 0x1) << (uint64_t)0)
                            + ((p_gain->quan_out[0].shift & 0xff) << (uint64_t)4)
                            + ((p_gain->quan_out[0].scale & 0xffff) << (uint64_t)12);
                p_cmd[idx] = (p_cmd[idx] & instr_mask) + (tmp_val << (uint64_t)32);
                break;
            case PPU_QUAN_OUT1_REG_OFS:
                tmp_val = ((p_gain->quan_out[1].dir & 0x1) << (uint64_t)0)
                            + ((p_gain->quan_out[1].shift & 0xff) << (uint64_t)4)
                            + ((p_gain->quan_out[1].scale & 0xffff) << (uint64_t)12);
                p_cmd[idx] = (p_cmd[idx] & instr_mask) + (tmp_val << (uint64_t)32);
                break;
            case PPU_QUAN_OUT2_REG_OFS:
                tmp_val = ((p_gain->quan_out[2].dir & 0x1) << (uint64_t)0)
                            + ((p_gain->quan_out[2].shift & 0xff) << (uint64_t)4)
                            + ((p_gain->quan_out[2].scale & 0xffff) << (uint64_t)12);
                p_cmd[idx] = (p_cmd[idx] & instr_mask) + (tmp_val << (uint64_t)32);
                break;
            default:
                break;
            }
        }
    }
    return 0;
}

int32_t get_ppu_gain_instr(JMISP_INSTR_GAIN_INFO* p_gain, JMISP_REG_INFO* p_reg_info, JMISPPPU_HEAD* p_ppu_head, uintptr_t cmd_addr, uint32_t cmd_lines, AI_JMISP_INFO* p_jmisp_info) {
#define QUAN_OUT0_FOUND 0x1
#define QUAN_OUT1_FOUND 0x2
#define QUAN_OUT2_FOUND 0x4
    uint64_t* p_cmd;
    if ((p_gain == NULL) || (p_reg_info == NULL) || (p_ppu_head == NULL) || (cmd_addr == 0) || (p_jmisp_info == NULL)) {
        return -1;
    }

    p_cmd = (uint64_t*)cmd_addr;
    {
        const e_AI_JMISP_IOBUF_CONST iobuf_info_type = (e_AI_JMISP_IOBUF_CONST)p_ppu_head->match_iobuf_ID[0];
        const e_AI_JMISP_REG_IO_CONST reg_iobuf_type = convert_iobuf2regbuf_type(&p_jmisp_info->iobuf[iobuf_info_type], iobuf_info_type);
        const JMISP_REG_IOBUF* p_reg_iobuf;
        int32_t idx=0;
        uint8_t is_found = 0;

        if (reg_iobuf_type < 0) {
            ASSERT_MSG(0, "Unknown regbuf type in reg.\n");
            return -1;
        } else if (!is_jmisp_sup_buf(&p_jmisp_info->iobuf[iobuf_info_type], iobuf_info_type)) {
            ASSERT_MSG(0, "Unknown iobuf type in reg.\n");
            return -1;
        }
        p_reg_iobuf = &p_reg_info->iobuf_reg[reg_iobuf_type];

        for (idx=0; idx<cmd_lines; ++idx) {
            const uint32_t mode = get_cmd_mode(p_cmd[idx]);
            const uint32_t reg_ofs = get_upd_cmd_regofs(p_cmd[idx]);
            const uint32_t is_upd = get_upd_cmd_byte_en(p_cmd[idx]);
            uint64_t tmp_val;
            if (mode == NULL_MODE) {
                break;
            } else if ((mode != UPD_MODE) || (0 == is_upd)) {
                continue;
            } else if ((is_found & QUAN_OUT0_FOUND) && (is_found & QUAN_OUT1_FOUND) && (is_found & QUAN_OUT2_FOUND)) {
                break;
            }

            switch (reg_ofs) {
            case PPU_QUAN_OUT0_REG_OFS:
                tmp_val = p_cmd[idx] >> (uint64_t)32;
                p_gain->quan_out[0].dir = tmp_val & 0x1;
                p_gain->quan_out[0].shift = (tmp_val >> 4) & 0xff;
                p_gain->quan_out[0].scale = (tmp_val >> 12) & 0xffff;
                is_found |= QUAN_OUT0_FOUND;
                break;
            case PPU_QUAN_OUT1_REG_OFS:
                tmp_val = p_cmd[idx] >> (uint64_t)32;
                p_gain->quan_out[1].dir = tmp_val & 0x1;
                p_gain->quan_out[1].shift = (tmp_val >> 4) & 0xff;
                p_gain->quan_out[1].scale = (tmp_val >> 12) & 0xffff;
                is_found |= QUAN_OUT1_FOUND;
                break;
            case PPU_QUAN_OUT2_REG_OFS:
                tmp_val = p_cmd[idx] >> (uint64_t)32;
                p_gain->quan_out[2].dir = tmp_val & 0x1;
                p_gain->quan_out[2].shift = (tmp_val >> 4) & 0xff;
                p_gain->quan_out[2].scale = (tmp_val >> 12) & 0xffff;
                is_found |= QUAN_OUT2_FOUND;
                break;
            default:
                break;
            }
        }
    }
    return 0;
}

int32_t upd_blockcase_instr(JMISP_INSTR_BLOCK_INFO* p_blk_case, JMISP_REG_INFO* p_reg_info, JMISPPPU_HEAD* p_ppu_head, uintptr_t cmd_addr, uint32_t cmd_lines, AI_JMISP_INFO* p_jmisp_info) {
    if (p_ppu_head == NULL) {
        return -1;
    }
    if (p_ppu_head->config.word == 0) {
        // for 690, 538
        return upd_ppu_blockcase_instr(p_blk_case, p_reg_info, p_ppu_head, cmd_addr, cmd_lines, p_jmisp_info);
    } else if (p_ppu_head->config.bit.unit_mode == e_UNIT_SEL_PPU) {
        return upd_ppu_blockcase_instr(p_blk_case, p_reg_info, p_ppu_head, cmd_addr, cmd_lines, p_jmisp_info);
    } else if (p_ppu_head->config.bit.unit_mode == e_UNIT_SEL_POU) {
        return upd_pou_blockcase_instr(p_blk_case, p_reg_info, p_ppu_head, cmd_addr, cmd_lines, p_jmisp_info);
    } else {
        ASSERT_MSG(0, "ERR: unknown pre/post proc unit type\n");
        return -1;
    }
}

int32_t upd_tuning_instr(JMISP_INSTR_TUNING_INFO* p_tuning, JMISP_REG_INFO* p_reg_info, JMISPPPU_HEAD* p_ppu_head, uintptr_t cmd_addr, uint32_t cmd_lines, AI_JMISP_INFO* p_jmisp_info) {
    if (p_ppu_head == NULL) {
        return -1;
    }
    if (p_ppu_head->config.bit.unit_mode == e_UNIT_SEL_POU) {
        return upd_pou_tuning_instr(p_tuning, p_reg_info, p_ppu_head, cmd_addr, cmd_lines, p_jmisp_info);
    } else {
        ASSERT_MSG(0, "ERR: unknown pre/post proc unit type\n");
        return -1;
    }
}

int32_t set_gain_instr(JMISP_INSTR_GAIN_INFO* p_gain, JMISP_REG_INFO* p_reg_info, JMISPPPU_HEAD* p_ppu_head, uintptr_t cmd_addr, uint32_t cmd_lines, AI_JMISP_INFO* p_jmisp_info) {
    if (p_ppu_head == NULL) {
        return -1;
    }
    if (p_ppu_head->config.word == 0) {
        // for 690, 538
        return set_ppu_gain_instr(p_gain, p_reg_info, p_ppu_head, cmd_addr, cmd_lines, p_jmisp_info);
    } else if (p_ppu_head->config.bit.unit_mode == e_UNIT_SEL_PPU) {
        return set_ppu_gain_instr(p_gain, p_reg_info, p_ppu_head, cmd_addr, cmd_lines, p_jmisp_info);
    } else {
        ASSERT_MSG(0, "ERR: unknown pre/post proc unit type\n");
        return -1;
    }
}

int32_t gen_block_case_num(JMISPPPU_HEAD* p_ppu_head, AI_JMISP_INFO* p_jmisp_info) {
    int32_t case_num = 0;
    if ((p_ppu_head == NULL) || (p_jmisp_info == NULL)) {
        return case_num;
    }

    case_num += 9;

#if 1   // disable if block case shrink
    case_num += 6;
#endif

#if REF_OUT_STRIPE_CROP_WITH_PPU
{
    e_AI_JMISP_IOBUF_CONST iobuf_info_type = (e_AI_JMISP_IOBUF_CONST)p_ppu_head->match_iobuf_ID[0];
    AI_JMISP_IOBUF_INFO* p_iobuf_info = (AI_JMISP_IOBUF_INFO*)&p_jmisp_info->iobuf[iobuf_info_type];
    if (is_frame_out_buf(p_iobuf_info, iobuf_info_type)) {
        case_num += 6;
    }
}
#endif
    return case_num;
}

int32_t gen_block_case_table(JMISP_INSTR_CASE* p_table, int32_t table_sz, JMISPPPU_HEAD* p_ppu_head, AI_JMISP_INFO* p_jmisp_info) {
    const int32_t case_num = gen_block_case_num(p_ppu_head, p_jmisp_info);
    int32_t idx=0;
    if ((p_table == NULL) || (table_sz <= 0) || (table_sz < (sizeof(JMISP_INSTR_CASE) * case_num))
        || (case_num <= 0) || (p_ppu_head == NULL) || (p_jmisp_info == NULL)) {
        return -1;
    }
    p_table[idx].stripe_case = e_STRIPECASE_LEFT;
    p_table[idx].block_case = e_BLOCKCASE_LEFT_TOP;
    ++idx;
    p_table[idx].stripe_case = e_STRIPECASE_LEFT;
    p_table[idx].block_case = e_BLOCKCASE_LEFT;
    ++idx;
    p_table[idx].stripe_case = e_STRIPECASE_LEFT;
    p_table[idx].block_case = e_BLOCKCASE_BOTTOM_LEFT;
    ++idx;
    p_table[idx].stripe_case = e_STRIPECASE_LEFT;
    p_table[idx].block_case = e_BLOCKCASE_TOP;
    ++idx;
    p_table[idx].stripe_case = e_STRIPECASE_LEFT;
    p_table[idx].block_case = e_BLOCKCASE_MID;
    ++idx;
    p_table[idx].stripe_case = e_STRIPECASE_LEFT;
    p_table[idx].block_case = e_BLOCKCASE_BOTTOM;
    ++idx;
    p_table[idx].stripe_case = e_STRIPECASE_LEFT;
    p_table[idx].block_case = e_BLOCKCASE_TOP_RIGHT;
    ++idx;
    p_table[idx].stripe_case = e_STRIPECASE_LEFT;
    p_table[idx].block_case = e_BLOCKCASE_RIGHT;
    ++idx;
    p_table[idx].stripe_case = e_STRIPECASE_LEFT;
    p_table[idx].block_case = e_BLOCKCASE_RIGHT_BOTTOM;
    ++idx;
#if 1   // disable if block case shrink
    p_table[idx].stripe_case = e_STRIPECASE_MID;
    p_table[idx].block_case = e_BLOCKCASE_TOP_RIGHT;
    ++idx;
    p_table[idx].stripe_case = e_STRIPECASE_MID;
    p_table[idx].block_case = e_BLOCKCASE_RIGHT;
    ++idx;
    p_table[idx].stripe_case = e_STRIPECASE_MID;
    p_table[idx].block_case = e_BLOCKCASE_RIGHT_BOTTOM;
    ++idx;
    p_table[idx].stripe_case = e_STRIPECASE_RIGHT;
    p_table[idx].block_case = e_BLOCKCASE_TOP_RIGHT;
    ++idx;
    p_table[idx].stripe_case = e_STRIPECASE_RIGHT;
    p_table[idx].block_case = e_BLOCKCASE_RIGHT;
    ++idx;
    p_table[idx].stripe_case = e_STRIPECASE_RIGHT;
    p_table[idx].block_case = e_BLOCKCASE_RIGHT_BOTTOM;
    ++idx;
#endif
#if REF_OUT_STRIPE_CROP_WITH_PPU
{
    e_AI_JMISP_IOBUF_CONST iobuf_info_type = (e_AI_JMISP_IOBUF_CONST)p_ppu_head->match_iobuf_ID[0];
    AI_JMISP_IOBUF_INFO* p_iobuf_info = (AI_JMISP_IOBUF_INFO*)&p_jmisp_info->iobuf[iobuf_info_type];
    if (is_frame_out_buf(iobuf_info_type)) {
        p_table[idx].stripe_case = e_STRIPECASE_LEFT;
        p_table[idx].block_case = e_BLOCKCASE_LEFT_TOP;
        ++idx;
        p_table[idx].stripe_case = e_STRIPECASE_LEFT;
        p_table[idx].block_case = e_BLOCKCASE_LEFT;
        ++idx;
        p_table[idx].stripe_case = e_STRIPECASE_LEFT;
        p_table[idx].block_case = e_BLOCKCASE_BOTTOM_LEFT;
        ++idx;
        p_table[idx].stripe_case = e_STRIPECASE_LEFT;
        p_table[idx].block_case = e_BLOCKCASE_LEFT_TOP;
        ++idx;
        p_table[idx].stripe_case = e_STRIPECASE_LEFT;
        p_table[idx].block_case = e_BLOCKCASE_LEFT;
        ++idx;
        p_table[idx].stripe_case = e_STRIPECASE_LEFT;
        p_table[idx].block_case = e_BLOCKCASE_BOTTOM_LEFT;
        ++idx;
    }
}
#endif
    if (case_num != idx) {
        ASSERT_MSG(0, "ERR: not correct number of block case\n");
    }

    return 0;
}


int32_t upd_ppu_all_blockcase_instr(JMISP_INSTR_BLOCK_CASE_INFO* p_blkcase_info, JMISP_REG_INFO* p_reg_info, JMISPPPU_HEAD* p_ppu_head, AI_JMISP_INFO* p_jmisp_info) {
    int32_t case_num = 0, idx;
    uint32_t per_case_llcmd_sz, cmd_lines;
    uintptr_t cmd_addr;
    JMISP_INSTR_CASE table[e_STRIPECASE_NUM * e_BLOCKCASE_NUM];
    if ((p_blkcase_info == NULL) || (p_reg_info == NULL) || (p_ppu_head == NULL) || (p_jmisp_info == NULL)) {
        return -1;
    }

    case_num = gen_block_case_num(p_ppu_head, p_jmisp_info);
    if (case_num <= 0) {
        return -1;
    }
    gen_block_case_table(table, sizeof(table), p_ppu_head, p_jmisp_info);
    per_case_llcmd_sz = p_ppu_head->parm_size / case_num;
    cmd_addr = p_ppu_head->parm_addr;
    cmd_lines = per_case_llcmd_sz >> 3; // sizeof(UINT64);
    for (idx=0; idx<case_num; ++idx) {
        const JMISP_INSTR_STRIPE_CASE_TYPE stripe_case = table[idx].stripe_case;
        const JMISP_INSTR_BLOCK_CASE_TYPE block_case = table[idx].block_case;
        upd_blockcase_instr(&p_blkcase_info->blocks[stripe_case][block_case], p_reg_info, p_ppu_head, cmd_addr, cmd_lines, p_jmisp_info);
        cmd_addr += per_case_llcmd_sz;
    }

    return 0;
}

int32_t upd_tuning_in_all_instr(JMISP_INSTR_TUNING_INFO* p_info, JMISP_REG_INFO* p_reg_info, JMISPPPU_HEAD* p_ppu_head, AI_JMISP_INFO* p_jmisp_info) {
    int32_t case_num = 0, idx;
    uint32_t per_case_llcmd_sz, cmd_lines;
    uintptr_t cmd_addr;
    if ((p_info == NULL) || (p_reg_info == NULL) || (p_ppu_head == NULL) || (p_jmisp_info == NULL)) {
        return -1;
    }

    case_num = gen_block_case_num(p_ppu_head, p_jmisp_info);
    if (case_num <= 0) {
        return -1;
    }
    per_case_llcmd_sz = p_ppu_head->parm_size / case_num;
    cmd_addr = p_ppu_head->parm_addr;
    cmd_lines = per_case_llcmd_sz >> 3; // sizeof(UINT64);
    for (idx=0; idx<case_num; ++idx) {
        upd_tuning_instr(p_info, p_reg_info, p_ppu_head, cmd_addr, cmd_lines, p_jmisp_info);
        cmd_addr += per_case_llcmd_sz;
    }

    return 0;
}

int32_t set_gain_in_all_instr(JMISP_INSTR_GAIN_INFO* p_info, JMISP_REG_INFO* p_reg_info, JMISPPPU_HEAD* p_ppu_head, AI_JMISP_INFO* p_jmisp_info) {
    int32_t case_num = 0, idx;
    uint32_t per_case_llcmd_sz, cmd_lines;
    uintptr_t cmd_addr;
    if ((p_info == NULL) || (p_reg_info == NULL) || (p_ppu_head == NULL) || (p_jmisp_info == NULL)) {
        return -1;
    }

    case_num = gen_block_case_num(p_ppu_head, p_jmisp_info);
    if (case_num <= 0) {
        return -1;
    }
    per_case_llcmd_sz = p_ppu_head->parm_size / case_num;
    cmd_addr = p_ppu_head->parm_addr;
    cmd_lines = per_case_llcmd_sz >> 3; // sizeof(UINT64);
    for (idx=0; idx<case_num; ++idx) {
        set_gain_instr(p_info, p_reg_info, p_ppu_head, cmd_addr, cmd_lines, p_jmisp_info);
        cmd_addr += per_case_llcmd_sz;
    }

    return 0;
}

int32_t get_gain_in_all_instr(JMISP_INSTR_GAIN_INFO* o_info, JMISP_REG_INFO* p_reg_info, JMISPPPU_HEAD* p_ppu_head, AI_JMISP_INFO* p_jmisp_info) {
    uint32_t cmd_lines;
    uintptr_t cmd_addr;
    if ((o_info == NULL) || (p_reg_info == NULL) || (p_ppu_head == NULL) || (p_jmisp_info == NULL)) {
        return -1;
    }

    cmd_addr = p_ppu_head->parm_addr;
    cmd_lines = p_ppu_head->parm_size >> 3; // sizeof(UINT64);
    get_ppu_gain_instr(o_info, p_reg_info, p_ppu_head, cmd_addr, cmd_lines, p_jmisp_info);

    return 0;
}