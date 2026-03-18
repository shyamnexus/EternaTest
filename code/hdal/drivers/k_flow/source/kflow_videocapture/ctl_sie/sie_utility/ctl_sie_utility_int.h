/**
       ctrl SIE utility

    .

    @file       ctl_sie_utility_int.h
    @ingroup    mISYSAlg
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#ifndef _CTL_SIE_UTILITY_INT_H_
#define _CTL_SIE_UTILITY_INT_H_

#if defined (__LINUX)
#include <linux/string.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/soc/nvt/nvt-timer.h>
#include <linux/clk.h>
#elif defined (__FREERTOS)
#include <string.h>
#include <malloc.h>
#include <plat/hrtimer.h>
#endif

#include <plat/top.h>
#include "kwrap/cpu.h"
#include "kwrap/mem.h"
#include "kwrap/util.h"
#include "kwrap/file.h"
#include "kwrap/flag.h"
#include "kwrap/list.h"
#include "ctl_sie_dbg.h"
#include "ctl_sie_int.h"
#include "ctl_sie_utility.h"
#include "comm/hwclock.h"
#include <kdrv_builtin/kdrv_builtin.h>
#include <nvt_api_ver.h>

#define CTL_SIE_MAX_SUPPORT_ID      KDRV_SIE_MAX_ENG    //reference max support id from kdrv
#define CTL_SIE_FORCE_REG_DBG_CB    ENABLE              //disable when remove ctl_sie_debug_int.c source code

typedef struct vos_list_head CTL_SIE_TEST_LIST_HEAD;

/**
    type for  CTL_SIE_MCLK
    must correspond to KDRV_SIE_MCLK_INFO
*/
typedef struct {
	BOOL                    clk_en;
	CTL_SIE_MCLK_SRC_SEL    mclk_src_sel;
	UINT32                  clk_rate;
	CTL_SIE_MCLK_ID         mclk_id_sel;
} CTL_SIE_MCLK_INFO;

/**
    CCIR YUV order
    must correspond to KDRV_SIE_YUV_ORDER
*/
typedef enum {
	CTL_SIE_YUYV = 0,
	CTL_SIE_YVYU,
	CTL_SIE_UYVY,
	CTL_SIE_VYUY,
	CTL_SIE_YYU_YYV,
	CTL_SIE_YUY_YVY,
	CTL_SIE_UYY_VYY,
	ENUM_DUMMY4WORD(CTL_SIE_YUV_ORDER)
} CTL_SIE_YUV_ORDER;

/**
    CCIR FORMAT
    must correspond to DAL_SIE_DVI_FORMAT_SEL
    sensor drv bt601 + sd   --> sie bt601 + sd
    sensor drv bt656 + sd   --> sie bt656 + sd
    sensor drv bt709 + hd   --> sie bt601 + hd
    sensor drv bt1120 + hd  --> sie bt656 + hd
*/
typedef enum {
	CTL_DVI_FMT_CCIR601  = 0,    ///< CCIR 601
	CTL_DVI_FMT_CCIR656_EAV,     ///< CCIR 656 EAV
	CTL_DVI_FMT_CCIR656_ACT,     ///< CCIR 656 ACT
	CTL_DVI_FMT_LEGACY_YUV420,
	ENUM_DUMMY4WORD(CTL_SIE_DVI_FORMAT_SEL)
} CTL_SIE_DVI_FORMAT_SEL;

/**
    CCIR MODE
    must correspond to DAL_SIE_DVI_IN_MODE_SEL
*/
typedef enum {
	CTL_DVI_MODE_SD  = 0,   ///< SD mode (8 bits)
	CTL_DVI_MODE_HD,        ///< HD mode (16bits)
	CTL_DVI_MODE_HD_INV,    ///< HD mode (16bits) with Byte Inverse
	ENUM_DUMMY4WORD(CTL_SIE_DVI_IN_MODE_SEL)
} CTL_SIE_DVI_IN_MODE_SEL;

/**
    CCIR internal info
    must correspond to DAL_SIE_CCIR_INFO
*/
typedef struct {
	CTL_SIE_YUV_ORDER yuv_order;
	CTL_SIE_DVI_FORMAT_SEL fmt;
	CTL_SIE_DVI_IN_MODE_SEL dvi_mode;
	BOOL b_filed_en;                ///< (HDAL)for CCIR interlaced
	BOOL b_filed_sel;               ///< (HDAL)select data for CCIR interlaced
	BOOL b_ccir656_vd_sel;          ///< (SenDrv)656 vd mode, 0 --> interlaced(field change as VD), 1 --> progressive(SAV + V Blank status)
#if 0
	BOOL b_auto_align;              ///< (Kdrv)CCIR656 mux sensor data auto alignment
	UINT8 data_period;              ///< (SenDrv)for mux sensor, 0 --> 1 input(YUYV), 1 --> 2 input(YYUUYYVV)
	UINT8 data_idx;                 ///< (HDAL)select data idx 0/1 when mux sensor input
#endif
} CTL_SIE_CCIR_INFO_INT;

/**
    sie signal receive mode
*/
typedef enum {
	CTL_SIE_IN_PARA_MSTR_SNR = 0,   ///< Parallel Master Sensor
	CTL_SIE_IN_PARA_SLAV_SNR,       ///< Parallel Slave Sensor
	CTL_SIE_IN_PATGEN,              ///< Self Pattern-Generator
	CTL_SIE_IN_VX1_IF0_SNR,         ///< Vx1 Sensor
	CTL_SIE_IN_CSI_1,               ///< Serial Sensor from CSI-1
	CTL_SIE_IN_CSI_2,               ///< Serial Sensor from CSI-2
	CTL_SIE_IN_CSI_3,               ///< Serial Sensor from CSI-3
	CTL_SIE_IN_CSI_4,               ///< Serial Sensor from CSI-4
	CTL_SIE_IN_CSI_5,               ///< Serial Sensor from CSI-5
	CTL_SIE_IN_CSI_6,               ///< Serial Sensor from CSI-6
	CTL_SIE_IN_CSI_7,               ///< Serial Sensor from CSI-7
	CTL_SIE_IN_CSI_8,               ///< Serial Sensor from CSI-8
	CTL_SIE_IN_2PPATGEN,			///< 2-pixel pattern gen mode
	CTL_SIE_IN_SLVS_EC,             ///< Serial Sensor from SLVS-EC
	CTL_SIE_IN_VX1_IF1_SNR,         ///< Vx1 Sensor, interface 1
	CTL_SIE_IN_PARA_THERMAL,   		///< Parallel thermal sensor
	ENUM_DUMMY4WORD(CTL_SIE_ACT_MODE)
} CTL_SIE_ACT_MODE;

