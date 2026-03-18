/**
    Public header file for JMISP module.

    @file       jmisp_eng.h
    @ingjmispp    mIIPPJMISP

    @brief

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.
*/
#ifndef _JMISP_ENG_H_
#define _JMISP_ENG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "kwrap/type.h"
#include "kdrv_type.h"
#include "jmisp_eng_base.h"
#include "jmisp_eng_handle.h"
#include "jmisp_eng_dma_base.h"

#define JMISP_ENG_REG_NUMS  (0xDC4/4+1)  // for NT98690

#define JMISP_SSD_DRV_NAME "Ssdrv_jmisp"
#define JMISP_SSD_DRV_MODULE_VERSION "1.00.Beta.01"

#define FLGPTN_JMISP_PL0_END     	FLGPTN_BIT(0)
#define FLGPTN_JMISP_PL1_END     	FLGPTN_BIT(1)
#define FLGPTN_JMISP_PL2_END     	FLGPTN_BIT(2)
#define FLGPTN_JMISP_PL3_END     	FLGPTN_BIT(3)

#define JMISP_ENG_MAX_FRAME_NUM    6
#define JMISP_ENG_MAX_PINGPONG_NUM 3
#define JMISP_ENG_MAX_PATH_NUM     4
#define JMISP_ENG_MAX_CUST_NUM     2
#define JMISP_ENG_MAX_PIPEBUFF_NUM 2

/*****************************************************************************/
typedef struct {
	UINT32 ofs;
	UINT32 val;
} JMISP_ENG_REG;

typedef enum {
	JMISP_ENG_INTERRUPT_PL0_END            = 0x00000001,
	JMISP_ENG_INTERRUPT_PL1_END            = 0x00000002,
	JMISP_ENG_INTERRUPT_PL2_END            = 0x00000004,
	JMISP_ENG_INTERRUPT_PL3_END            = 0x00000008,
	
	JMISP_ENG_INTERRUPT_PL0_SLICE_END      = 0x00000100,
	JMISP_ENG_INTERRUPT_PL1_SLICE_END      = 0x00000200,
	JMISP_ENG_INTERRUPT_PL2_SLICE_END      = 0x00000400,
	JMISP_ENG_INTERRUPT_PL3_SLICE_END      = 0x00000800,
	
	JMISP_ENG_INTERRUPT_PL0_STRIPE_END     = 0x00001000,
	JMISP_ENG_INTERRUPT_PL1_STRIPE_END     = 0x00002000,
	JMISP_ENG_INTERRUPT_PL2_STRIPE_END     = 0x00004000,
	JMISP_ENG_INTERRUPT_PL3_STRIPE_END     = 0x00008000,
	
	JMISP_ENG_INTERRUPT_PL0_ERR            = 0x00010000,
	JMISP_ENG_INTERRUPT_PL1_ERR            = 0x00020000,
	JMISP_ENG_INTERRUPT_PL2_ERR            = 0x00040000,
	JMISP_ENG_INTERRUPT_PL3_ERR            = 0x00080000,

	JMISP_ENG_INTERRUPT_PL0_JLA_PPU_ERR    = 0x00100000,
	JMISP_ENG_INTERRUPT_PL1_JLB_PPU_ERR    = 0x00200000,
	JMISP_ENG_INTERRUPT_PL2_JLC_PPU_ERR    = 0x00400000,
	JMISP_ENG_INTERRUPT_PL3_JLD_PPU_ERR    = 0x00800000,
	
	JMISP_ENG_INTERRUPT_KDRV               = 0x00FF000F,
	JMISP_ENG_INTERRUPT_ALL                = 0x00FFFF0F,
} JMISP_ENG_INTERRUPT;

typedef enum {
	JMISP_ENG_HANDSHAKE_CPU        = 0,
	JMISP_ENG_HANDSHAKE_IPP0_PATH0 = 1,
	JMISP_ENG_HANDSHAKE_IPP0_PATH1 = 2,
	JMISP_ENG_HANDSHAKE_IPP0_PATH2 = 3,
	JMISP_ENG_HANDSHAKE_IPP0_PATH3 = 4,
	JMISP_ENG_HANDSHAKE_IPP1_PATH0 = 5,
	JMISP_ENG_HANDSHAKE_IPP1_PATH1 = 6,
	JMISP_ENG_HANDSHAKE_IPP1_PATH2 = 7,
	JMISP_ENG_HANDSHAKE_IPP1_PATH3 = 8,
	ENUM_DUMMY4WORD(JMISP_ENG_HANDSHAKE_TYPE)
} JMISP_ENG_HANDSHAKE_TYPE;

