/**
    SIE CTRL Layer

    @file       sie_ctrl_int.h
    @ingroup    mIIPL_Ctrl
    @note       None

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/

#ifndef _SIE_CTRL_INT_H
#define _SIE_CTRL_INT_H

#define _INLINE static inline
#include "ctl_sen.h"
#include "ctl_sen_ext.h"
#include "ctl_sie.h"
#include "kdf_sie_int.h"
#include "ctl_sie_isp_task_int.h"
#include "ctl_sie_dbg.h"

#if (defined(__LINUX) && !defined(CONFIG_NVT_TIMER)) || defined(__FREERTOS)
#define CTL_SIE_HRTIMER_MODE	0				// 0: Not support
#else
#define CTL_SIE_HRTIMER_MODE	1				// 1: Normal; 2: test hrtimer; 3: compare hwclock/hrtimer
#endif
#define CTL_SIE_RAW_COMPRESS_BIT        12
#if defined(CONFIG_NVT_FPGA_EMULATION) || defined(_NVT_FPGA_)
#define CTL_SIE_PAT_GEN_DIRECT_MIN_ROW_T 100 //us
#else
#define CTL_SIE_PAT_GEN_DIRECT_MIN_ROW_T 15 //us, for ipp 12us*1.2 ~15us
#endif
#define CTL_SIE_PAT_GEN_CLK 			320000000	//
#define CTL_SIE_PAT_GEN_SRC_PAD 		8
#define CTL_SIE_PAT_GEN_SRC_WIDTH_ALIGN	64		// for bcc using
#define CTL_SIE_BP_MIN_END_LINE         200     // bp to crop_end line counter, for error handle
#define CTL_SIE_BP_MIN_END_TIME         40000   // bp to crop_end time (*10us)
#define CTL_SIE_BP_STR_LINE_DIRECT_MIN  100     // crop start to bp line counter(direct mode) minimum line

#define CTL_SIE_DFT_ERR_INT_EN		(CTL_SIE_INTE_ERR_RAWENC | CTL_SIE_INTE_SRC_BUF_OVFL | CTL_SIE_INTE_RAWENC_FIFO_OVFL | CTL_SIE_INTE_RAWENC_DATA_OVLAP | CTL_SIE_INTE_RAWENC_DATA_BEHAVIOR_ERR)
#define CTL_SIE_DFT_INT_EN  		(CTL_SIE_DFT_ERR_INT_EN | CTL_SIE_INTE_VD | CTL_SIE_INTE_CRPST | CTL_SIE_INTE_BP3 | CTL_SIE_INTE_CROPEND | CTL_SIE_INTE_DRAM_OUTD_END | CTL_SIE_INTE_DRAM_OUT0_END) //VD/BP3/Crop_End for buffer control, others : debugs
#define CTL_SIE_DFT_TSEN_INT_EN 	(CTL_SIE_INT_TSEN_OOC_VD | CTL_SIE_INT_TSEN_END | CTL_SIE_INT_TSEN_DRAM_IN3_UDFL | CTL_SIE_INT_TSEN_FIFO_OVFL)

#define CTL_SIE_DBG_DRAMOUT_SZ KDRV_SIE_DBG_DRAMOUT_SZ

#define CTL_SIE_BP3_NEXTVD_MIN_US   3000    // 3000us, bp3 to next_vd min time, for sie latch parameters in next vd
#define CTL_SIE_BP3_CROPEND_LINE    40       // 40 line, bp3 to crop_end line_num

#define CTL_SIE_DFT_ENC_RATE CTL_SIE_ENC_50

#define CTL_SIE_IFE_CHK_MODE 1 // after 530, always fixed 1 (before 560 alway fixed 0, because hw not support)

#if defined(CONFIG_NVT_FPGA_EMULATION) || defined(_NVT_FPGA_)
#define CTL_SIE_TO_MS 10000
#else
#define CTL_SIE_TO_MS 2000
#endif

/**
    atomic operations
*/
#if defined(__LINUX)
/* use linux kernel atomic api */
#else
/* use gcc api */
typedef INT32 atomic_t;
#define ATOMIC_INIT(i) (i)
#define atomic_inc(x) __sync_add_and_fetch(x, 1)
#define atomic_dec(x) __sync_sub_and_fetch(x, 1)
#define atomic_read(x) __sync_add_and_fetch(x, 0)
#define atomic_set(x, v) __sync_bool_compare_and_swap(x, *x, v)
#define atomic_add(i, x) __sync_add_and_fetch(x, i)
#define atomic_xchg(x, v) __sync_val_compare_and_swap(x, *x, v)
#endif