/**
    sie pclk source select
    must correspond to KDRV_SIE_PXCLKSRC_SEL
*/
typedef enum {
	CTL_SIE_PXCLKSRC_OFF,       ///< SIE pixel-clock disable
	CTL_SIE_PXCLKSRC_PAD,       ///< SIE pixel-clock enabled, source as pixel-clock-pad
	CTL_SIE_PXCLKSRC_MCLK,      ///< SIE pixel-clock enabled, source as MCLK (SIE_PXCLK from SIE_MCLK, SIE2_PXCLK from SIE_MCLK2, SIE3_PXCLK from SIE_MCLK2, SIE4_PXCLK from SIE_MCLK2)
	///< Usually for Pattern Gen Only
	CTL_SIE_PXCLKSRC_VX1_1X,    ///< SIE pixel-clock enabled, source as Vx1 1x clock (only for SIE2/3)
	CTL_SIE_PXCLKSRC_VX1_2X,    ///< SIE pixel-clock enabled, source as Vx1 2x clock (only for SIE2/3)
	ENUM_DUMMY4WORD(CTL_SIE_PXCLKSRC_SEL)
} CTL_SIE_PXCLKSRC_SEL;

typedef struct {
	UINT32 data_rate;
	CTL_SIE_PXCLKSRC_SEL pclk_src_sel;
	CTL_SIE_CLKSRC_SEL clk_src_sel;
	CTL_SIE_ACT_MODE act_mode;
	UINT32 drv_data_rate;
} CTL_SIE_CLK_INFO;

typedef struct {
	CTL_SIE_PATGEN_SEL  pat_gen_mode;
	UINT32              pat_gen_val;
	USIZE               pat_gen_src_win;
	UINT32              frame_rate;         // fps * 100
	UINT32 				frame_num;
} CTL_SIE_PATGEN_PAR;

typedef struct {
	BOOL single_out_en[CTL_SIE_DRAM_CH_MAX];
} CTL_SIE_SINGLE_OUT_CTRL;

typedef struct {
	BOOL en;
	UINT32 buf_len;
	ULONG  buf_addr_va;
	ULONG  buf_addr_pa;
	UINT32 buf_size;
} CTL_SIE_RING_BUF_CTL;

#if 0
typedef enum {
	CTL_SIE_PP_BUF_IDX_EVEN = 0,
	CTL_SIE_PP_BUF_IDX_ODD,
	CTL_SIE_PP_BUF_IDX_MAX,
	ENUM_DUMMY4WORD(CTL_SIE_PP_BUF_IDX)
} CTL_SIE_PP_BUF_IDX;

typedef struct {
	BOOL en;
	ULONG  buf_addr_va[CTL_SIE_BUF_IDX_MAX];
	ULONG  buf_addr_pa[CTL_SIE_BUF_IDX_MAX];
	UINT32 buf_size;
} CTL_SIE_EVS_BUF_CTL;
#endif

typedef struct {
	struct vos_mem_info_t cma_info;
	VOS_MEM_HDL cma_hdl;
} CTL_SIE_VOS_MEM_INFO;

typedef enum {
	CTL_SIE_NORMAL_OUT = 0,
	CTL_SIE_SINGLE_OUT,
	ENUM_DUMMY4WORD(CTL_SIE_OUT_MODE_TYPE)
} CTL_SIE_OUT_MODE_TYPE;

typedef struct {
	CTL_SIE_OUT_MODE_TYPE  out_mode[CTL_SIE_DRAM_CH_MAX];
} CTL_SIE_DRAM_OUT_CTRL;

typedef enum {
	CTL_SIE_VD_DLY_NONE,
	CTL_SIE_VD_DLY_AUTO,

	ENUM_DUMMY4WORD(CTL_SIE_VD_DLY)
} CTL_SIE_VD_DLY;


/**
    CTL_SIE_SYS_INFO
    vd_cnt_clk / vd_cnt_clk_base = vd time (s)
    hd_cnt_clk / hd_cnt_clk_base = hd time (s)
*/
typedef struct {
	/* vd */
	UINT32 vd_cnt_clk_base;     // units : Hz
	UINT32 vd_cnt_clk;          // units : vd_cnt_clk_base (apb clk)
	UINT32 vd_cnt_pxl;          // units : pixel

	/* hd */
	UINT32 hd_cnt_clk_base;     // units : Hz
	UINT32 hd_cnt_clk;          // units : hd_cnt_clk_base (apb clk)
	UINT32 hd_cnt_line;         // units : line
	UINT32 line_end_cnt;	    // units : line
	UINT32 hd_cur_cnt_line;	    // units : line
} CTL_SIE_SYS_INFO;

/* active window idx */
typedef enum {
	CTL_SIE_ACTWIN_IDX_1,   // active window
	CTL_SIE_ACTWIN_IDX_2,   // active 2 window
	CTL_SIE_ACTWIN_IDX_MAX,
	ENUM_DUMMY4WORD(CTL_SIE_ACTWIN_IDX)
} CTL_SIE_ACTWIN_IDX;

typedef struct {
	BOOL act_en[CTL_SIE_ACTWIN_IDX_MAX];
} CTL_SIE_DMA_OUT;

typedef enum {
	CTL_SIE_EXTDATA_LOOP_OFF,           // disable extraction1 & extraction2
	CTL_SIE_EXTDATA_LOOP_FIRST_MAIN,    // loop main data (act) first (initial -> main_data (act) -> extract_data (act2) -> main_data (act) -> extract_data (act2) -> ... -> main_data (act) -> extract_data (act2))
	CTL_SIE_EXTDATA_LOOP_FIRST_EXT,     // loop extract data (act2) first (initial -> extract_data (act2) -> main_data (act) -> extract_data (act2) -> main_data (act) -> ... -> extract_data (act2) -> main_data (act))
	ENUM_DUMMY4WORD(CTL_SIE_EXTDATA_LOOP)
} CTL_SIE_EXTDATA_LOOP;

typedef struct {

	/* initial data information */
	UINT32                  ini_len;        // initial len, units : line

	/* loop data information */
	CTL_SIE_EXTDATA_LOOP    loop_sel;       // loop select
	UINT32                  loop_cnt;       // loop times
	UINT32                  loop_main_len;  // loop main_data len, units : line
	UINT32                  loop_ext_len;   // loop extract_data len, units : line

} CTL_SIE_EXTDATA;