typedef struct _JMISP_ENG_BLOCK_NUM_INFO {
	UINT32 blk_x_first_num;  //0xC8/0xCC  FIRST/LAST/SLICEBUF_BLOCK_X_NUM
	UINT32 blk_x_middle_num;
	UINT32 blk_x_last_num;
	UINT32 blk_y_first_num;  //0xC8/0x2B8 FIRST/LAST/SLICEBUF_BLOCK_Y_NUM
	UINT32 blk_y_middle_num;
	UINT32 blk_y_last_num;
} JMISP_ENG_BLOCK_NUM_INFO;

typedef struct _JMISP_ENG_BLOCK_OFS_INFO {
	UINT32 blk_x_first_ofs;  //0x138 SLICEBUF0_BLOCK_X_OFS0
	UINT32 blk_x_middle_ofs; //0x138 SLICEBUF0_BLOCK_X_OFS1
	UINT32 blk_y_first_ofs;  //0x130 SLICEBUF0_BLOCK_Y_OFS0
	UINT32 blk_y_middle_ofs; //0x134 SLICEBUF0_BLOCK_Y_OFS1
} JMISP_ENG_BLOCK_OFS_INFO;


typedef struct _JMISP_ENG_FRAME_INFO {
	UINT32 ring_en;
	UINT64 ring_start_addr;
	UINT64 ring_end_addr;
	UINT64 frame_addr;
	UINT32 first_stripe_x_ofs;  //0xD0 FRAMEBUF0_FIRST_STRIPE_X_OFS
	UINT32 middle_stripe_x_ofs; //0xD0 FRAMEBUF0_MIDDLE_STRIPE_X_OFS	
	UINT32 first_slice_y_ofs;   //0xD8 FRAMEBUF0_SLICE_Y_OFS0
	UINT32 middle_slice_y_ofs;  //0xDC FRAMEBUF0_SLICE_Y_OFS1	
	JMISP_ENG_BLOCK_OFS_INFO blk_ofs_info;
} JMISP_ENG_FRAME_INFO;

typedef struct _JMISP_ENG_PINGPONG_INFO {
	UINT64 pingpong_addr0;
	UINT64 pingpong_addr1;
	JMISP_ENG_BLOCK_OFS_INFO blk_ofs_info;
} JMISP_ENG_PINGPONG_INFO;

typedef struct _JMISP_ENG_PATH_INFO {
	JMISP_ENG_FRAME_INFO     frame[JMISP_ENG_MAX_FRAME_NUM];
	JMISP_ENG_PINGPONG_INFO  pingpong[JMISP_ENG_MAX_PINGPONG_NUM];
	JMISP_ENG_HANDSHAKE_TYPE handshake;
	UINT32 stripe_num;
	UINT32 slice_num;
	UINT32 signal_mode_en;
	JMISP_ENG_BLOCK_NUM_INFO blk_num_info;
	UINT32 path_id;
	UINT32 cust[JMISP_ENG_MAX_CUST_NUM];
	UINT32 ppu_pipebuf_lsbaddr[JMISP_ENG_MAX_PIPEBUFF_NUM];
	UINT32 blk_skip;
	UINT32 wait_en;
} JMISP_ENG_PATH_INFO;

typedef struct _JMISP_ENG_FRAME_ADDR_INFO {
	UINT64 frame_addr[JMISP_ENG_MAX_FRAME_NUM];
	UINT32 path_id;
} JMISP_ENG_FRAME_ADDR_INFO;

typedef struct _JMISP_ENG_FRM_UPD_INFO {
	UINT32 ring_en;
	UINT64 ring_start_addr;
	UINT64 ring_end_addr;
	UINT64 frame_addr;
	JMISP_ENG_BLOCK_OFS_INFO blk_ofs_info;
} JMISP_ENG_FRM_UPD_INFO;

typedef struct _JMISP_ENG_FRM_UPD_PATH_INFO {
    JMISP_ENG_FRM_UPD_INFO   frame[JMISP_ENG_MAX_FRAME_NUM];
    JMISP_ENG_PINGPONG_INFO  pingpong[JMISP_ENG_MAX_PINGPONG_NUM];
    UINT32 cust[JMISP_ENG_MAX_CUST_NUM];
    UINT32 ppu_pipebuf_lsbaddr[JMISP_ENG_MAX_PIPEBUFF_NUM];
	UINT32 path_id;
} JMISP_ENG_FRM_UPD_PATH_INFO;

typedef void (*JMISP_ISR_CB)(void *eng, UINT32 status, void *reserve);

extern INT32 jmisp_eng_init(UINT32 chip_num, UINT32 eng_num);
extern INT32 jmisp_eng_release(void);

