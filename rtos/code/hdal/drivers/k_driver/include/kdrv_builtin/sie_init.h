/*
    Internal header file for SIE module.

    @file       sie_init.h
    @ingroup    mIIPPSIE

    Copyright   Novatek Microelectronics Corp. 2010.  All rights reserved.
*/

#ifndef _SIE_INIT_H
#define _SIE_INIT_H
#if defined(__KERNEL__)
#include <linux/slab.h>
#endif
#include "kwrap/type.h"
#include "kwrap/perf.h"

typedef enum {
	SIE_FB_ID_1 = 0,
	SIE_FB_ID_2,
	SIE_FB_ID_3,
	SIE_FB_ID_4,
	SIE_FB_ID_5,
	SIE_FB_ID_MAX,
	ENUM_DUMMY4WORD(SIE_FB_ID)
} SIE_FB_ID;

typedef struct _SIE_FB_BUF_ADDR_INFO_ {
	ULONG va;
	ULONG pa;
} SIE_FB_BUF_ADDR_INFO;

typedef enum {
	SIE_FB_OUT_BASE = 0,	//base buf for internal control using
	SIE_FB_OUT_CH0,			//out ch0
	SIE_FB_OUT_CH1,
	SIE_FB_OUT_CH2,
	SIE_FB_OUT_CH_MAX,
	ENUM_DUMMY4WORD(SIE_FB_OUT_CH_IDX)
} SIE_FB_OUT_CH_IDX;

typedef struct _SIE_FB_BRIDGE_INFO_ {
	UINT32 frame_cnt;
	ULONG buf_addr_0;
	ULONG buf_addr_1;
	VOS_TICK timestamp;		///< VD timestamp
} SIE_FB_BRIDGE_INFO;

typedef enum {
	SIE_FB_BUF_IDX_CUR_OUT = 0,
	SIE_FB_BUF_IDX_RDY,
	SIE_FB_BUF_IDX_MAX,
	ENUM_DUMMY4WORD(SIE_FB_BUF_IDX)
} SIE_FB_BUF_IDX;

typedef struct {
	UINT32 sie_id_bit;				// fastboot total sie config id (for rtos and linux builtin) (main id, not include duplicate id)
	UINT32 rtos_streaming_id_bit;	// rtos streaming bit, linux builtin will skip config sie
	/* private buffer info */
	ULONG ring_buf_blk_addr;	//for SIE2 ring buffer using
	UINT32 ring_buf_blk_size;	//for SIE2 ring buffer using
} SIE_BUILTIN_INIT_INFO;

/*
	read from fastboot dtsi
	for debug function enable
*/
typedef struct {
	BOOL   out_two_frame_only;
	BOOL   isp_bypass_en;
	BOOL   push_ts_print;				// print sie vd/push out timestamp
} SIE_BUILTIN_DBG_INFO;

typedef enum {
	KDRV_SIE_BUILTIN_FUNC_ALGO_BY_PASS   =  0x00000001,
	KDRV_SIE_BUILTIN_FUNC_CSI_PAT_GEN    =  0x00000002,
	KDRV_SIE_BUILTIN_FUNC_OUT_TWO_FRAME  =  0x00000004,
	KDRV_SIE_BUILTIN_PUSH_TS  			 =  0x00000008,
} KDRV_SIE_BUILTIN_DBG_FUNC;

#define SIE_BUILTIN_HEADER_CTL_LOCK 	0x1			// push ready buffer for ipp, ipp cannot release buffer
#define SIE_BUILTIN_HEADER_CTL_PUSH 	0x2 		// push ready buffer for ipp, ipp need to release buffer
#define SIE_BUILTIN_HEADER_CTL_UNLOCK 	0x3 		// unlock buffer


typedef struct {
	UINT32 buf_ctrl;
	ULONG  buf_addr;               	///< buffer start virtual address
	ULONG  buf_addr_pa;            	///< buffer start physical address
	UINT32 count;					///< fastboot VD frame count
	VOS_TICK timestamp;				///< VD timestamp
	ULONG addr_ch0;               	///< dramout_ch0 virutal address(raw or ccir y)
	ULONG addr_ch0_pa;            	///< dramout_ch0 physical address(raw or ccir y)
	ULONG addr_ch1;               	///< dramout_ch1 virutal address(ccir uv)
	ULONG addr_ch1_pa;            	///< dramout_ch1 physical addresS(ccir uv)
	// for shdr + dram mode long frame using
	ULONG frm2_buf_addr;		///< buffer start address
	ULONG frm2_buf_addr_pa;    	///< buffer start physical address
	ULONG frm2_addr_ch0;		///< ch0(raw or ccir y) output addr
	ULONG frm2_addr_ch0_pa;		///< ch0(raw or ccir y) output addr
	USIZE  out_size;
} SIE_BUILTIN_HEADER_INFO;

typedef struct {
	URECT act_win;
	URECT crp_win;
} SIE_BUILTIN_IOSIZE;

typedef void (*SIE_FB_KDRV_ISR_FP)(void *eng, UINT32 status, void *reserve);
typedef void (*SIE_FB_ISR_FP)(UINT32 id, UINT32 status); //status ref. to SIE_INT_VD
typedef void (*SIE_FB_BUF_OUT_FP)(UINT32 id, SIE_BUILTIN_HEADER_INFO *info);
extern ER sie_builtin_init(SIE_BUILTIN_INIT_INFO *info);	//set sie fastboot streaming
extern ER sie_builtin_set_start(UINT32 id);
extern ER sie_builtin_set_stop(UINT32 id);
extern ER sie_builtin_rst(UINT32 id_bit);
extern ER sie_fb_reg_kdrv_isr_Cb(UINT32 id, SIE_FB_KDRV_ISR_FP fp);	//register isr callback function
extern ER sie_fb_reg_isr_Cb(SIE_FB_ISR_FP fp);	//register isr callback function
extern ER sie_fb_reg_buf_out_cb(SIE_FB_BUF_OUT_FP fp);	//register buffer push out callback function
extern void sie_fb_get_rdy_addr(UINT32 id, ULONG *addr_ch0, ULONG *addr_ch1, ULONG *addr_ch2);	//get sie output ready address
extern void sie_fb_buf_ctrl(UINT32 id, BOOL int_cb_trig, UINT32 status,SIE_FB_BUF_IDX buf_idx, UINT32 buf_io_ctl);
extern void sie_fb_upd_timestp(UINT32 id);
extern ER sie_fb_set_iosize(UINT32 id, SIE_BUILTIN_IOSIZE iosize);
extern UINT32 sie_set_dbg_func(UINT32 id, UINT32 dbg_func_en, BOOL en);
extern void sie_fb_wait_vd(UINT32 id);
extern void sie_fb_get_dbg_info(SIE_BUILTIN_DBG_INFO *dbg_info);

//get api
extern ER sie_fb_get_clk(UINT32 id, UINT32 *clk_src, UINT32 *clk_rate);
extern USIZE sie_fb_get_out_size(UINT32 id);
extern void sie_fb_get_bridge_info(UINT32 id, SIE_FB_BRIDGE_INFO *brg_info);

#endif// _SIE_INIT_H