typedef struct {
	BOOL                dvs_en;			// dvs sensor enable
	USIZE               dvs_size;		// dvs sensor only
} CTL_SIE_DVS_INFO;

typedef enum {
	CTL_SID_TSEN_TX_LEVEL_LOW = 0,				///< unused data output level low
	CTL_SIE_TSEN_TX_LEVEL_HIGH,					///< unused data output level high
	CTL_SIE_TSEN_TX_LEVEL_MAX,
} CTL_SIE_TSEN_TX_LEVEL_SEL;

typedef enum {
	CTL_SIE_TSEN_TX_SYNC_CODE_MODE_SD0 = 0,		///< output port 1-bit
	CTL_SIE_TSEN_TX_SYNC_CODE_MODE_SD0_1,		///< output port 2-bit
	CTL_SIE_TSEN_TX_SYNC_CODE_MODE_SD0_2,		///< output port 3-bit
	CTL_SIE_TSEN_TX_SYNC_CODE_MODE_SD0_6,		///< output port 7-bit
	CTL_SIE_TSEN_TX_SYNC_CODE_MODE_MAX,
} CTL_SIE_TSEN_TX_SYNC_CODE_MODE;

typedef enum {
    CTL_SIE_TSEN_TX_FS_IDLE_MODE_HILO_END = 0,          ///< frame sync signal go to idle when tx high/low clock cycle end
    CTL_SIE_TSEN_TX_FS_IDLE_MODE_DATA_END,              ///< frame sync signal go to idle when tx config/stream data end
    CTL_SIE_TSEN_TX_FS_IDLE_MODE_NS,
	CTL_SIE_TSEN_TX_FS_IDLE_MODE_MAX = CTL_SIE_TSEN_TX_FS_IDLE_MODE_NS,
    ENUM_DUMMY4WORD(CTL_SIE_TSEN_TX_FS_IDLE_MODE_SEL)
} CTL_SIE_TSEN_TX_FS_IDLE_MODE_SEL;                     ///< 538 not supported, 539A supported

typedef enum {
    CTL_SIE_TSEN_TX_FS_INV_NONE = 0,                    ///< frame sync signal invert disable
    CTL_SIE_TSEN_TX_FS_INV_OUT,                         ///< frame sync signal invert output(high/low/delay/idle)
    CTL_SIE_TSEN_TX_FS_INV_HILO,                        ///< frame sync signal invert high/low only, delay/idle keep low
    CTL_SIE_TSEN_TX_FS_INV_HILO_OUT,                    ///< frame sync signal invert high/low + output
    CTL_SIE_TSEN_TX_FS_INV_NS,
	CTL_SIE_TSEN_TX_FS_INV_MAX = CTL_SIE_TSEN_TX_FS_INV_NS,
    ENUM_DUMMY4WORD(CTL_SIE_TSEN_TX_FS_INV_SEL)
} CTL_SIE_TSEN_TX_FS_INV_SEL;                           ///< 538 not supported, 539A supported

typedef struct {
    UINT32  tx_dly_cnt;                             ///< clock count of delay for frame sync signal,      from 0 ~ 0xffff
    UINT32  tx_high_cnt;                            ///< clock count of level high for frame sync signal, from 1 ~ 0xffffffff
    UINT32  tx_low_cnt;                             ///< clock count of level low  for frame sync signal, from 1 ~ 0xffffffff
}  CTL_SIE_TSEN_TX_FS_CLKCNT;                    	///< 538 not supported, 539A supported, SD6 as FS output pin when enable tsen_tx_fs

typedef enum {
	CTL_SIE_TSEN_TX_OOC_MODE_SD0 = 0,			///< output port 1-bit
	CTL_SIE_TSEN_TX_OOC_MODE_SD0_1_2PXL_M1,		///< output port 2-bit, 2-pixel mode-1, SD0=>Pixel(N,0), SD1=>Pixel(N,1)
	CTL_SIE_TSEN_TX_OOC_MODE_SD0_1_2PXL_M2,		///< output port 2-bit, 2-pixel mode-2, SD0=>Pixel(N,1), SD1=>Pixel(N,0)
	CTL_SIE_TSEN_TX_OOC_MODE_SD0_1,				///< output port 2-bit, 1-pixel mode
	CTL_SIE_TSEN_TX_OOC_MODE_SD0_2,				///< output port 3-bit
	CTL_SIE_TSEN_TX_OOC_MODE_SD0_6,				///< output port 7-bit
	CTL_SIE_TSEN_TX_OOC_MODE_MAX,
} CTL_SIE_TSEN_TX_OOC_MODE_SEL;

typedef enum {
	CTL_SIE_TSEN_TX_OOC_BITDEPTH_6 = 0,
	CTL_SIE_TSEN_TX_OOC_BITDEPTH_7,
	CTL_SIE_TSEN_TX_OOC_BITDEPTH_8,
	CTL_SIE_TSEN_TX_OOC_BITDEPTH_MAX,
} CTL_SIE_TSEN_TX_OOC_BITDEPTH_SEL;

typedef enum {
	CTL_SIE_TSEN_RX_DECODE_MODE_DVP_8BIT = 0,	///< PCLK + VD + HD + D0~7
	CTL_SIE_TSEN_RX_DECODE_MODE_DVP_14BIT,		///< PCLK + VD + HD + D0~13
	CTL_SIE_TSEN_RX_DECODE_MODE_SYNC_CODE,		///< base on sync code setting
	CTL_SIE_TSEN_RX_DECODE_MODE_TX_FS_LS,		///< base on OOC TX FS/LS loopback signal with FS/LS_Delay setting
	CTL_SIE_TSEN_RX_DECODE_MODE_MAX,
} CTL_SIE_TSEN_RX_DECODE_MODE;

typedef enum {
	CTL_SIE_TSEN_RX_OUT_FMT_MSB = 0,			///< output data for raw 14bit from buffer[15:0] MSB => [15:2]
	CTL_SIE_TSEN_RX_OUT_FMT_LSB,				///< output data for raw 14bit from buffer[15:0] LSB => [13:0]
	CTL_SIE_TSEN_RX_OUT_FMT_MAX,
} CTL_SIE_TSEN_RX_OUT_FMT;