#define CTL_SIE_DRAMOUT_DBG_DATA_TAG_START  0x87654321
#define CTL_SIE_DRAMOUT_DBG_DATA_TAG_END    0x5a5a5a5a
typedef struct {

	UINT32 ofs0_tag_start: 32;

	UINT32 ofs1_tag_vd: 32;

	UINT32 ofs2_tag_bp3: 32;

	UINT32 ofs3_tag_cropend: 32;

	UINT32 ofs4_dramout_mode_ch0: 1;
	UINT32 ofs4_dramout_mode_ch1: 1;
	UINT32 ofs4_dramout_mode_ch2: 1;
	UINT32 ofs4_dramout_mode_chd: 1;
	UINT32 ofs4_rev1: 12;
	UINT32 ofs4_dramout_singleout_ch0: 1;
	UINT32 ofs4_dramout_singleout_ch1: 1;
	UINT32 ofs4_dramout_singleout_ch2: 1;
	UINT32 ofs4_dramout_singleout_chd: 1;
	UINT32 ofs4_rev2: 12;

	UINT32 ofs5_rev1: 1;
	UINT32 ofs5_funcen_pg: 1;
	UINT32 ofs5_rev2: 1;
	UINT32 ofs5_funcen_obavg: 1;
	UINT32 ofs5_funcen_obsub: 1;
	UINT32 ofs5_funcen_obbypass: 1;
	UINT32 ofs5_funcen_mask0: 1;
	UINT32 ofs5_funcen_mask1: 1;
	UINT32 ofs5_funcen_mask2: 1;
	UINT32 ofs5_funcen_mask3: 1;
	UINT32 ofs5_funcen_dvs: 1;
	UINT32 ofs5_funcen_dpc: 1;
	UINT32 ofs5_rev3: 1;
	UINT32 ofs5_funcen_actdataext: 1;
	UINT32 ofs5_rev4: 1;
	UINT32 ofs5_funcen_ecs: 1;
	UINT32 ofs5_funcen_dgain: 1;
	UINT32 ofs5_funcen_bsh: 1;
	UINT32 ofs5_funcen_bsv: 1;
	UINT32 ofs5_funcen_rawenc: 1;
	UINT32 ofs5_funcen_cgain: 1;
	UINT32 ofs5_funcen_roi_acc: 1;
	UINT32 ofs5_funcen_stcs_histo_y: 1;
	UINT32 ofs5_funcen_stcs_la: 1;
	UINT32 ofs5_funcen_stcs_ca: 1;
	UINT32 ofs5_funcen_stcs_dgain: 1;
	UINT32 ofs5_funcen_evs: 1;
	UINT32 ofs5_funcen_dramout_ch0: 1;
	UINT32 ofs5_funcen_comp: 1;
	UINT32 ofs5_bayer_fmt: 2;
	UINT32 ofs5_funcen_dbg: 1;

	UINT32 ofs6_crop_x: 14;
	UINT32 ofs6_rev1: 2;
	UINT32 ofs6_crop_y: 14;
	UINT32 ofs6_rev2: 2;

	UINT32 ofs7_crop_w: 14;
	UINT32 ofs7_rev1: 2;
	UINT32 ofs7_crop_h: 14;
	UINT32 ofs7_rev2: 2;

	UINT32 ofs8_scale_w: 14;
	UINT32 ofs8_rev1: 2;
	UINT32 ofs8_scale_h: 14;
	UINT32 ofs8_rev2: 2;

	UINT32 ofs9_dramout_addr0_ch0: 32;
	UINT32 ofs10_dramout_addr1_ch0: 32;

	UINT32 ofs11_dramout_ofs_ch0: 16;
	UINT32 ofs11_dramout_packbit_ch0: 2;
	UINT32 ofs11_rev1: 2;
	UINT32 ofs11_dramout_ringbuf_en: 1;
	UINT32 ofs11_rev2: 9;
	UINT32 ofs11_dramout_packfmt_ch0: 1;
	UINT32 ofs11_rev3: 1;

	UINT32 ofs12_dramout_ringbuf_len: 32;

	UINT32 ofs13_dramout_addr0_ch1: 32;
	UINT32 ofs14_dramout_addr1_ch1: 32;

	UINT32 ofs15_dramout_ofs_ch1: 32;

	UINT32 ofs16_dramout_addr0_ch2: 32;
	UINT32 ofs17_dramout_addr1_ch2: 32;

	UINT32 ofs18_rev1: 32;

	UINT32 ofs19_tag_end: 32;

} CTL_SIE_DRAMOUT_DBG_DATA;

