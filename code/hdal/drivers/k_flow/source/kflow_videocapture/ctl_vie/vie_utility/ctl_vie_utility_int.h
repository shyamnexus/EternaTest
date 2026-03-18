/**
       ctrl VIE utility

    .

    @file       ctl_vie_utility_int.h
    @ingroup    mISYSAlg
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#ifndef _CTL_VIE_UTILITY_INT_H_
#define _CTL_VIE_UTILITY_INT_H_

#if defined (__LINUX)
#include <linux/string.h>
#include <linux/of.h>
#include <linux/slab.h>
#elif defined (__FREERTOS)
#include <string.h>
#include <malloc.h>
#endif

#include "kwrap/cpu.h"
#include "kwrap/mem.h"
#include "kwrap/util.h"
#include "kwrap/flag.h"
#include "kwrap/list.h"
#include "comm/hwclock.h"
#include <kdrv_builtin/kdrv_builtin.h>
#include "ctl_vie_dbg.h"
#include "ctl_vie_int.h"
#include "ctl_vie_utility.h"
#include <nvt_api_ver.h>

#define CTL_VIE_MAX_SUPPORT_ID      (KDRV_VIE_MAX_ENG * KDRV_VIE_MAX_VDO_CH)    //reference max support id from kdrv
#define CTL_VIE_FORCE_REG_DBG_CB    ENABLE              //disable when remove ctl_vie_debug_int.c source code

typedef struct vos_list_head CTL_VIE_TEST_LIST_HEAD;

/**
    CCIR YUV order
    must correspond to KDRV_VIE_YUV_ORDER
*/
typedef enum {
	CTL_VIE_YUYV = 0,
	CTL_VIE_YVYU,
	CTL_VIE_UYVY,
	CTL_VIE_VYUY,
	CTL_VIE_YYU_YYV,
	CTL_VIE_YUY_YVY,
	CTL_VIE_UYY_VYY,
	ENUM_DUMMY4WORD(CTL_VIE_YUV_ORDER)
} CTL_VIE_YUV_ORDER;

/**
    CCIR FORMAT
    must correspond to KDRV_VIE_DVI_FORMAT_SEL
    sensor drv bt601   --> vie bt601
    sensor drv bt656   --> vie bt656
    sensor drv bt709   --> vie bt601
    sensor drv bt1120  --> vie bt1120

    N.S. CCIR65_ACT
*/
typedef enum {
	CTL_DVI_FMT_CCIR601  = 0,    			// CCIR 601
	CTL_DVI_FMT_CCIR656_EAV,     			// CCIR 656 EAV
	CTL_DVI_FMT_CCIR656_EAV_DUAL_HEADER,	// CCIR 656 EAV with dual header
	CTL_DVI_FMT_LEGACY_YUV420,
	ENUM_DUMMY4WORD(CTL_VIE_DVI_FORMAT_SEL)
} CTL_VIE_DVI_FORMAT_SEL;

/**
    CCIR MODE
    must correspond to KDRV_VIE_DVI_IN_MODE_SEL
*/
typedef enum {
	CTL_DVI_MODE_SD  = 0,   ///< SD mode (8 bits)
	CTL_DVI_MODE_HD,        ///< HD mode (16bits)
	ENUM_DUMMY4WORD(CTL_VIE_DVI_IN_MODE_SEL)
} CTL_VIE_DVI_IN_MODE_SEL;

/**
    CCIR internal info
    must correspond to KDRV_VIE_CCIR_INFO
*/
typedef struct {
	CTL_VIE_YUV_ORDER yuv_order;
	CTL_VIE_DVI_FORMAT_SEL fmt;
	CTL_VIE_DVI_IN_MODE_SEL dvi_mode;
	BOOL b_filed_en;                ///< (HDAL)for CCIR interlaced
	BOOL b_filed_sel;               ///< (HDAL)select data for CCIR interlaced
	BOOL b_ccir656_vd_sel;          ///< (SenDrv)656 vd mode, 0 --> interlaced(field change as VD), 1 --> progressive(SAV + V Blank status)
	UINT8 data_period;              ///< (SenDrv)for mux sensor, 0 --> 1 input(YUYV), 1 --> 2 input(YYUUYYVV)
	UINT8 data_idx;                 ///< (HDAL)select data idx 0/1 when mux sensor input
} CTL_VIE_CCIR_INFO_INT;

