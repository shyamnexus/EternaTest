/**
	@brief Source file of kflow_ai_isp_change_res.

	@file kflow_isp_change_res.h

	@ingroup kflow_isp

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#ifndef _KFLOW_ISP_CHANGE_RES_H_
#define _KFLOW_ISP_CHANGE_RES_H_

#include "kwrap/type.h"
#include "kwrap/platform.h"
#include "kwrap/error_no.h"
#include "kdrv_ai.h"

#define E_RES           (-101)      //Resolution exceeds model limits


typedef enum {
  e_AI_JMISP_DRV_IO_UNKNOWN   = -1,
  e_AI_JMISP_DRV_IO_RING_BUF0 = 0,
  e_AI_JMISP_DRV_IO_RING_BUF1 = 1,
  e_AI_JMISP_DRV_IO_FRAME_BUF0 = 2,
  e_AI_JMISP_DRV_IO_FRAME_BUF1 = 3,
  e_AI_JMISP_DRV_IO_FRAME_BUF2 = 4,
  e_AI_JMISP_DRV_IO_FRAME_BUF3 = 5,

  e_AI_JMISP_DRV_IO_PINGPONG_BUF0 = 0,
  e_AI_JMISP_DRV_IO_PINGPONG_BUF1 = 1,
  e_AI_JMISP_DRV_IO_PINGPONG_BUF2 = 2,

  e_AI_JMISP_DRV_IO_PIPE_BUF0 = 0,
  e_AI_JMISP_DRV_IO_PIPE_BUF1 = 1,
} e_AI_JMISP_DRV_IO_CONST;

typedef enum {
  e_AI_JMISP_IOBUF_UNKNOWN   = -1,
  e_AI_JMISP_IOBUF_RING_IN0,                  // uBuff y
  e_AI_JMISP_IOBUF_RING_IN1,                  // uBuff uv
  e_AI_JMISP_IOBUF_PINGPONG_OUT0,             // uBuff y(slice)
  e_AI_JMISP_IOBUF_PINGPONG_OUT1,             // uBuff uv(slice)
  e_AI_JMISP_IOBUF_PINGPONG_OUT2,             // uBuff texture(slice)
  e_AI_JMISP_IOBUF_REF_IN0,                   // Dram Buff y-in(frame)
  e_AI_JMISP_IOBUF_REF_IN1,                   // Dram Buff uv-in(frame)
  e_AI_JMISP_IOBUF_REF_OUT0,                  // Dram Buff y
  e_AI_JMISP_IOBUF_REF_OUT1,                  // Dram Buff uv
  e_AI_JMISP_IOBUF_INPUTFEAT_PINGPONG_SEG0,   // input feat pingpong 0
  e_AI_JMISP_IOBUF_INPUTFEAT_PINGPONG_SEG1,   // input feat pingpong 1
  e_AI_JMISP_IOBUF_PPU_PIPEBUF0,              // ppu pipe pingpong buffer 0
  e_AI_JMISP_IOBUF_PPU_PIPEBUF1,              // ppu pipe pingpong buffer 1
  e_AI_JMISP_IOBUF_FUSION_WEIGHT_IN,          // Dram isp fusion weight
  e_AI_JMISP_IOBUF_SIGMA_IN,                  // Dram isp sigma
  e_AI_JMISP_IOBUF_GAMMA_IN,                  // Dram isp gamma
  e_AI_JMISP_IOBUF_HDR_IN0,                   // hdr Buff
  e_AI_JMISP_IOBUF_GAMMA_OUT,                 // Dram isp gamma out
  e_AI_JMISP_IOBUF_RESERVED_blk5,             // reserved_blk5
  e_AI_IOBUF_NUM,
} e_AI_JMISP_IOBUF_CONST;

typedef enum {
  e_AI_JMISP_BASE_ADDR_NON = 0, // use absolute addr
  e_AI_JMISP_BASE_ADDR_0 = 1, // weight
  e_AI_JMISP_BASE_ADDR_1 = 2, // DRAM
  e_AI_JMISP_BASE_ADDR_2 = 3, // llcmd trigger blk
  e_AI_JMISP_BASE_ADDR_3 = 4, // SRAM
  e_AI_JMISP_BASE_ADDR_4 = 5  // for JMISP
} IO_BASE_ADDR_TYPE;

typedef enum {
  e_BOUND_UNKNOWN = -1,
  e_BOUND_TOP = 0,
  e_BOUND_RIGHT,
  e_BOUND_BOTTOM,
  e_BOUND_LEFT,

  e_BOUND_NUM,
} JMISP_INSTR_BOUND_TYPE;   

typedef struct {
  int32_t frame_width;
  int32_t frame_height;
} FrameInfo;

typedef struct {
  int32_t first_stripe_width;
  int32_t mid_stripe_width;
  int32_t last_stripe_width;
  int32_t stripe_overlap_width;
  int32_t slice_overlap_height;
  int32_t output_slice_height;
} StripeInfo;

typedef struct {
  int32_t block_width;          // indicate input of PPU
  int32_t block_overlap_width;  // indicate input of PPU
  int32_t block_height;         // indicate input of PPU
  int32_t block_overlap_height; // indicate input of PPU
} BlockInfo;

/*
    for dynamic resolution
    scale/shift: based on input Y frame size
*/
typedef struct {
  int32_t scale_x;
  int32_t shift_x;
  int32_t scale_y;
  int32_t shift_y;
} RatioInfo;