typedef enum {
	ctl_sie_dbgitem_dbg_data_size,
	ctl_sie_dbgitem_tag,

	ctl_sie_dbgitem_dramout_ch0_mode,
	ctl_sie_dbgitem_dramout_ch1_mode,
	ctl_sie_dbgitem_dramout_ch2_mode,
	ctl_sie_dbgitem_dramout_chd_mode,

	ctl_sie_dbgitem_dramout_ch0_singleout,
	ctl_sie_dbgitem_dramout_ch1_singleout,
	ctl_sie_dbgitem_dramout_ch2_singleout,
	ctl_sie_dbgitem_dramout_chd_singleout,

	ctl_sie_dbgitem_dramout_ch0_addr,
	ctl_sie_dbgitem_dramout_ch1_addr,
	ctl_sie_dbgitem_dramout_ch2_addr,

	ctl_sie_dbgitem_crp_x,
	ctl_sie_dbgitem_crp_y,
	ctl_sie_dbgitem_crp_w,
	ctl_sie_dbgitem_crp_h,

	ctl_sie_dbgitem_scl_w,
	ctl_sie_dbgitem_scl_h,

	ctl_sie_dbgitem_func_bsh,
	ctl_sie_dbgitem_func_bsv,

	ctl_sie_dbgitem_dramout_ch0_lofs,
	ctl_sie_dbgitem_dramout_ch0_packbit,
	ctl_sie_dbgitem_dramout_ch0_packfmt,
	ctl_sie_dbgitem_dramout_ch0_ringen,
	ctl_sie_dbgitem_dramout_ch0_ringlen,

	ctl_sie_dbgitem_func_pg,
	ctl_sie_dbgitem_func_dvs,
	ctl_sie_dbgitem_func_dpc,
	ctl_sie_dbgitem_func_ext,
	ctl_sie_dbgitem_func_ecs,
	ctl_sie_dbgitem_func_dgain,
	ctl_sie_dbgitem_func_rawenc,
	ctl_sie_dbgitem_func_cgain,
	ctl_sie_dbgitem_func_roi_acc,
	ctl_sie_dbgitem_func_stcs_histy,
	ctl_sie_dbgitem_func_stcs_la,
	ctl_sie_dbgitem_func_stcs_ca,
	ctl_sie_dbgitem_func_stcs_dgain,
	ctl_sie_dbgitem_func_evs,
	ctl_sie_dbgitem_func_comp,
	ctl_sie_dbgitem_func_dramout_ch0_dest,
	ctl_sie_dbgitem_func_dramout_ch0,

	CTL_SIE_DRAMOUT_DBG_ITEM_MAX,

} CTL_SIE_DRAMOUT_DBG_ITEM;