/**
    vie signal receive mode
*/
typedef enum {
	CTL_VIE_IN_PARA_MSTR_SNR = 0,   ///< Parallel Master Sensor
	CTL_VIE_IN_PATGEN,              ///< Self Pattern-Generator
	CTL_VIE_IN_CSI_1,               ///< Serial Sensor from CSI-1
	CTL_VIE_IN_CSI_2,               ///< Serial Sensor from CSI-2
	CTL_VIE_IN_CSI_3,               ///< Serial Sensor from CSI-3
	CTL_VIE_IN_CSI_4,               ///< Serial Sensor from CSI-4
	CTL_VIE_IN_CSI_5,               ///< Serial Sensor from CSI-5
	CTL_VIE_IN_CSI_6,               ///< Serial Sensor from CSI-6
	CTL_VIE_IN_CSI_7,               ///< Serial Sensor from CSI-7
	CTL_VIE_IN_CSI_8,               ///< Serial Sensor from CSI-8
	ENUM_DUMMY4WORD(CTL_VIE_ACT_MODE)
} CTL_VIE_ACT_MODE;

typedef struct {
	UINT32 data_rate;
	CTL_VIE_CLKSRC_SEL clk_src_sel;
	CTL_VIE_ACT_MODE act_mode;
	UINT32 drv_data_rate;
} CTL_VIE_CLK_INFO;

typedef struct {
	CTL_VIE_PATGEN_SEL  pat_gen_mode;
	UINT32              pat_gen_val;
	USIZE               pat_gen_src_win;
	UINT32              frame_rate;         // fps * 100
} CTL_VIE_PATGEN_PAR;

typedef struct {
	BOOL single_out_en[CTL_VIE_DRAM_CH_MAX];
} CTL_VIE_SINGLE_OUT_CTRL;

typedef struct {
	struct vos_mem_info_t cma_info;
	VOS_MEM_HDL cma_hdl;
} CTL_VIE_VOS_MEM_INFO;

typedef enum {
	CTL_VIE_NORMAL_OUT = 0,
	CTL_VIE_SINGLE_OUT,
	ENUM_DUMMY4WORD(CTL_VIE_OUT_MODE_TYPE)
} CTL_VIE_OUT_MODE_TYPE;

typedef struct {
	CTL_VIE_OUT_MODE_TYPE  out_mode[CTL_VIE_DRAM_CH_MAX];
} CTL_VIE_DRAM_OUT_CTRL;

typedef enum {
	CTL_VIE_VD_DLY_NONE,
	CTL_VIE_VD_DLY_AUTO,

	ENUM_DUMMY4WORD(CTL_VIE_VD_DLY)
} CTL_VIE_VD_DLY;


/**
    CTL_VIE_SYS_INFO
    vd_cnt_clk / vd_cnt_clk_base = vd time (s)
    hd_cnt_clk / hd_cnt_clk_base = hd time (s)
*/
typedef struct {
	BOOL pxclk_a_in;          	// check pixel clock in,  for parallel sensor debug only
	BOOL pxclk_b_in;          	// check pixel clock in,  for parallel sensor debug only
	BOOL vieclk_in;         	// check vie module clock in

	/* vd */
	UINT32 vd_cnt_clk_base;     // units : Hz
	UINT32 vd_cnt_clk;          // units : vd_cnt_clk_base (apb clk)
	UINT32 vd_cnt_pxl;          // units : pixel

	/* hd */
	UINT32 hd_cnt_clk_base;     // units : Hz
	UINT32 hd_cnt_clk;          // units : hd_cnt_clk_base (apb clk)
	UINT32 hd_cnt_line;         // units : line

} CTL_VIE_SYS_INFO;


typedef struct {
	ULONG dbg_buf_addr_va; // dbg dramout virtual address
	ULONG dbg_buf_addr_pa; // dbg dramout physical address

	URECT crp_win;
	CTL_VIE_FLIP_TYPE flip;

} CTL_VIE_DRAMOUT_DBG_INFO;