#define CTL_SIE_TSEN_LVL_NUM 7
#define CTL_SIE_TSEN_FSCODE_NUM 8
#define CTL_SIE_TSEN_LSCODE_NUM 8
#define CTL_SIE_TSEN_BNK_NUM 3
typedef struct {
	BOOL                thermal_en;								///< thermal sensor enable
	CTL_SIE_TSEN_TX_CFG_MODE tx_mode;							///< config data output port mode
	UINT16 tx_len;												///< from 4 ~ 512 clock cycle
	UINT32 tx_data[CTL_SIE_TSEN_CFG_NUM];						///< config data
	UINT32 tx_period;       									///< config data output period, clock cycle, 538 not supported, 539A supported, from 0 ~ 31 => means 1~32 clock cycle output 1 data
    UINT32 tx_blanking[2];  									///< blanking clock cycle for tx config,     538 not supported, 539A supported, | blanking[0] | tx_config | blanking[1] |, from 0 ~ 0xffff
	CTL_SIE_TSEN_TX_LEVEL_SEL tx_lvl[CTL_SIE_TSEN_LVL_NUM];		///< SD#0~6 output level

	CTL_SIE_TSEN_TX_SYNC_CODE_MODE sync_mode;					///< sync_code data output port mode
	UINT16 fs_code_len;											///< FS code length, 4 ~ 32 clock cycle
	UINT16 ls_code_len;											///< LS code length, 4 ~ 32 clock cycle
	UINT32 fs_code[CTL_SIE_TSEN_FSCODE_NUM];					///< FS code data
	UINT32 ls_code[CTL_SIE_TSEN_LSCODE_NUM];					///< LS code data

	UINT16 fs_delay;											///< frame start delay cycle when rx_mode=CTL_SIE_TSEN_RX_DECODE_MODE_TX_FS_LS
	UINT16 ls_delay;											///< line  start delay cycle when rx_mode=CTL_SIE_TSEN_RX_DECODE_MODE_TX_FS_LS

	BOOL tx_combine_en;											//538 not supported, 539A supported
	BOOL tx_fs_en;												//538 not supported, 539A supported
	CTL_SIE_TSEN_TX_FS_IDLE_MODE_SEL	fs_idle_mode_sel;		//538 not supported, 539A supported
	CTL_SIE_TSEN_TX_FS_INV_SEL			fs_inv_sel;				//538 not supported, 539A supported
	CTL_SIE_TSEN_TX_FS_CLKCNT			fs_clk_cnt;				//538 not supported, 539A supported

	CTL_SIE_TSEN_TX_OOC_MODE_SEL ooc_mode;						///< OOC data output port mode
	CTL_SIE_TSEN_TX_OOC_BITDEPTH_SEL ooc_bitdepth;				///< support on CTL_SIE_TSEN_TX_OOC_MODE_SD0_1_2PXL_M1 and CTL_SIE_TSEN_TX_OOC_MODE_SD0_1_2PXL_M2
	BOOL ooc_data_swap;											///< data swap, none(MSB->LSB), do-swap(LSB->MSB)
	UINT8 ooc_data_r_shift;										///< data right-shift 0 ~ 7
	UINT8 ooc_err_det_code;										///< OOC illegal code detection
	UINT8 ooc_err_rep_code;										///< OOC replace code for illegal data
	UINT16 ooc_width;											///< OOC frame width
	UINT16 ooc_height;											///< OOC frame height
	UINT8 ooc_dummy_top_cnt;									///< OOC frame dummy top    line count
	UINT8 ooc_dummy_bot_cnt;									///< OOC frame dummy bottom line count
	UINT8 ooc_dummy_val;
	UINT16 ooc_vblanking[CTL_SIE_TSEN_BNK_NUM];					///< OOC frame vertical   blanking0,1,2, unit: clock cycle
	UINT16 ooc_hblanking[CTL_SIE_TSEN_BNK_NUM];					///< OOC frame horizontal blanking0,1,2, unit: clock cycle

	CTL_SIE_TSEN_RX_DECODE_MODE rx_mode;						///< receive frame data mode
	CTL_SIE_TSEN_RX_OUT_FMT out_fmt;							///< receive frame data output format
	BOOL hi_byte_inv;											///< [15:8][7:0] => [8:15][7:0]
	BOOL lo_byte_inv;											///< [15:8][7:0] => [15:8][0:7]
	UINT8 hi_byte_r_shift;										///< [15:8][7:0] => 2bit shift, [15:10][0][0][7:0]
	UINT8 lo_byte_r_shift;										///< [15:8][7:0] => 2bit shift, [15 :8][7:2][0][0]
	BOOL two_byte_swap;											///< [15:8][7:0] => [7:0][15:8]
	BOOL two_byte_inv;											///< [15:0]      => [0:15]
	UINT8 two_byte_r_shift;										///< [15:0]      => 3bit shift, [15:3][0][0][0]
} CTL_SIE_TSEN_INFO;

typedef struct {
	ULONG dbg_buf_addr_va; // dbg dramout virtual address
	ULONG dbg_buf_addr_pa; // dbg dramout physical address

	URECT crp_win;
	USIZE scale_sz;
	URECT act2_win;

	CTL_SIE_OUT_DEST        out_dest;
	CTL_SIE_DATAFORMAT      data_fmt_ch3;
	CTL_SIE_FLIP_TYPE 		flip;
	CTL_SIE_EXTDATA_LOOP    extdata_loop_sel;

	BOOL b_chk_flag;
	UINT32 act_en: 1;
	UINT32 act2_en: 1;
	UINT32 ringbuf_en: 1;

	UINT32 funcen_rawenc: 1;
	UINT32 funcen_dvs: 1;
	UINT32 funcen_ecs: 1;
	UINT32 funcen_dpc: 1;
	UINT32 funcen_cgain: 1;
	UINT32 funcen_roi_acc: 1;
	UINT32 funcen_dgain: 1;
	UINT32 funcen_evs: 1;
	UINT32 funcen_stcs_ca: 1;
	UINT32 funcen_stcs_la: 1;
	UINT32 funcen_stcs_histo_y: 1;
	UINT32 funcen_stcs_dgain: 1;
	UINT32 funcen_comp: 1;

} CTL_SIE_DRAMOUT_DBG_INFO;

#if CTL_SIE_HRTIMER_MODE != 0
typedef struct {
	BOOL en;
	INT32 bufidx;
	INT32  vd_cnt_idx;
	INT32  crpst_cnt_idx;
	INT32  crpend_cnt_idx;
} CTL_SIE_HRTIMER_CTL;
#endif