extern JMISP_ENG_HANDLE* jmisp_eng_get_handle(UINT32 chip_id, UINT32 eng_id);
extern void jmisp_eng_reg_isr_callback(JMISP_ENG_HANDLE *p_eng, JMISP_ISR_CB cb);
extern INT32 jmisp_eng_open(JMISP_ENG_HANDLE *p_eng);
extern INT32 jmisp_eng_close(JMISP_ENG_HANDLE *p_eng);
//extern void jmisp_eng_trig_single_hw_reg(JMISP_ENG_HANDLE *p_eng);
//extern void jmisp_eng_trig_ll_hw_reg(JMISP_ENG_HANDLE *p_eng, uintptr_t ll_addr_msb, uintptr_t ll_addr_lsb);
extern void jmisp_eng_trig_pl_hw_reg(JMISP_ENG_HANDLE *p_eng, uintptr_t ll_addr_msb, uintptr_t ll_addr_lsb, UINT32 pl_id);
extern INT32 jmisp_eng_init_resource(JMISP_ENG_HANDLE *p_eng);

extern UINT32 jmisp_eng_get_reg_base_buf_size(UINT32 eng_id);
extern UINT32 jmisp_eng_get_reg_flag_buf_size(UINT32 eng_id);

extern VOID jmisp_eng_set_reg_buf(JMISP_ENG_HANDLE *p_eng, uintptr_t reg_base_addr, uintptr_t reg_flag_addr);
extern VOID jmisp_eng_dma_channel_enable_hw_reg(JMISP_ENG_HANDLE *p_eng, BOOL set_en);
extern VOID jmisp_eng_axi_channel_enable_hw_reg(JMISP_ENG_HANDLE *p_eng, BOOL set_en);
extern BOOL jmisp_ssdrv_proc_ver(void);
extern INT32 jmisp_eng_set_intrpt_en(JMISP_ENG_HANDLE *p_eng, UINT32 int_en, UINT32 pl_id);
extern void jmisp_eng_isr_hw_reg(JMISP_ENG_HANDLE *p_eng);

extern INT32 jmisp_eng_set_single_dbg_en(JMISP_ENG_HANDLE *p_eng, UINT32 enable, UINT32 pl_id);
extern INT32 jmisp_eng_run_single_dbg(JMISP_ENG_HANDLE *p_eng, UINT32 pl_id);
extern INT32 jmisp_eng_set_arb_dbg_en(JMISP_ENG_HANDLE *p_eng, UINT32 enable_eng);
extern INT32 jmisp_eng_run_arb_dbg(JMISP_ENG_HANDLE *p_eng, UINT32 enable_eng);

extern INT32 jmisp_eng_run_cpu_slice(JMISP_ENG_HANDLE *p_eng, UINT32 path_id);

extern void jmisp_eng_wait_framend(JMISP_ENG_HANDLE *p_eng, UINT32 pl_id);

extern INT32 jmisp_eng_set_path_info(JMISP_ENG_HANDLE *p_eng, JMISP_ENG_PATH_INFO* path_info);

extern INT32 jmisp_eng_get_path_info(JMISP_ENG_HANDLE *p_eng, JMISP_ENG_PATH_INFO* path_info);
extern INT32 jmisp_eng_set_frame_addr(JMISP_ENG_HANDLE *p_eng, JMISP_ENG_FRAME_ADDR_INFO* frame_info);
extern INT32 jmisp_eng_set_frm_upd_path_info(JMISP_ENG_HANDLE *p_eng, JMISP_ENG_FRM_UPD_PATH_INFO* path_info);
extern INT32 jmisp_eng_get_frm_upd_path_info(JMISP_ENG_HANDLE *p_eng, JMISP_ENG_FRM_UPD_PATH_INFO* path_info);
extern INT32 jmisp_eng_reset(JMISP_ENG_HANDLE *p_eng);
extern UINT32 jmisp_eng_get_joblist_cycle(JMISP_ENG_HANDLE *p_eng, UINT32 pl_id);
extern UINT32 jmisp_eng_get_wait_dma_cycle(JMISP_ENG_HANDLE *p_eng, UINT32 pl_id);
extern UINT32 jmisp_eng_get_wait_unit_cycle(JMISP_ENG_HANDLE *p_eng, UINT32 pl_id);
extern BOOL jmisp_eng_chk_dma_channel_idle_reg(JMISP_ENG_HANDLE *p_eng);
extern UINT32 jmisp_eng_get_arb_status(JMISP_ENG_HANDLE *p_eng);
extern UINT32 jmisp_eng_get_eng_arb_info(JMISP_ENG_HANDLE *p_eng, UINT32 eng, UINT32 *pl_idx, UINT32 *dispatch_sel, UINT32 *net_id, UINT64 *job_addr);
extern UINT32 jmisp_eng_get_clk_rate(JMISP_ENG_HANDLE *p_eng);

#endif //_JMISP_ENG_H_