typedef enum {
	CTL_VIE_INT_ITEM_INTE = CTL_VIE_ITEM_MAX + 1,       ///< [Set    ] data_type: UINT32, set interrupte enable
	CTL_VIE_INT_ITEM_BP3,                               ///< [Set/Get] data_type: UINT32, set 0 to disable bp3
	CTL_VIE_INT_ITEM_CH0_ADDR,                          ///< [Set/Get] data_type: UINT32, need 4 byte align
	CTL_VIE_INT_ITEM_CH1_ADDR,                          ///< [Set/Get] data_type: UINT32, need 4 byte align
	CTL_VIE_INT_ITEM_CHDBG_ADDR,                        ///< [Set/Get] data_type: UINT32, need 4 byte align
	CTL_VIE_INT_ITEM_CLK,                               ///< [    Get] data_type: UINT32
	CTL_VIE_INT_ITEM_LIMIT,                             ///< [    Get] data_type: CTL_VIE_LIMIT
	CTL_VIE_INT_ITEM_CH_OUT_MODE,                       ///< [Set    ] data_type: CTL_VIE_OUTPUT_MODE_TYPE
	CTL_VIE_INT_ITEM_SINGLE_OUT_CTL,                    ///< [Set/Get] data_type: CTL_VIE_SINGLE_OUT_CTRL,
	CTL_VIE_INT_ITEM_SYS_INFO,                          ///< [    Get] data_type: CTL_VIE_SYS_INFO
	CTL_VIE_INT_ITEM_CCIR_H_STS,                        ///< [    Get] data_type: UINT32
	CTL_VIE_INT_ITEM_DRAMOUT_DBG,                       ///< [Set    ] data_type: BOOL
	CTL_VIE_INT_ITEM_MAX,
	CTL_VIE_INT_ITEM_REV = 0x8000000000000000,
	ENUM_DUMMY4WORD(CTL_VIE_INT_ITEM)
} CTL_VIE_INT_ITEM;
STATIC_ASSERT((CTL_VIE_INT_ITEM_MAX &CTL_VIE_INT_ITEM_REV) == 0);
#define CTL_VIE_IGN_CHK CTL_VIE_INT_ITEM_REV    //only support set/get function

typedef struct {
	CTL_VIE_DATAFORMAT      data_fmt;           ///< vie data format
	CTL_VIE_CCIR_INFO_INT   ccir_info_int;      ///< ccir internal information
	CTL_VIE_IO_SIZE_INFO    io_size_info;       ///< vie io size info
	CTL_VIE_FLIP_TYPE       flip;               ///< flip type
	CTL_VIE_CHGSENMODE_INFO chg_senmode_info;
	BOOL                    dma_abort;
	CTL_VIE_MASK            mask;
	CTL_VIE_CROP_MAX		crop_max;
	BOOL					four_k_mode;
} CTL_VIE_ITEM_PARAM;

typedef struct {
	UINT32 out_ch_lof[CTL_VIE_DRAM_CH_MAX];          ///< line offset of each output channel
	CTL_VIE_PATGEN_PAR  pat_gen_param;          ///< pattern gen parameters
	CTL_VIE_SIGNAL      signal;                 ///< signal info
	UINT32              total_frame_num;        // multi frame info
} CTL_VIE_RTC_CTRL_OBJ;

// internal ctrl
typedef struct {
	CTL_VIE_ISRCB           int_isrcb_fp;   ///< internal isr cb function pointer
	CTL_VIE_CB_INFO         ext_isrcb_fp;   ///< external isr cb function pointer
	CTL_VIE_CB_INFO         bufiocb;        ///< buf in/out isr cb function pointer
	UINT64                  update_item;    ///< 1<< update_item from CTL_VIE_ITEM or CTL_VIE_INT_ITEM
	CTL_VIE_INTE            inte;           ///< interrupt enable
	ULONG                   out_ch_addr_va[CTL_VIE_DRAM_CH_MAX]; ///< virtual address of each output channel
	ULONG                   out_ch_addr_pa[CTL_VIE_DRAM_CH_MAX]; ///< physical address of each output channel
	CTL_VIE_HEADER_INFO     head_info[CTL_VIE_HEAD_IDX_MAX];    ///< vie push out header address
	CTL_VIE_FRM_CTL_INFO    frame_ctl_info;
	CTL_VIE_TRIG_INFO       trig_info;
	CTL_VIE_CLK_INFO        clk_info;
	CTL_VIE_BUF_INFO        buf_info;
	CTL_VIE_RST_FC_STATUS   rst_fc_sts;
	UINT32                  row_time;
	UINT32                  bp3;
	CTL_VIE_DRAM_OUT_CTRL   ch_out_mode;
	CTL_VIE_SINGLE_OUT_CTRL sin_out_ctl;
	UINT32                  sen_dft_fps;        ///< (fps * 100)
	BOOL 						dbg_en; 						// enable check fw & hw debug data dramout debug enable
	CTL_VIE_DRAMOUT_DBG_INFO    dbg_info[CTL_VIE_HEAD_IDX_MAX]; // fw debug data, check with hw dramout debug info
	atomic_t 					dbg_log_cnt[CTL_VIE_DRAMOUT_DBG_ITEM_MAX];

	//isp ctrl
	BOOL                    alg_func_en[CTL_VIE_ALG_TYPE_MAX];  ///< algorithm id(iq, ae, awb, af...) */

	//runtime change obj
	CTL_VIE_RTC_CTRL_OBJ    rtc_obj;        ///< runtime change obj
} CTL_VIE_CTRL_OBJ;