typedef enum {
	CTL_SIE_INT_ITEM_REV_0 = 0,      					///< for ctl_sie_module_update_load_flg / kdf_sie_get / ...
	CTL_SIE_INT_ITEM_DMA_OUT,    						///< [Set    ] data_type: KDRV_SIE_DMA_OUT_EN
	CTL_SIE_INT_ITEM_INTE,                              ///< [Set    ] data_type: UINT32, set interrupte enable
	CTL_SIE_INT_ITEM_BP1,                               ///< [Set/Get] data_type: UINT32, set 0 to disable bp
	CTL_SIE_INT_ITEM_BP2,                               ///< [Set/Get] data_type: UINT32, set 0 to disable bp
	CTL_SIE_INT_ITEM_BP3,                               ///< [Set/Get] data_type: UINT32, set 0 to disable bp
	CTL_SIE_INT_ITEM_CH0_ADDR,                          ///< [Set/Get] data_type: UINT32, need 4 byte align
	CTL_SIE_INT_ITEM_CH1_ADDR,                          ///< [Set/Get] data_type: UINT32, need 4 byte align
	CTL_SIE_INT_ITEM_CH2_ADDR,                          ///< [Set/Get] data_type: UINT32, need 4 byte align
	CTL_SIE_INT_ITEM_CH3_ADDR,                          ///< [Set/Get] data_type: UINT32, need 4 byte align
	CTL_SIE_INT_ITEM_CHDBG_ADDR,                        ///< [Set/Get] data_type: UINT32, need 4 byte align
	CTL_SIE_INT_ITEM_CA_ROI,                            ///< [Set/Get] data_type: KDRV_SIE_CA_ROI
	CTL_SIE_INT_ITEM_LA_ROI,                            ///< [Set/Get] data_type: KDRV_SIE_LA_ROI
	CTL_SIE_INT_ITEM_ROI_ACC_ROI,                       ///< [Set/Get] data_type: KDRV_SIE_ROI_ACC
	CTL_SIE_INT_ITEM_OB,                                ///< [Set/Get] data_type: CTL_SIE_OB_PARAM
	CTL_SIE_INT_ITEM_CA,                                ///< [Set/Get] data_type: CTL_SIE_CA_PARAM
	CTL_SIE_INT_ITEM_LA,                                ///< [Set/Get] data_type: CTL_SIE_LA_PARAM
	CTL_SIE_INT_ITEM_ROI_ACC,                           ///< [Set/Get] data_type: CTL_SIE_ROI_ACC_PARAM
	CTL_SIE_INT_ITEM_ROI_ACC_RSLT,						///< [    Get] data_type: CTL_SIE_ISP_ROI_ACC_RSLT
	CTL_SIE_INT_ITEM_CGAIN,                             ///< [Set/Get] data_type: CTL_SIE_CGAIN
	CTL_SIE_INT_ITEM_DGAIN,                             ///< [Set/Get] data_type: CTL_SIE_DGAIN
	CTL_SIE_INT_ITEM_DPC,                               ///< [Set/Get] data_type: CTL_SIE_DPC
	CTL_SIE_INT_ITEM_ECS,                               ///< [Set/Get] data_type: CTL_SIE_ECS
	CTL_SIE_INT_ITEM_COMPAND,                           ///< [Set/Get] data_type: CTL_SIE_COMPANDING
	CTL_SIE_INT_ITEM_PXCLK,                             ///< [Set/Get] data_type: CTL_SIE_PXCLKSRC_SEL
	CTL_SIE_INT_ITEM_CLK,                               ///< [    Get] data_type: UINT32
	CTL_SIE_INT_ITEM_LIMIT,                             ///< [    Get] data_type: KDRV_SIE_LIMIT
	CTL_SIE_INT_ITEM_IR_INFO,                           ///< [    Get] data_type: CTL_SIE_ISP_RGBIR_INFO
	CTL_SIE_INT_ITEM_CH_OUT_MODE,                       ///< [Set    ] data_type: CTL_SIE_OUTPUT_MODE_TYPE
	CTL_SIE_INT_ITEM_SINGLE_OUT_CTL,                    ///< [Set/Get] data_type: CTL_SIE_SINGLE_OUT_CTRL,
	CTL_SIE_INT_ITEM_RING_BUF,                          ///< [Set    ] data_type: CTL_SIE_RING_BUF_CTL
	CTL_SIE_INT_ITEM_REF_LOAD_ID,                       ///< [Set    ] data_type: CTL_SIE_ID
	CTL_SIE_INT_ITEM_DVS,                            	///< [Set/Get] data_type: CTL_SIE_DVS_INFO
	CTL_SIE_INT_ITEM_TSEN_INFO,                         ///< [Set/Get] data_type: CTL_SIE_TSEN_INFO
	CTL_SIE_INT_ITEM_VD_DLY,                            ///< [Set/Get] data_type: CTL_SIE_VD_DLY, multi-frame vd delay in SIE
	CTL_SIE_INT_ITEM_SYS_INFO,                          ///< [    Get] data_type: CTL_SIE_SYS_INFO
	CTL_SIE_INT_ITEM_EXTDATA,                           ///< [Set    ] data_type: CTL_SIE_EXTDATA
	CTL_SIE_INT_ITEM_DRAMOUT_DBG,                       ///< [Set    ] data_type: BOOL
	CTL_SIE_INT_ITEM_CCIR,                       		///< [Set    ]
	CTL_SIE_INT_ITEM_MAX,
	CTL_SIE_INT_ITEM_REV = 0x8000000000000000,
	ENUM_DUMMY4WORD(CTL_SIE_INT_ITEM)
} CTL_SIE_INT_ITEM;
STATIC_ASSERT((CTL_SIE_INT_ITEM_MAX &CTL_SIE_INT_ITEM_REV) == 0);
#define CTL_SIE_IGN_CHK CTL_SIE_INT_ITEM_REV    //only support set/get function

typedef struct {
	CTL_SIE_OUT_DEST        out_dest;           ///< out dest.
	CTL_SIE_SERIAL_RAW_BIT  serial_raw_bitdepth;///< sie input bitdepth
	CTL_SIE_DATAFORMAT      data_fmt;           ///< sie data format
	CTL_SIE_DATAFORMAT      data_fmt_ch3;       ///< sie data format
	CTL_SIE_CCIR_INFO_INT   ccir_info_int;      ///< ccir internal information
	CTL_SIE_IO_SIZE_INFO    io_size_info;       ///< sie io size info
	CTL_SIE_FLIP_TYPE       flip;               ///< flip type
	CTL_SIE_ENC_INFO 		enc_info;
	CTL_SIE_CHGSENMODE_INFO chg_senmode_info;
	BOOL                    dma_abort;
	CTL_SIE_DVS_CODE        dvs_code;
	CTL_SIE_EVS_INFO        evs_info;
	CTL_SIE_MASK            mask;
	CTL_SIE_GROUP_INFO		group_info;
	CTL_SIE_IN_CH3_INFO		in_ch3_info;
	CTL_SIE_TSEN_TX_CFG		tsen_tx_cfg;
} CTL_SIE_ITEM_PARAM;