// funtion type for sie
typedef INT32(*CTL_SIE_ISRCB)(UINT32, UINT32, void *, void *);
typedef INT32(*CTL_SIE_SET_FP)(CTL_SIE_ID, void *);
typedef INT32(*CTL_SIE_GET_FP)(CTL_SIE_ID, void *);

/**
            SIE STATE MACHINE

                          ----- SUSPEND -----
                         / --->         <--- \
                 resume / /suspend   suspend\ \ resume
                       / /                    \ \
            Open      < /        Trig start    \ >
    CLOSE   ----->  READY   ----------------->  RUN
            <-----          <-----------------
            Close                Trig stop



            ======= DMA_ABORT =======

            READY   -----------> DMA_ABORT -----> system shutdown
            RUN     -----------> DMA_ABORT -----> system shutdown
            SUSPEND -----------> DMA_ABORT -----> system shutdown

*/
typedef enum {
	CTL_SIE_STS_CLOSE = 0,
	CTL_SIE_STS_READY,
	CTL_SIE_STS_RUN,
	CTL_SIE_STS_SUSPEND,
	CTL_SIE_STS_DMA_ABORT,
	CTL_SIE_STS_MAX,
	ENUM_DUMMY4WORD(CTL_SIE_STATUS)
} CTL_SIE_STATUS;

typedef enum {
	CTL_SIE_HEAD_IDX_CUR = 0,
	CTL_SIE_HEAD_IDX_NEXT,
	CTL_SIE_HEAD_IDX_MAX,
	ENUM_DUMMY4WORD(CTL_SIE_HEAD_IDX)
} CTL_SIE_HEAD_IDX;

typedef enum {
	CTL_SIE_OP_OPEN = 0,
	CTL_SIE_OP_CLOSE,
	CTL_SIE_OP_TRIG_START,
	CTL_SIE_OP_TRIG_STOP,
	CTL_SIE_OP_SET,
	CTL_SIE_OP_GET,
	CTL_SIE_OP_SUSPEND,
	CTL_SIE_OP_RESUME,
	CTL_SIE_OP_DMA_ABORT,
	CTL_SIE_OP_MAX,
	ENUM_DUMMY4WORD(CTL_SIE_OP)
} CTL_SIE_OP;

typedef enum {
	CTL_SIE_RST_FC_DONE = 0,
	CTL_SIE_RST_FC_BEGIN,
	CTL_SIE_RST_FC_NONE,
	CTL_SIE_RST_FC_MAX,
	ENUM_DUMMY4WORD(CTL_SIE_RST_FC_STATUS)
} CTL_SIE_RST_FC_STATUS;

typedef struct {
	BOOL out_enable;
	UINT32 data_size;
} CTL_SIE_CH_OUT_INFO;

typedef struct {
	BOOL				update_flg;			// Set TRUE if the buf_info should update. Will be clear after update
	UINT32              total_size;			// Size needed for next buffer
	CTL_SIE_CH_OUT_INFO ch_info[CTL_SIE_DRAM_CH_MAX];
} CTL_SIE_BUF_INFO;

typedef enum {
	CTL_SIE_INT_DATAFORMAT_RAW  	= 0x00000001, // CTL_SIE_BAYER_xx
	CTL_SIE_INT_DATAFORMAT_CCIR 	= 0x00000002, // CTL_SIE_YUV_xx
	CTL_SIE_INT_DATAFORMAT_DVS  	= 0x00000004, // CTL_SIE_Y_8
	CTL_SIE_INT_DATAFORMAT_EVS  	= 0x00000008, //
	CTL_SIE_INT_DATAFORMAT_THERMAL  = 0x00000010, // thermal only support 16bit out
} CTL_SIE_INT_DATAFORMAT;

