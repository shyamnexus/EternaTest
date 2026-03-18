/**
    VIE CTRL Layer

    @file       vie_ctrl_int.h
    @ingroup    mIIPL_Ctrl
    @note       None

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/

#ifndef _VIE_CTRL_INT_H
#define _VIE_CTRL_INT_H

#define _INLINE static inline
#include "ctl_sen.h"
#include "ctl_vie.h"
#include "kdf_vie_int.h"
#include "ctl_vie_isp_task_int.h"
#include "ctl_vie_dbg.h"

#define CTL_VIE_PAT_GEN_SRC_W_PAD_RATIO 5
#define CTL_VIE_PAT_GEN_SRC_H_PAD       8
#define CTL_VIE_PAT_GEN_SRC_WIDTH_MAX     4096
#define CTL_VIE_PAT_GEN_SRC_HEIGHT_MAX     2160
#define CTL_VIE_BP_MIN_END_LINE         200     //bp to crop_end line counter, for error handle
#define CTL_VIE_DFT_INT_EN  (CTL_VIE_INTE_VD | CTL_VIE_INTE_CRPST | CTL_VIE_INTE_BP3 | CTL_VIE_INTE_CROPEND | CTL_VIE_INTE_DRAM_OUTD_END) //VD/BP3/Crop_End for buffer control, others : debugs
#define CTL_VIE_ALL_INT_EN  (CTL_VIE_INTE_VD | CTL_VIE_INTE_CRPST | CTL_VIE_INTE_BP3 | CTL_VIE_INTE_CROPEND | CTL_VIE_INTE_DRAM_OUTD_END | CTL_VIE_INTE_DRAM_OUT0_END | CTL_VIE_INTE_DRAM_OUT1_END)

#define CTL_VIE_BP3_NEXTVD_MIN_US   3000    // 3000us, bp3 to next_vd min time, for vie latch parameters in next vd
#define CTL_VIE_BP3_CROPEND_LINE    20      // 20 line, bp3 to crop_end line_num

#define CTL_VIE_ENCODE_LOF(lof) (lof * 3) / 4 // 75%

#define CTL_VIE_DBG_DRAMOUT_SZ KDRV_VIE_DBG_DRAMOUT_SZ


#if defined(CONFIG_NVT_FPGA_EMULATION) || defined(_NVT_FPGA_)
#define CTL_VIE_TO_MS 10000
#else
#define CTL_VIE_TO_MS 2000
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



#define CTL_VIE_DRAMOUT_DBG_DATA_TAG_START  0x87654321
#define CTL_VIE_DRAMOUT_DBG_DATA_TAG_END    0x5a5a5a5a
typedef struct {

	UINT32 ofs0_tag_start: 32;

	UINT32 ofs1_tag_vd: 32;

	UINT32 ofs2_tag_bp3: 32;

	UINT32 ofs3_tag_cropend: 32;

	UINT32 ofs4_dramout_mode_ch0: 1;
	UINT32 ofs4_dramout_mode_ch1: 1;
	UINT32 ofs4_dramout_mode_chd: 1;
	UINT32 ofs4_rev1: 13;
	UINT32 ofs4_dramout_singleout_ch0: 1;
	UINT32 ofs4_dramout_singleout_ch1: 1;
	UINT32 ofs4_dramout_singleout_chd: 1;
	UINT32 ofs4_rev2: 13;

	UINT32 ofs5_crop_x: 14;
	UINT32 ofs5_rev1: 2;
	UINT32 ofs5_crop_y: 14;
	UINT32 ofs5_rev2: 2;

	UINT32 ofs6_crop_w: 14;
	UINT32 ofs6_rev1: 2;
	UINT32 ofs6_crop_h: 14;
	UINT32 ofs6_rev2: 2;

	UINT32 ofs7_dramout_addr0_ch0: 32;

	UINT32 ofs8_dramout_ofs_ch0: 16;
	UINT32 ofs8_dramout_mirror_ch0: 1;
	UINT32 ofs8_dramout_flip_ch0: 1;
	UINT32 ofs8_rev1: 14;

	UINT32 ofs9_dramout_addr0_ch1: 32;

	UINT32 ofs10_dramout_ofs_ch1: 16;
	UINT32 ofs10_dramout_mirror_ch1: 1;
	UINT32 ofs10_dramout_flip_ch1: 1;
	UINT32 ofs10_rev1: 14;

	UINT32 ofs11_dramout_addr1_ch0: 32;
	UINT32 ofs12_dramout_addr1_ch1: 32;
	UINT32 ofs13_tag_end: 32;

} CTL_VIE_DRAMOUT_DBG_DATA;

typedef enum {
	ctl_vie_dbgitem_dbg_data_size,
	ctl_vie_dbgitem_tag,

	ctl_vie_dbgitem_dramout_ch0_mode,
	ctl_vie_dbgitem_dramout_ch1_mode,
	ctl_vie_dbgitem_dramout_chd_mode,

	ctl_vie_dbgitem_dramout_ch0_singleout,
	ctl_vie_dbgitem_dramout_ch1_singleout,
	ctl_vie_dbgitem_dramout_chd_singleout,

	ctl_vie_dbgitem_dramout_ch0_addr,
	ctl_vie_dbgitem_dramout_ch1_addr,

	ctl_vie_dbgitem_crp_x,
	ctl_vie_dbgitem_crp_y,
	ctl_vie_dbgitem_crp_w,
	ctl_vie_dbgitem_crp_h,

	ctl_vie_dbgitem_dramout_ch0_lofs,
	ctl_vie_dbgitem_dramout_ch0_mirror,
	ctl_vie_dbgitem_dramout_ch0_flip,

	ctl_vie_dbgitem_dramout_ch1_lofs,
	ctl_vie_dbgitem_dramout_ch1_mirror,
	ctl_vie_dbgitem_dramout_ch1_flip,

	CTL_VIE_DRAMOUT_DBG_ITEM_MAX,

} CTL_VIE_DRAMOUT_DBG_ITEM;


// funtion type for vie
typedef INT32(*CTL_VIE_ISRCB)(UINT32, UINT32, void *, void *);
typedef INT32(*CTL_VIE_SET_FP)(CTL_VIE_ID, void *);
typedef INT32(*CTL_VIE_GET_FP)(CTL_VIE_ID, void *);

/**
            VIE STATE MACHINE

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
	CTL_VIE_STS_CLOSE = 0,
	CTL_VIE_STS_READY,
	CTL_VIE_STS_RUN,
	CTL_VIE_STS_SUSPEND,
	CTL_VIE_STS_DMA_ABORT,
	CTL_VIE_STS_MAX,
	ENUM_DUMMY4WORD(CTL_VIE_STATUS)
} CTL_VIE_STATUS;

typedef enum {
	CTL_VIE_HEAD_IDX_CUR = 0,
	CTL_VIE_HEAD_IDX_NEXT,
	CTL_VIE_HEAD_IDX_MAX,
	ENUM_DUMMY4WORD(CTL_VIE_HEAD_IDX)
} CTL_VIE_HEAD_IDX;

typedef enum {
	CTL_VIE_OP_OPEN = 0,
	CTL_VIE_OP_CLOSE,
	CTL_VIE_OP_TRIG_START,
	CTL_VIE_OP_TRIG_STOP,
	CTL_VIE_OP_SET,
	CTL_VIE_OP_GET,
	CTL_VIE_OP_SUSPEND,
	CTL_VIE_OP_RESUME,
	CTL_VIE_OP_DMA_ABORT,
	CTL_VIE_OP_MAX,
	ENUM_DUMMY4WORD(CTL_VIE_OP)
} CTL_VIE_OP;

typedef enum {
	CTL_VIE_RST_FC_DONE = 0,
	CTL_VIE_RST_FC_BEGIN,
	CTL_VIE_RST_FC_NONE,
	CTL_VIE_RST_FC_MAX,
	ENUM_DUMMY4WORD(CTL_VIE_RST_FC_STATUS)
} CTL_VIE_RST_FC_STATUS;

typedef struct {
	BOOL out_enable;
	UINT32 data_size;
} CTL_VIE_CH_OUT_INFO;

typedef struct {
	UINT32              total_size;
	CTL_VIE_CH_OUT_INFO ch_info[CTL_VIE_DRAM_CH_MAX];
} CTL_VIE_BUF_INFO;

typedef enum {
	CTL_VIE_INT_DATAFORMAT_CCIR = 0x00000001, // CTL_VIE_YUV_xx
} CTL_VIE_INT_DATAFORMAT;

typedef struct {
	UINT64      vd_cnt;             ///< vie ctl vd interrupt counter
	UINT64      crp_end_cnt;        ///< vie ctl crop end interrupt counter
	UINT64      frame_cnt;          ///< vie ctl frame counter
	UINT64      new_ok_cnt;         ///< new buffer success counter
	UINT64      new_fail_cnt;       ///< new buffer fail counter
	UINT64      buf_queue_full_cnt; ///< buffer queue full, skip new buffer
	UINT64      drop_cnt;           ///< drop buffer counter
	UINT64      push_cnt;           ///< push out frame counter
	UINT64      flush_cnt;          ///< buffer flush counter(flush only when vie_stop(wait_end) and close)
	UINT64      chk_hw_data_ok_cnt;         ///< check hw dramout debug data
	UINT64      chk_hw_data_fail_cnt;       ///< check hw dramout debug data
	UINT64      chk_hw_data_tag_fail_cnt;   ///< check hw dramout debug data tag fail
} CTL_VIE_FRM_CTL_INFO;

typedef enum {
	CTL_VIE_BUF_IDX_CUR = 0,
	CTL_VIE_BUF_IDX_NEXT,
	CTL_VIE_BUF_IDX_MAX,
	ENUM_DUMMY4WORD(CTL_VIE_BUF_IDX)
} CTL_VIE_BUF_IDX;

typedef enum {
	CTL_VIE_UPD_INTE_DISABLE,       // for runtime change, will not disable flow control inte
	CTL_VIE_UPD_INTE_ENABLE,
	CTL_VIE_UPD_INTE_FORCE_DISABLE, // for stop, will disable flow control inte
	CTL_VIE_UPD_INTE_FORCE_ENABLE,  // for start, will enable hdal & isp inte
} CTL_VIE_UPD_INTE;

/* time record */
typedef enum {
	CTL_VIE_PROC_TIME_ITEM_ENTER,
	CTL_VIE_PROC_TIME_ITEM_EXIT,
	CTL_VIE_PROC_TIME_ITEM_MAX,
	ENUM_DUMMY4WORD(CTL_VIE_PROC_TIME_ITEM),
} CTL_VIE_PROC_TIME_ITEM;

void ctl_vie_update_inte(CTL_VIE_ID id, CTL_VIE_UPD_INTE enable, CTL_VIE_INTE sts);
void ctl_vie_hdl_update_item(CTL_VIE_ID id, UINT64 item, BOOL set_imm);
void ctl_vie_hdl_load_all(CTL_VIE_ID id);

void ctl_vie_update_fc(CTL_VIE_ID id);
UINT64 ctl_vie_get_fc(CTL_VIE_ID id);

INT32 ctl_vie_set_load(CTL_VIE_ID id, void *data);

#endif //_VIE_CTRL_INT_H