typedef struct {
  FrameInfo   frameInfo;
  StripeInfo stripeInfo;
  BlockInfo   blockInfo;
  IO_BASE_ADDR_TYPE addrType;
  UINT64 pa;
  UINT64 va;
  int32_t match_ppu_id; // initialize as -1
  uint32_t bitdepth;
  // ping pong buffer size = slice size * 2
  // ring buffer size = slice size * 3
  // ref buffer size = frame size * 1
  uint32_t buffer_sz; 
  RatioInfo ratioInfo;
  uint16_t channel;
  uint8_t reserved_blk[62];
}__attribute__((packed)) AI_JMISP_IOBUF_INFO;

typedef struct {
  AI_JMISP_IOBUF_INFO iobuf[e_AI_IOBUF_NUM];
} AI_JMISP_INFO;

typedef struct {
  int32_t frame_width;
  int32_t frame_height;
  int32_t first_stripe_width;
  int32_t mid_stripe_width;
  int32_t last_stripe_width;
  int32_t stripe_overlap_width;
  int32_t stripe_num;
} IPP_INFO;

typedef struct {
  int32_t mctrl_id;
  int32_t tuning_para;
} AI_TUNING_PARA;

typedef struct {
  uint8_t  dir;
  uint8_t  shift;
  uint32_t scale;
} AI_PPU_QUAN_SCALE_SHIFT;

typedef struct {
  int32_t mctrl_id;
  AI_PPU_QUAN_SCALE_SHIFT quan_out[3];
} AI_TUNING_QUAN_PARAM;

typedef struct {
  uint8_t  int_bit;
  uint8_t  frac_bit;
  uint32_t value;
} AI_FLOAT_FORMAT;


extern uint32_t get_ppu_num(AI_JMISP_INFO* p_info);
extern ER convert_dyframe2jmisp_info(const IPP_INFO* p_ipp_info, AI_JMISP_INFO* p_ori, AI_JMISP_INFO* p_out);
extern ER parsing_ppu_instrs(AI_JMISP_INFO* p_info, uintptr_t ppu_addr);
extern ER change_pou_inputfeat_seg(AI_JMISP_INFO* p_info, uintptr_t pou_addr, AI_TUNING_PARA* p_para);
extern ER update_jmisp_kdrv_resolution(AI_JMISP_INFO* p_info, KDRV_AI_JMISP_PATH_INFO* p_drv_para);
extern ER get_ppu_quan_param(AI_JMISP_INFO* p_info, uintptr_t ppu_addr, AI_TUNING_QUAN_PARAM* p_tuning_param);
extern ER set_ppu_quan_param(AI_JMISP_INFO* p_info, uintptr_t ppu_addr, AI_TUNING_QUAN_PARAM* p_tuning_param);
extern ER update_gain(const AI_TUNING_QUAN_PARAM* i_para, AI_TUNING_QUAN_PARAM* o_para, const AI_FLOAT_FORMAT gain);