typedef struct {
	CTL_SIE_RAW_PIX     	rawcfa_act;                             // raw cfa start pixel for act window
	CTL_SIE_RAW_PIX     	rawcfa_crp;                             // raw cfa start pixel for crop window
	URECT               	sie_act_win;                            // sie active window
	URECT               	sie_act_win2;                           // sie active window 2
	UINT32              	out_ch_lof[CTL_SIE_DRAM_CH_MAX];        // line offset of each output channel
	CTL_SIE_PATGEN_PAR  	pat_gen_param;                          // pattern gen parameters
	CTL_SIE_SIGNAL      	signal;                                 // signal info
	CTL_SIE_DVS_INFO    	dvs_info;								// dvs sensor only
	CTL_SIE_TSEN_INFO		tsen_info;								// thermal sensor only, get from sen drv
	CTL_SIE_EXTDATA     	extdata;                                // extract data information
} CTL_SIE_RTC_CTRL_OBJ;

typedef struct {
	CTL_SIE_GYRO_CFG gyro_cfg;
	UINT32 gyro_buf_sz;
	BOOL   chk_latency_en;	//set ENABLE when trigger start for check gyro latency
#if CTL_SIE_HRTIMER_MODE != 0
	CTL_SIE_HRTIMER_CTL hrtmr_ctl;
#endif
} CTL_SIE_GYRO_CTL_INFO;

#define SYNC_REC_MAX_NUM 10
typedef struct {
	UINT32 mode;            //0:disable, 1: rec, 2:rec + sync(main)
	UINT32 sync_id;         //
	UINT32 sync_diff;       //frame start time diff (us)
	UINT32 det_pause;       //det pause flag, 0:resume, 1:pause
	UINT32 det_frm_int;     //det frame interval (frame counter)
	UINT32 det_frm_cnt;     //det frame counter
	UINT32 det_1st_done;    //det first time

	UINT32 adj_thres;       //frame diff (us)
	UINT32 adj_proc_step;   //sync flow step
	UINT32 adj_time;        //sync flow adj vd time
	UINT32 adj_id;          //sync flow adj id
	UINT32 adj_auto;        //1:auto, 0: adj_self
	UINT32 adj_cnt;

	UINT32 org_fps;         //keep org sensor fps setting

	UINT32 rec_cnt;                     //rec cnt
	UINT32 rec_time[SYNC_REC_MAX_NUM];  //rec vd time

	//debug
	UINT32 isr_vd_t;

} CTL_SIE_SYNC_CTRL_OBJ;

typedef struct {
	UINT64	item;		// 1<< update_item from CTL_SIE_ITEM
	UINT64  item_int;	// 1<< update_item from CTL_SIE_INT_ITEM
} CTL_SIE_UPD_LOAD_ITEM;

typedef enum {
	CTL_SIE_INT_GRP_OFF 	= 0,
	CTL_SIE_INT_GRP_FS 		= 0x1,
	CTL_SIE_INT_GRP_SHDR 	= 0x2,
	CTL_SIE_INT_GRP_COMBINE = 0x4,
	CTL_SIE_INT_GRP_PATGEN 	= 0x8,
	ENUM_DUMMY4WORD(CTL_SIE_INT_GRP_TYPE)
} CTL_SIE_INT_GRP_TYPE;

typedef struct {
	UINT32	total_num;				// total SIE number
	UINT32  total_idx;				// toal CTL_SIE_ID_IDX
	CTL_SIE_INT_GRP_TYPE  grp_type;	// group type
	UINT32	comb_num;				// combine frame number (2: left+right)
	UINT32  comb_idx;				// combine id idx
	UINT32  comb_order;				// order of image 0(left) ~ comb_num -1(right)
	UINT32  comb_ovlp_left_width;	// left overlap width
	UINT32  comb_ovlp_right_width;	// right overlap width
	USIZE 	valid_size;
	URECT 	act_win;
	USIZE 	crp_size;
	UINT32  lead_id;				// first output frame CTL_SIE_ID
} CTL_SIE_INT_MFRM_INFO;