typedef struct {
	UINT64      vd_cnt;             ///< sie ctl vd interrupt counter
	UINT64      crp_end_cnt;        ///< sie ctl crop end interrupt counter
	UINT64      leader_frame_cnt;   ///< sie ctl leader frame counter for multi frame
	UINT64      frame_cnt;          ///< sie ctl frame counter
	UINT64      new_ok_cnt;         ///< new buffer success counter
	UINT64      new_fail_cnt;       ///< new buffer fail counter
	UINT64      in_buf_queue_full_cnt; ///< in buffer queue full,  skip new buffer
	UINT64      out_buf_queue_full_cnt; ///< out buffer queue full, skip push/unlock buffer
	UINT64      drop_cnt;           ///< drop buffer counter
	UINT64      push_cnt;           ///< push out frame counter
	UINT64      flush_cnt;          ///< buffer flush counter(flush only when sie_stop(wait_end) and close)
	UINT64      dir_rls_cnt;        ///< ipp direct mode buffer release cnt
	UINT64      dir_rls_fail_cnt;   ///< ipp direct mode buffer release fail cnt
	UINT64      dir_drop_cnt;       ///< ipp direct mode buffer drop cnt
	UINT64      dir_drop_fail_cnt;  ///< ipp direct mode buffer drop fail cnt
	UINT64      chk_hw_data_ok_cnt;         ///< check hw dramout debug data
	UINT64      chk_hw_data_fail_cnt;       ///< check hw dramout debug data
	UINT64      chk_hw_data_tag_fail_cnt;   ///< check hw dramout debug data tag fail
} CTL_SIE_FRM_CTL_INFO;

typedef struct {
	BOOL	combine_en;
	UINT32  combine_idx;
	UINT32 	combine_num;
	UINT32	combine_width;		//center width of each stripe
	UINT32	combine_ovlp_width;	//right overlap of each stripe
	VDO_FRAME vdo_frm;
} CTL_SIE_DIRCT_CB_INFO;

typedef enum {
	CTL_SIE_BUF_IDX_CUR = 0,
	CTL_SIE_BUF_IDX_NEXT,
	CTL_SIE_BUF_IDX_MAX,
	ENUM_DUMMY4WORD(CTL_SIE_BUF_IDX)
} CTL_SIE_BUF_IDX;

/* time record */
typedef enum {
	CTL_SIE_PROC_TIME_ITEM_ENTER,
	CTL_SIE_PROC_TIME_ITEM_EXIT,
	CTL_SIE_PROC_TIME_ITEM_MAX,
	ENUM_DUMMY4WORD(CTL_SIE_PROC_TIME_ITEM),
} CTL_SIE_PROC_TIME_ITEM;

void ctl_sie_update_inte(CTL_SIE_ID id, BOOL enable, CTL_SIE_INTE sts);
BOOL ctl_sie_chk_fmt(ULONG sie_hdl, CTL_SIE_DATAFORMAT src, CTL_SIE_INT_DATAFORMAT chk);
void ctl_sie_hdl_update_item(CTL_SIE_ID id, UINT64 item, UINT64 item_int, BOOL set_imm);
void ctl_sie_hdl_load_all(CTL_SIE_ID id);
INT32 ctl_sie_module_get_gyro_data(CTL_SIE_ID id, ULONG sie_header_addr);
INT32 ctl_sie_module_direct_to_both(CTL_SIE_ID id);
INT32 ctl_sie_module_both_to_direct(CTL_SIE_ID id);
INT32 ctl_sie_set_load(CTL_SIE_ID id, void *data);
INT32 ctl_sie_direct_flow_ipp_cb(UINT32 event, void *p_in, void *p_out);	//for ipp callback to sie

#endif //_SIE_CTRL_INT_H