//for frame mode
//block pipe
extern uint32_t ispframemode_get_total_blk_num(VOID* p_reg_info);
extern uint32_t ispframemode_get_max_blk_num_in_a_slice(VOID* p_info);
extern uint32_t ispframemode_get_blk_num_in_a_slice(VOID* p_info, uint32_t slice_idx);
extern ER ispframemode_create_blknum_array(VOID* p_reg_info, uint32_t* p_blknum, const uint32_t cnt);
extern ER ispframemode_create_blkinfo_array(AI_JMISP_INFO* p_info, KDRV_AI_JMISP_PATH_INFO* p_drv_array, const uint32_t cnt);
extern ER ispframemode_upd_blk_pingpong_addr(VOID *p_reg_info, KDRV_AI_JMISP_PATH_INFO* p_drv_array, const uint32_t cnt, e_AI_JMISP_DRV_IO_CONST type
                                              , const uint64_t pp_addr0, const uint64_t pp_addr1);
extern ER ispframemode_upd_blk_ringbuf_addr(VOID *p_reg_info, KDRV_AI_JMISP_PATH_INFO* p_drv_array, const uint32_t cnt, e_AI_JMISP_DRV_IO_CONST type
                                              , bool ring_en, const uint64_t ring_saddr, const uint64_t ring_eaddr);

//block pipe & ppu pipe
extern uint32_t ispframemode_get_total_substripe_num(VOID* p_info, const uint32_t core_num);
extern uint32_t ispframemode_get_max_substripe_num_in_a_slice(VOID* p_info, const uint32_t core_num);
extern uint32_t ispframemode_get_substripe_num_in_a_slice(VOID* p_info, uint32_t slice_idx, const uint32_t core_num);
extern ER ispframemode_create_substripe_num_array(VOID *p_info, uint32_t* p_substripe_num, const uint32_t cnt, const uint32_t core_num);
extern ER ispframemode_create_substripe_array(AI_JMISP_INFO* p_info, KDRV_AI_JMISP_PATH_INFO* p_drv_array, const uint32_t cnt, const uint32_t core_num);
extern ER ispframemode_upd_substripe_pingpong_addr(VOID *p_reg_info, KDRV_AI_JMISP_PATH_INFO* p_drv_array, const uint32_t cnt, e_AI_JMISP_DRV_IO_CONST type
                                                    , const uint64_t pp_addr0, const uint64_t pp_addr1, const uint32_t core_num);
extern ER ispframemode_upd_substripe_ringbuf_addr(VOID *p_reg_info, KDRV_AI_JMISP_PATH_INFO* p_drv_array, const uint32_t cnt, e_AI_JMISP_DRV_IO_CONST type
                                                    , bool ring_en, const uint64_t ring_saddr, const uint64_t ring_eaddr, const uint32_t core_num);
extern ER ispframemode_upd_substripe_ppupipebuf_addr(VOID *p_info, KDRV_AI_JMISP_PATH_INFO* p_drv_array, const uint32_t cnt, const uint32_t core_num
                                                      , const uint64_t ppubuf_addr0, const uint64_t ppubuf_addr1, const uint32_t ppubuf_sz);

//other
extern uintptr_t ispframemode_get_jmisp_null_pa_pos(uintptr_t jmisp_addr, uintptr_t parm_va_ofs, uintptr_t parm_pa_ofs, UINT32 size);
extern uint32_t ispframemode_get_total_slice_num(VOID* p_info);
extern e_AI_JMISP_DRV_IO_CONST convert_regbuf2drvio_type(int32_t type);
extern e_AI_JMISP_DRV_IO_CONST convert_iobuf2drvio_type(AI_JMISP_IOBUF_INFO* p_iobuf_info, e_AI_JMISP_IOBUF_CONST iobuf_type);

#endif //_KFLOW_ISP_CHANGE_RES_H_