// internal ctrl
typedef struct {
	CTL_SIE_ISRCB           int_isrcb_fp;                           // internal isr cb function pointer
	CTL_SIE_CB_INFO         ext_isrcb_fp;                           // external isr cb function pointer
	CTL_SIE_CB_INFO         bufiocb;                                // buf in/out isr cb function pointer
	CTL_SIE_CB_INFO         direct_cb_fp;                           // direct mode cb function pointer, used for sync with ipp unit
	CTL_SIE_DMA_OUT         dma_out;                                // all dma output enable(active enable)
	CTL_SIE_UPD_LOAD_ITEM   update_item;							// "1<< update_item from CTL_SIE_ITEM" or "1<< update_item from CTL_SIE_INT_ITEM"
	CTL_SIE_INTE            inte;                                   // interrupt enable
	ULONG                   out_ch_addr_va[CTL_SIE_DRAM_CH_MAX];    // virtual address of each output channel
	ULONG                   out_ch_addr_pa[CTL_SIE_DRAM_CH_MAX];    // physical address of each output channel
	CTL_SIE_HEADER_INFO     head_info[CTL_SIE_HEAD_IDX_MAX];        // sie push out header address
	CTL_SIE_DIRECT_CB_TO_IPP_INFO direct_to_ipp_info[CTL_SIE_HEAD_IDX_MAX];	//sie direct cb to ipp info
	CTL_SIE_FRM_CTL_INFO    frame_ctl_info;
	CTL_SIE_TRIG_INFO       trig_info;
	CTL_SIE_CLK_INFO        clk_info;
	CTL_SIE_BUF_INFO        buf_info;
	CTL_SIE_RST_FC_STATUS   rst_fc_sts;
	UINT32                  row_time;
	UINT32                  bp1;
	UINT32                  bp2;
	UINT32                  bp3;
	UINT32                  bp3_ratio;          // manual ratio setting for act_start to bp3, bp3 will set by (ratio*total_line)/100
	CTL_SIE_DRAM_OUT_CTRL   ch_out_mode;
	CTL_SIE_SINGLE_OUT_CTRL sin_out_ctl;
	CTL_SIE_RING_BUF_CTL    ring_buf_ctl;
//	CTL_SIE_EVS_BUF_CTL     evs_buf_ctl;
	ULONG ca_subout_addr;	//keep subout addr for get statis out before push buf
	ULONG la_subout_addr;	//keep subout addr for get statis out before push buf
	ULONG ca_private_addr;	//copy subout data here and wait isp get
	ULONG la_private_addr;	//copy subout data here and wait isp get
	CTL_SIE_ID              ref_load_id;    ///< hw global load, 560 only support SIE1 ref SIE2
	CTL_SIE_VD_DLY          vd_dly;
	UINT32                  sen_dft_fps;        ///< (fps * 100)
	BOOL                        dbg_en;                         // enable check fw & hw debug data dramout debug enable
	CTL_SIE_DRAMOUT_DBG_INFO    dbg_info[CTL_SIE_HEAD_IDX_MAX]; // fw debug data, check with hw dramout debug info
	atomic_t 					dbg_log_cnt[CTL_SIE_DRAMOUT_DBG_ITEM_MAX];

	//isp ctrl
	KDRV_SIE_CA_ROI         ca_roi;         ///< ca crop roi
	KDRV_SIE_LA_ROI         la_roi;         ///< la crop roi
	KDRV_SIE_ROI_ACC_ROI    roi_acc_roi;    ///< roi acc crop roi
	BOOL ca_rst_valid;
	BOOL la_rst_valid;

	CTL_SIE_OB_PARAM        ob_param;
	CTL_SIE_CA_PARAM        ca_param;
	KDRV_SIE_LA_PARAM       la_param;
	CTL_SIE_CGAIN           cgain_param;
	CTL_SIE_DGAIN           dgain_param;
	KDRV_SIE_DPC            dpc_param;
	KDRV_SIE_ECS            ecs_param;
	KDRV_SIE_ROI_ACC_PARAM  roi_acc_param;
	CTL_SIE_COMPANDING      companding_param;
	BOOL                    alg_func_en[CTL_SIE_ALG_TYPE_MAX];  ///< algorithm id(iq, ae, awb, af...) */

	//multi frame ctrl
	CTL_SEN_MODE_TYPE		sen_mode_type;
	CTL_SEN_DATA_FMT   		sen_data_fmt;
	CTL_SIE_INT_MFRM_INFO 	mfrm_info;

	//runtime change obj
	CTL_SIE_RTC_CTRL_OBJ    rtc_obj;        ///< runtime change obj

	// sw vd sync
	CTL_SIE_SYNC_CTRL_OBJ   sync_obj;
	//gyro config
	CTL_SIE_GYRO_CTL_INFO	gyro_ctl_info;
} CTL_SIE_CTRL_OBJ;

typedef struct {
	BOOL                    normal_hd_create;
	BOOL                    normal_shdr_rst;
	BOOL                    normal_vd;
	BOOL                    normal_bp;
	BOOL                    normal_ce;
} CTL_SIE_FB_OBJ;

typedef struct {
	UINT32                  tag;
	ULONG                   kdf_hdl;
	ULONG                   evt_hdl;

	/* CTL_SIE_OPEN_CFG */
	CTL_SIE_ID              id;
	CTL_SIE_ID              dupl_src_id;
	UINT32                  sen_id;
	ISP_ID                  isp_id;
	CTL_SIE_FLOW_TYPE       flow_type;

	/* set by CTL_SIE_ITEM */
	CTL_SIE_ITEM_PARAM      param;
	/* internal ctrl parameters */
	CTL_SIE_CTRL_OBJ        ctrl;

	/* runtime change CTL_SIE_ITEM */
	CTL_SIE_ITEM_PARAM      rtc_param;
	/* runtime change internal ctrl parameters */
	CTL_SIE_RTC_CTRL_OBJ    rtc_ctrl;

	/* ready for load CTL_SIE_ITEM */
	CTL_SIE_ITEM_PARAM      load_param;
	/* ready for load internal ctrl parameters */
	CTL_SIE_RTC_CTRL_OBJ    load_ctrl;

	/* fastboot object */
	CTL_SIE_FB_OBJ fb_obj;

	BOOL dvi_interlace_skip_en;
} CTL_SIE_HDL;

/* active window idx */
typedef enum {
	CTL_SIE_INT_GETRAW_STS_BEGIN,
	CTL_SIE_INT_GETRAW_STS_PROC,
	CTL_SIE_INT_GETRAW_STS_END,
	CTL_SIE_INT_GETRAW_STS_MAX,
	ENUM_DUMMY4WORD(CTL_SIE_INT_GETRAW_STS)
} CTL_SIE_INT_GETRAW_STS;

typedef struct {
	CTL_SIE_UPD_LOAD_ITEM set_load_flg; // "1<< update_item from CTL_SIE_ITEM" or "1<< update_item from CTL_SIE_INT_ITEM"
	UINT32 get_cnt_flg;
	UINT32 cfg_start_bp;
	UINT32 last_isr_sts;
	BOOL isr_dram_end_flg;
	BOOL isr_valid_vd_flg;      //vd proc will set TRUE, bp3 proc will clear to FALSE
	BOOL isr_chk_dram_end_flg;  //vd proc will set TRUE
	BOOL isr_load_en_flg;
	BOOL resume_rst_flg;        //reset ctrl info when resume
	BOOL interlace_skip_flg;	//skip next frame sie config and trigger ipp
	BOOL gyro_en;
	CTL_SIE_INT_GETRAW_STS get_raw_sts;
} CTL_SIE_INT_CTL_FLG;

typedef struct {
	UINT32 context_num;
	UINT32 context_used;    //current used context number
	UINT32 context_idx[CTL_SIE_MAX_SUPPORT_ID];
	UINT32 contex_size;;    //each context buf size
	ULONG  start_addr;
	UINT32 req_size;        //total ctl_sie ctx require size (context_num*contex_size)
	UINT32 total_buf_sz;    //total ctx require size (ctl_sie+kdf_sie+kdrv_sie)s
	BOOL dbg_buf_alloc;
	BOOL auto_alloc_mem;
	CTL_SIE_VOS_MEM_INFO vos_mem_info;  //private memory, auto allocate memory when init address is 0
} CTL_SIE_HDL_CONTEXT;

#define CTL_SIE_GET_ID(hdl) ctl_sie_hdl_conv2_id(hdl)

#define CTL_SIE_DIV_U64(x, y) ctl_sie_uint64_dividend(x, y)
#define CTL_SIE_MAX(x, y) ctl_sie_max(x, y)
#define CTL_SIE_MIN(x, y) ctl_sie_min(x, y)
#define CTL_SIE_LCM(x, y) ctl_sie_lcm(x, y)
#define CTL_SIE_CHK_ALIGN(src, align) ctl_sie_chk_align(src, align)