typedef struct {
	UINT32                  tag;
	ULONG                   kdf_hdl;
	ULONG                   evt_hdl;

	/* CTL_VIE_OPEN_CFG */
	CTL_VIE_ID              id;
	CTL_VIE_ID              dupl_src_id;
	UINT32                  sen_id;
	ISP_ID                  isp_id;
	CTL_VIE_FLOW_TYPE       flow_type;

	//
	BOOL dvi_interlace_skip_en;

	/* CTL_VIE_ITEM */
	CTL_VIE_ITEM_PARAM      param;
	/* internal ctrl parameters */
	CTL_VIE_CTRL_OBJ        ctrl;

	/* runtime change CTL_VIE_ITEM */
	CTL_VIE_ITEM_PARAM      rtc_param;
	/* runtime change internal ctrl parameters */
	CTL_VIE_RTC_CTRL_OBJ    rtc_ctrl;

	/* ready for load CTL_VIE_ITEM */
	CTL_VIE_ITEM_PARAM      load_param;
	/* ready for load internal ctrl parameters */
	CTL_VIE_RTC_CTRL_OBJ    load_ctrl;
} CTL_VIE_HDL;

typedef struct {
	UINT64 set_load_flg;
	UINT32 get_cnt_flg;
	UINT32 cfg_start_bp;
	UINT32 last_isr_sts;
	BOOL isr_dram_end_flg;
	BOOL isr_valid_vd_flg;      //vd proc will set TRUE, bp3 proc will clear to FALSE
	BOOL bp3_rst_en;
	BOOL isr_bp3_rst_flg;
	BOOL isr_load_en_flg;
	BOOL resume_rst_flg;        //reset ctrl info when resume
	BOOL interlace_skip_flg;
} CTL_VIE_INT_CTL_FLG;

typedef struct {
	UINT32 context_num;
	UINT32 context_used;    //current used context number
	UINT32 context_idx[CTL_VIE_MAX_SUPPORT_ID];
	UINT32 contex_size;;    //each context buf size
	ULONG  start_addr;
	UINT32 req_size;        //total ctl_vie ctx require size (context_num*contex_size)
	UINT32 total_buf_sz;    //total ctx require size (ctl_vie+kdf_vie+kdrv_vie)s
	BOOL dbg_buf_alloc;
	BOOL auto_alloc_mem;
	CTL_VIE_VOS_MEM_INFO vos_mem_info;  //private memory, auto allocate memory when init address is 0
} CTL_VIE_HDL_CONTEXT;

#define CTL_VIE_GET_ID(hdl) ctl_vie_hdl_conv2_id(hdl)

#define CTL_VIE_DIV_U64(x, y) ctl_vie_uint64_dividend(x, y)
#define CTL_VIE_MAX(x, y) ctl_vie_max(x, y)
#define CTL_VIE_MIN(x, y) ctl_vie_min(x, y)
#define CTL_VIE_LCM(x, y) ctl_vie_lcm(x, y)
#define CTL_VIE_CHK_ALIGN(src, align) ctl_vie_chk_align(src, align)

