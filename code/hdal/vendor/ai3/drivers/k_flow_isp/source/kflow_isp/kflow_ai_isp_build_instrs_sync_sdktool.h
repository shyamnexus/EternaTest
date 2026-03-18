/**
	@brief Source file of kflow_ai_isp_build_instrs_sync_sdktool.

	@file kflow_ai_isp_build_instrs_sync_sdktool.h

	@ingroup kflow_isp

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#ifndef _KFLOW_AI_ISP_BUILD_INSTRS_SYNC_SDKTOOL_H_
#define _KFLOW_AI_ISP_BUILD_INSTRS_SYNC_SDKTOOL_H_

#include "kwrap/type.h"
#include "kwrap/platform.h"
#include "kwrap/error_no.h"
#include "kflow_isp/kflow_isp_change_res.h"


#define ALIGN_CEIL_DIV(value, base)     ((value + (base - 1)) / base)
#define ALIGN_CEIL_2(a)                 (((a) + 0x01) & ~0x01)
#define JMISP_PPU_SUP_IOBUF_NUM         2

#define REF_OUT_STRIPE_CROP_WITH_PPU    0


typedef union _JMISPPPU_CONFIG {
    struct {
        unsigned unit_mode : 4;     // bits :0
    } bit;
    uint32_t word;
}__attribute__((packed)) JMISPPPU_CONFIG;

typedef struct _JMISPPPU_HEAD {
    uintptr_t parm_addr;
    uint32_t parm_size;
    uint32_t match_mctrl_idx;
    uint32_t match_iobuf_ID[JMISP_PPU_SUP_IOBUF_NUM];
    JMISPPPU_CONFIG config;
    uint32_t __alignment_padding;
} __attribute__((packed)) JMISPPPU_HEAD;


typedef enum {
    e_AI_JMISP_REG_IO_UNKNOWN   = -1,
    e_AI_JMISP_REG_IO_RING_BUF0  = 0,
    e_AI_JMISP_REG_IO_RING_BUF1,
    e_AI_JMISP_REG_IO_FRAME_BUF0,
    e_AI_JMISP_REG_IO_FRAME_BUF1,
    e_AI_JMISP_REG_IO_FRAME_BUF2,
    e_AI_JMISP_REG_IO_FRAME_BUF3,
    e_AI_JMISP_REG_IO_PINGPONG_BUF0,
    e_AI_JMISP_REG_IO_PINGPONG_BUF1,
    e_AI_JMISP_REG_IO_PINGPONG_BUF2,
    e_AI_JMISP_REG_IO_CUSTPIPE_BUF0,
    e_AI_JMISP_REG_IO_CUSTPIPE_BUF1,

    e_AI_JMISP_REG_IO_NUM,

    e_AI_JMISP_REG_IO_RING_BUF_BEGIN = e_AI_JMISP_REG_IO_RING_BUF0,
    e_AI_JMISP_REG_IO_RING_BUF_END = e_AI_JMISP_REG_IO_RING_BUF1 + 1,
    e_AI_JMISP_REG_IO_FRAME_BUF_BEGIN = e_AI_JMISP_REG_IO_FRAME_BUF0,
    e_AI_JMISP_REG_IO_FRAME_BUF_END = e_AI_JMISP_REG_IO_FRAME_BUF3 + 1,
    e_AI_JMISP_REG_IO_PINGPONG_BUF_BEGIN = e_AI_JMISP_REG_IO_PINGPONG_BUF0,
    e_AI_JMISP_REG_IO_PINGPONG_BUF_END = e_AI_JMISP_REG_IO_PINGPONG_BUF2 + 1,
} e_AI_JMISP_REG_IO_CONST;

typedef enum {
    e_STRIPECASE_UNKNOWN = -1,
    e_STRIPECASE_LEFT = 0,
    e_STRIPECASE_MID,
    e_STRIPECASE_RIGHT,

    e_STRIPECASE_NUM,
} JMISP_INSTR_STRIPE_CASE_TYPE;

typedef enum {
    e_BLOCKCASE_UNKNOWN = -1,
    e_BLOCKCASE_LEFT_TOP = 0,
    e_BLOCKCASE_TOP,
    e_BLOCKCASE_TOP_RIGHT,

    e_BLOCKCASE_LEFT,
    e_BLOCKCASE_MID,
    e_BLOCKCASE_RIGHT,

    e_BLOCKCASE_BOTTOM_LEFT,
    e_BLOCKCASE_BOTTOM,
    e_BLOCKCASE_RIGHT_BOTTOM,

    e_BLOCKCASE_NUM,
} JMISP_INSTR_BLOCK_CASE_TYPE;

typedef enum {
    e_UNIT_SEL_UNKNOWN = -1,
    e_UNIT_SEL_PPU = 9,
    e_UNIT_SEL_POU = 10,
} JMISP_UNIT_SEL_TYPE;

typedef struct {
    int32_t width;
    int32_t height;
    int32_t pad[e_BOUND_NUM];
    int32_t crop[e_BOUND_NUM];
} JMISP_INSTR_BLOCK_INFO;

typedef struct {
    JMISP_INSTR_BLOCK_INFO blocks[e_STRIPECASE_NUM][e_BLOCKCASE_NUM];
} JMISP_INSTR_BLOCK_CASE_INFO;

typedef struct {
    int32_t str;    //motion + still
    int8_t reserved[60];
} JMISP_INSTR_TUNING_INFO;

typedef struct {
    uint8_t dir;
    uint8_t shift;
    uint16_t scale;
} JMISP_INSTR_QUAN_INFO;

typedef struct {
    JMISP_INSTR_QUAN_INFO quan_out[3];
    int8_t reserved[52];
} JMISP_INSTR_GAIN_INFO;

typedef struct {
    int32_t is_from_ringbuf;

    int32_t block_lofs;

    int32_t first_stripe_x_ofs;
    int32_t mid_stripe_x_ofs;
    int32_t first_slice_y_ofs;
    int32_t mid_slice_y_ofs;

    int32_t first_block_x_ofs;
    int32_t mid_block_x_ofs;
    int32_t first_block_y_ofs;
    int32_t mid_block_y_ofs;

    int32_t first_stripe_w;
    int32_t mid_stripe_w;
    int32_t last_stripe_w;
    int32_t first_slice_h;
    int32_t mid_slice_h;
    int32_t last_slice_h;
} JMISP_REG_IOBUF;

typedef struct {
    int32_t signal_mode;

    int32_t stripe_x_num;
    int32_t slice_y_num;

    int32_t first_stripe_block_x_num;
    int32_t mid_stripe_block_x_num;
    int32_t last_stripe_block_x_num;
    int32_t first_slice_block_y_num;
    int32_t mid_slice_block_y_num;
    int32_t last_slice_block_y_num;
    
    JMISP_REG_IOBUF iobuf_reg[e_AI_JMISP_REG_IO_NUM];

} JMISP_REG_INFO;


INT32 gen_jmisp_reg_para(AI_JMISP_INFO* p_jmisp_info, JMISP_REG_INFO* p_reg_info);
INT32 gen_ppu_blockcase_info(AI_JMISP_INFO* p_jmisp_info, JMISP_REG_INFO* p_reg_info, JMISPPPU_HEAD* p_ppu_head, JMISP_INSTR_BLOCK_CASE_INFO* p_ppu_blkcase_info);
int32_t gen_block_case_num(JMISPPPU_HEAD* p_ppu_head, AI_JMISP_INFO* p_jmisp_info);
int32_t upd_ppu_all_blockcase_instr(JMISP_INSTR_BLOCK_CASE_INFO* p_blkcase_info, JMISP_REG_INFO* p_reg_info, JMISPPPU_HEAD* p_ppu_head, AI_JMISP_INFO* p_jmisp_info);
int32_t upd_tuning_in_all_instr(JMISP_INSTR_TUNING_INFO* p_info, JMISP_REG_INFO* p_reg_info, JMISPPPU_HEAD* p_ppu_head, AI_JMISP_INFO* p_jmisp_info);
int32_t set_gain_in_all_instr(JMISP_INSTR_GAIN_INFO* p_info, JMISP_REG_INFO* p_reg_info, JMISPPPU_HEAD* p_ppu_head, AI_JMISP_INFO* p_jmisp_info);
int32_t get_gain_in_all_instr(JMISP_INSTR_GAIN_INFO* o_info, JMISP_REG_INFO* p_reg_info, JMISPPPU_HEAD* p_ppu_head, AI_JMISP_INFO* p_jmisp_info);
e_AI_JMISP_IOBUF_CONST get_benchmark_iobuf_type(VOID);
AI_JMISP_IOBUF_INFO* get_benchmark_iobuf(AI_JMISP_INFO* p_jmisp_info);
e_AI_JMISP_REG_IO_CONST convert_iobuf2regbuf_type(AI_JMISP_IOBUF_INFO* p_iobuf_info, e_AI_JMISP_IOBUF_CONST iobuf_type);
BOOL is_in_buf(AI_JMISP_IOBUF_INFO* p_iobuf_info, e_AI_JMISP_IOBUF_CONST type);;
BOOL is_out_buf(AI_JMISP_IOBUF_INFO* p_iobuf_info, e_AI_JMISP_IOBUF_CONST type);
BOOL is_ring_buf(AI_JMISP_IOBUF_INFO* p_iobuf_info, e_AI_JMISP_IOBUF_CONST type);
BOOL is_pingpong_buf(AI_JMISP_IOBUF_INFO* p_iobuf_info, e_AI_JMISP_IOBUF_CONST type);
BOOL is_frame_buf(AI_JMISP_IOBUF_INFO* p_iobuf_info, e_AI_JMISP_IOBUF_CONST type);
BOOL is_cust_pipe_buf(AI_JMISP_IOBUF_INFO* p_iobuf_info, e_AI_JMISP_IOBUF_CONST type);
BOOL is_jmisp_sup_buf(AI_JMISP_IOBUF_INFO* p_iobuf_info, e_AI_JMISP_IOBUF_CONST type);

#if 0
BOOL is_ring_in_buf(AI_JMISP_IOBUF_INFO* p_iobuf_info, e_AI_JMISP_IOBUF_CONST type);
BOOL is_ring_out_buf(AI_JMISP_IOBUF_INFO* p_iobuf_info, e_AI_JMISP_IOBUF_CONST type);
BOOL is_pingpong_in_buf(AI_JMISP_IOBUF_INFO* p_iobuf_info, e_AI_JMISP_IOBUF_CONST type);
BOOL is_pingpong_out_buf(AI_JMISP_IOBUF_INFO* p_iobuf_info, e_AI_JMISP_IOBUF_CONST type);
BOOL is_frame_in_buf(AI_JMISP_IOBUF_INFO* p_iobuf_info, e_AI_JMISP_IOBUF_CONST type);
BOOL is_frame_out_buf(AI_JMISP_IOBUF_INFO* p_iobuf_info, e_AI_JMISP_IOBUF_CONST type);
#endif
#endif //_KFLOW_AI_ISP_BUILD_INSTRS_SYNC_SDKTOOL_H_