extern BOOL ctl_sie_module_chk_id_valid(CTL_SIE_ID id);
extern CTL_SIE_ID ctl_sie_hdl_conv2_id(ULONG hdl);
extern UINT32 ctl_sie_uint64_dividend(UINT64 dividend, UINT32 divisor);
extern BOOL ctl_sie_module_chk_is_raw(CTL_SEN_MODE_TYPE mode_type);
extern UINT32 ctl_sie_max(UINT32 x, UINT32 y);
extern UINT32 ctl_sie_min(UINT32 x, UINT32 y);
extern UINT32 ctl_sie_lcm(UINT32 x, UINT32 y);
extern BOOL ctl_sie_chk_align(UINT32 x, UINT32 y);
extern UINT32 ctl_sie_get_hdl_tag(void);
extern CTL_SIE_HDL *ctl_sie_get_hdl(CTL_SIE_ID id);
extern CTL_SIE_HDL_CONTEXT *ctl_sie_get_ctx(void);
extern CTL_SIE_LIMIT *ctl_sie_limit_query(CTL_SIE_ID id);
extern CTL_SIE_STATUS ctl_sie_get_state_machine(CTL_SIE_ID id);
CTL_SIE_DIRECT_SIE_STS ctl_sie_module_get_direct_state_machine(CTL_SIE_ID id);
extern INT32 ctl_sie_get_sen_cfg(CTL_SIE_ID id, CTL_SEN_CFG cfg, void *data);
extern UINT64 ctl_sie_util_get_syst_timestamp(void);
extern UINT32 ctl_sie_util_get_timestamp(void);
extern INT32 ctl_sie_util_os_malloc(CTL_SIE_VOS_MEM_INFO *vod_mem_info, UINT32 req_size);
extern INT32 ctl_sie_util_os_mfree(CTL_SIE_VOS_MEM_INFO *vod_mem_info);
extern void *ctl_sie_util_os_malloc_wrap(UINT32 want_size);
extern void ctl_sie_util_os_mfree_wrap(void *p_buf);
extern UINT32 (*ctl_sie_cert_func)(UINT32, UINT32, UINT32);

//debug api

typedef enum {
	CTL_SIE_DBG_TS_VD = 0,
	CTL_SIE_DBG_TS_BP3,
	CTL_SIE_DBG_TS_ADDR_RDY,
	CTL_SIE_DBG_TS_ADDR_FAIL,
	CTL_SIE_DBG_TS_DRAMEND,
	CTL_SIE_DBG_TS_RCVMSG,
	CTL_SIE_DBG_TS_UNLOCK,
	CTL_SIE_DBG_TS_CRPST,
	CTL_SIE_DBG_TS_CRPEND,
	CTL_SIE_DBG_TS_MAX
} CTL_SIE_DBG_TS_EVT;

typedef enum {
	CTL_SIE_STS_VD_ISR_FLG = 0,
	CTL_SIE_STS_BP3_ISR_FLG,
	CTL_SIE_STS_CROPEND_ISR_FLG,
	CTL_SIE_STS_ISR_FLG_MAX
} CTL_SIE_DBG_STS_ISR_TYPE;

typedef struct {
	BOOL(*chk_msg_type)(CTL_SIE_ID id, UINT32 chk_type);
	UINT32(*query_dbg_buf)(void);
	void(*set_dbg_buf)(UINT32 id, ULONG buf_addr);
	void(*set_ts)(CTL_SIE_ID id, CTL_SIE_DBG_TS_EVT evt, UINT64 sie_fc);                                // set timestamp record
	void(*reset_ts)(CTL_SIE_ID id);                                                                     // reset timestamp
	void(*dump_buf_io)(CTL_SIE_ID id, CTL_SIE_BUF_IO_CFG buf_io, UINT32 total_size, ULONG header_addr); // dump sie buffer in/out info
	void(*set_isr_ioctl)(CTL_SIE_ID id, UINT32 status, UINT32 buf_ctl_type, CTL_SIE_HEAD_IDX head_idx); // set isr control info
	void(*set_frm_ctrl_info)(CTL_SIE_ID id, CTL_SIE_FRM_CTL_INFO *frm_ctrl);                            // set frame control info
	void(*set_isp_cb_t_log)(ISP_ID id, ISP_EVENT evt, UINT64 fc, UINT32 ts_start, UINT32 ts_end);       // set isp callback timestamp info
	void(*isp_cb_t_dump)(int (*dump)(const char *fmt, ...));                                            // isp callback timestamp dump
	void(*set_proc_t)(CTL_SIE_ID id, CHAR *proc_name, CTL_SIE_PROC_TIME_ITEM item);                     // set timestamp record
	void(*upd_sysdbg_info)(CTL_SIE_ID id, UINT64 sie_fc, CTL_SIE_SYS_INFO *info_out);       			// update system information
	void(*get_buf_wp_info)(CTL_SIE_ID id, UINT32 *ddr_id, UINT32 *dis_wp);                              // get sie out ddr id, and release_before_push_en
	void(*stat_isr_proc)(CTL_SIE_ID id, CTL_SIE_DBG_STS_ISR_TYPE isr_flg, UINT32 start_ts);				// calculate ctl_sie vd, bp3, cropend proc time 
} CTL_SIE_INT_DBG_TAB;

void ctl_sie_update_getraw_sts(CTL_SIE_ID id, CTL_SIE_INT_GETRAW_STS sts);
extern CTL_SIE_INT_DBG_TAB *ctl_sie_get_dbg_tab(void);
extern void ctl_sie_reg_dbg_tab(CTL_SIE_INT_DBG_TAB *dbg_tab);
extern void ctl_sie_set_dbg_lvl(CTL_SIE_DBG_LVL dbg_lvl);

extern BOOL ctl_sie_chk_hw_direct_id(CTL_SIE_ID id);
extern BOOL ctl_sie_chk_hw_ringbuf_id(CTL_SIE_ID id);

extern CTL_SIE_LIMIT ctl_sie_limit[CTL_SIE_MAX_SUPPORT_ID];

#if defined(__KERNEL__)
extern struct clk *ctl_sie_clk_get(const char *clk_name);
extern void ctl_sie_clk_put(struct clk *clk);
extern struct clk *ctl_sie_clk_get_parent(struct clk *clk);
extern int ctl_sie_clk_set_parent(struct clk *clk, struct clk *parent);
extern unsigned long ctl_sie_clk_get_rate(struct clk *clk);
extern int ctl_sie_clk_set_gate(struct clk *clk, BOOL enable);
#endif

#endif //_CTL_SIE_UTILITY_INT_H_