extern BOOL ctl_vie_module_chk_id_valid(CTL_VIE_ID id);
extern CTL_VIE_ID ctl_vie_hdl_conv2_id(ULONG hdl);
extern UINT32 ctl_vie_uint64_dividend(UINT64 dividend, UINT32 divisor);
extern UINT32 ctl_vie_max(UINT32 x, UINT32 y);
extern UINT32 ctl_vie_min(UINT32 x, UINT32 y);
extern UINT32 ctl_vie_lcm(UINT32 x, UINT32 y);
extern BOOL ctl_vie_chk_align(UINT32 x, UINT32 y);
extern UINT32 ctl_vie_get_hdl_tag(void);
extern CTL_VIE_HDL *ctl_vie_get_hdl(CTL_VIE_ID id);
extern CTL_VIE_HDL_CONTEXT *ctl_vie_get_ctx(void);
extern CTL_VIE_LIMIT *ctl_vie_limit_query(CTL_VIE_ID id);
extern CTL_VIE_STATUS ctl_vie_get_state_machine(CTL_VIE_ID id);
extern INT32 ctl_vie_get_sen_cfg(CTL_VIE_ID id, CTL_SEN_CFG cfg, void *data);
extern UINT64 ctl_vie_util_get_syst_timestamp(void);
extern UINT32 ctl_vie_util_get_timestamp(void);
extern INT32 ctl_vie_util_os_malloc(CTL_VIE_VOS_MEM_INFO *vod_mem_info, UINT32 req_size);
extern INT32 ctl_vie_util_os_mfree(CTL_VIE_VOS_MEM_INFO *vod_mem_info);
extern void *ctl_vie_util_os_malloc_wrap(UINT32 want_size);
extern void ctl_vie_util_os_mfree_wrap(void *p_buf);
extern void ctl_vie_chk_hw_crop_w(CTL_VIE_ID id, UINT32 crop_w, BOOL *hw_spt, BOOL *four_k_mode);
extern UINT32 (*ctl_vie_cert_func)(UINT32, UINT32, UINT32);

/*
	debug api
*/
typedef enum {
	CTL_VIE_DBG_TS_VD = 0,
	CTL_VIE_DBG_TS_BP3,
	CTL_VIE_DBG_TS_ADDR_RDY,
	CTL_VIE_DBG_TS_DRAMEND,
	CTL_VIE_DBG_TS_RCVMSG,
	CTL_VIE_DBG_TS_UNLOCK,
	CTL_VIE_DBG_TS_CRPST,
	CTL_VIE_DBG_TS_CRPEND,
	CTL_VIE_DBG_TS_MAX
} CTL_VIE_DBG_TS_EVT;

typedef struct {
	BOOL(*chk_msg_type)(CTL_VIE_ID id, UINT32 chk_type);
	UINT32(*query_dbg_buf)(void);
	void(*set_dbg_buf)(UINT32 id, ULONG buf_addr);
	void(*set_ts)(CTL_VIE_ID id, CTL_VIE_DBG_TS_EVT evt, UINT64 vie_fc);    //set timestamp record
	void(*reset_ts)(CTL_VIE_ID id);                                         //reset timestamp
	void(*dump_buf_io)(CTL_VIE_ID id, CTL_VIE_BUF_IO_CFG buf_io, UINT32 total_size, ULONG header_addr);    //dump vie buffer in/out info
	void(*set_isr_ioctl)(CTL_VIE_ID id, UINT32 status, UINT32 buf_ctl_type, CTL_VIE_HEAD_IDX head_idx);     //set isr control info
	void(*set_frm_ctrl_info)(CTL_VIE_ID id, CTL_VIE_FRM_CTL_INFO *frm_ctrl);                                //set frame control info
	void(*set_isp_cb_t_log)(ISP_ID id, ISP_EVENT evt, UINT64 fc, UINT32 ts_start, UINT32 ts_end);           //set isp callback timestamp info
	void(*isp_cb_t_dump)(int (*dump)(const char *fmt, ...)); //isp callback timestamp dump
	void(*set_proc_t)(CTL_VIE_ID id, CHAR *proc_name, CTL_VIE_PROC_TIME_ITEM item); //set timestamp record
	void(*upd_sysdbg_info)(CTL_VIE_ID id, UINT32 evt, UINT64 vie_fc);   // update system information
	void(*get_buf_wp_info)(CTL_VIE_ID id, UINT32 *ddr_id, UINT32 *dis_wp);          //get vie out ddr id, and release_before_push_en
} CTL_VIE_INT_DBG_TAB;

extern CTL_VIE_INT_DBG_TAB *ctl_vie_get_dbg_tab(void);
extern void ctl_vie_reg_dbg_tab(CTL_VIE_INT_DBG_TAB *dbg_tab);
extern void ctl_vie_set_dbg_lvl(CTL_VIE_DBG_LVL dbg_lvl);


extern KDRV_DEV_ENGINE __conv2_vie_kdrv_eng(CTL_VIE_ID id);
extern KDRV_VIE_VDO_CH __conv2_vie_kdrv_ch(CTL_VIE_ID id);
extern KDRV_VIE_PROC_ID __conv2_vie_kdrv_id(CTL_VIE_ID id);

extern CTL_VIE_LIMIT ctl_vie_limit[CTL_VIE_MAX_SUPPORT_ID];

#endif //_CTL_VIE_UTILITY_INT_H_

