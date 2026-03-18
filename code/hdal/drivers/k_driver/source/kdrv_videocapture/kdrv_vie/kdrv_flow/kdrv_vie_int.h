#ifndef _KDRV_VIE_INT_H_
#define _KDRV_VIE_INT_H_

#include "kwrap/debug.h"
#include "kwrap/type.h"
#include "kwrap/cpu.h"
#include "kdrv_vie.h"
#include <nvt_api_ver.h>
#include "vie_lib.h"
#include "vie_limt.h"

#if defined(__FREERTOS)
#include <string.h>
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

// funtion type in vie driver
typedef void (*KDRV_VIE_DRV_ISR_FP)(UINT32 ui_intp_status, VIE_ENGINE_STATUS_INFO_CB *info);

// funtion type for dal_vie
typedef INT32(*KDRV_VIE_SET_FP)(KDRV_VIE_PROC_ID, KDRV_VIE_VDO_CH, void *);
typedef INT32(*KDRV_VIE_GET_FP)(KDRV_VIE_PROC_ID, KDRV_VIE_VDO_CH, void *);


/**
    VIE KDRV STATE MACHINE

                                   ----- SUSPEND -----
                                  / --->         <--- \
                          resume / /suspend   suspend\ \ resume
                                / /                    \ \
      OpenCfg        Open      < /        Trig start    \ >
CLOSE -------> INIT ----->  READY   ----------------->  RUN
      <-------------------          <-----------------
              Close                       Trig stop



*/
typedef enum {
	KDRV_VIE_STS_CLOSE = 0,
	KDRV_VIE_STS_READY,
	KDRV_VIE_STS_RUN,
	KDRV_VIE_STS_SUSPEND,
	KDRV_VIE_STS_INIT,
	KDRV_VIE_STS_MAX,
	ENUM_DUMMY4WORD(KDRV_VIE_STATUS)
} KDRV_VIE_STATUS;

typedef enum {
	KDRV_VIE_OP_OPEN = 0,
	KDRV_VIE_OP_CLOSE,
	KDRV_VIE_OP_TRIG_START,
	KDRV_VIE_OP_TRIG_STOP,
	KDRV_VIE_OP_SET,
	KDRV_VIE_OP_GET,
	KDRV_VIE_OP_OPENCFG,
	KDRV_VIE_OP_MAX,
	ENUM_DUMMY4WORD(KDRV_VIE_OP)
} KDRV_VIE_OP;

typedef enum {
	KDRV_VIE_PAR_CTL_SET = 0,   //user set parameters
	KDRV_VIE_PAR_CTL_CUR,       //current frame latch parameters
	KDRV_VIE_PAR_CTL_RDY,       //output ready for get
	KDRV_VIE_PAR_CTL_MAX,
	ENUM_DUMMY4WORD(KDRV_VIE_PAR_CTL)
} KDRV_VIE_PAR_CTL;

typedef struct {
	UINT32 ctx_num;
	UINT32 ctx_used;    //current used context number
	UINT32 ctx_idx[KDRV_VIE_MAX_ENG];
	UINT32 ctx_size;;   //each context buf size
	ULONG  start_addr;
	UINT32 req_size;    //total kdrv ctx require size (ctx_num*ctx_size)
} KDRV_VIE_HDL_CONTEXT;

typedef struct {
	KDRV_VIE_DRAM_CH ch_id;
	UINT32 lofs;
} KDRV_VIE_OUT_LOFS;

typedef struct {
	KDRV_VIE_DRAM_CH ch_id;
	ULONG addr_va;	// virtual address
	ULONG addr_pa;	// physical address
} KDRV_VIE_OUT_ADDR;

/**
    Collection of vie param

*/
#define KDRV_VIE_FUNCEN_CH0 (VIE_CH0_EN|VIE_CH0_MASK0_EN|VIE_CH0_MASK1_EN|VIE_CH0_MASK2_EN|VIE_CH0_MASK3_EN|VIE_CH0_YCC_ENC_EN|VIE_CH0_PATGEN_EN)
#define KDRV_VIE_FUNCEN_CH1 (VIE_CH1_EN|VIE_CH1_MASK0_EN|VIE_CH1_MASK1_EN|VIE_CH1_MASK2_EN|VIE_CH1_MASK3_EN|VIE_CH1_YCC_ENC_EN|VIE_CH1_PATGEN_EN)
#define KDRV_VIE_FUNCEN_CH2 (VIE_CH2_EN|VIE_CH2_MASK0_EN|VIE_CH2_MASK1_EN|VIE_CH2_MASK2_EN|VIE_CH2_MASK3_EN|VIE_CH2_YCC_ENC_EN|VIE_CH2_PATGEN_EN)
#define KDRV_VIE_FUNCEN_CH3 (VIE_CH3_EN|VIE_CH3_MASK0_EN|VIE_CH3_MASK1_EN|VIE_CH3_MASK2_EN|VIE_CH3_MASK3_EN|VIE_CH3_YCC_ENC_EN|VIE_CH3_PATGEN_EN)

#define KDRV_VIE_CH0_DBGEN 0x00000001 // enable video_ch0 dramout dbgdata function
#define KDRV_VIE_CH1_DBGEN 0x00000002 // enable video_ch1 dramout dbgdata function
#define KDRV_VIE_CH2_DBGEN 0x00000004 // enable video_ch2 dramout dbgdata function
#define KDRV_VIE_CH3_DBGEN 0x00000008 // enable video_ch3 dramout dbgdata function

typedef struct {
	KDRV_VIE_ACT_MODE	act_mode;
	KDRV_VIE_PAD_SEL    pad_sel[KDRV_VIE_MAX_VDO_CH];    // by channel. vie pad selection.
	KDRV_VIE_PAD_INFO	pad_info_a; // by engine. pad information.
	KDRV_VIE_PAD_INFO	pad_info_b; // by engine. pad information.
} KDRV_VIE_SIGNAL_INT;

typedef struct {
	KDRV_VIE_STATUS status;
	UINT32 func_en;                             // vie function enable (current channel only)
	KDRV_VIE_DATA_FMT data_fmt;                 // vie data format
	KDRV_VIE_CCIR_INFO ccir_info;               // ccir information
	KDRV_VIE_CRP_WIN crp_window;                // crop window
	UINT32 out_ch_lof[KDRV_VIE_DRAM_CH_MAX];    // line offset of each dram output channel
	ULONG  out_ch_addr_va[KDRV_VIE_DRAM_CH_MAX];   // virtual address of each dram output channel
	ULONG  out_ch_addr_pa[KDRV_VIE_DRAM_CH_MAX];   // physical address of each dram output channel
	KDRV_VIE_FLIP flip;                         // flip type
	KDRV_VIE_BP_INFO bp_info;                   // bp info
	KDRV_VIE_SINGLE_OUT_CTRL singleout_info;    // single-out info for all dram output channel
	KDRV_VIE_DRAM_OUT_CTRL ch_output_mode;      // channel output mode
	UINT16 ccir_header;                         // ccir header (for debug)
	UINT32 ccir_sts;                            // ccir haeder hit status (for debug)
	KDRV_VIE_PATGEN_INFO pat_gen_info;          // pattern gen information
	KDRV_VIE_MASK mask;							// mask
	BOOL four_k_mode; 							// enable/disable 4K mode
	BOOL dbg_en;								///< enable output dbg data
	KDRV_VIE_DELAYCHAIN_INFO delay_chan_info;
} KDRV_VIE_CH_INFO;
//STATIC_ASSERT((sizeof(KDRV_VIE_CH_INFO) % VOS_ALIGN_BYTES) == 0); // for kdrv_vie_set_opencfg allocate KDRV_VIE_CH_INFO memory

typedef struct {

	atomic_t open_cnt;                          // open++,close--
	atomic_t trigger_cnt;                       // TRIG_START++,TRIG_STOP--
	atomic_t suspend_cnt;                       // suspend++,resume--

	KDRV_VIE_OPENCFG    open_cfg;               // vie open config, no runtime change
	KDRV_VIE_ISRCB      isrcb_fp;               // isr cb function pointer
	KDRV_VIE_INT        inte;                   // interrupt enable

	KDRV_VIE_CH_INFO    *ch_info;               // video input channel information

	KDRV_VIE_SIGNAL_INT signal_int;             // signal info

	/* internal */
	UINT32              load_func_en;           // vie function enable
	UINT32				load_func_en2;          // vie function enable, 0x68 debug

} KDRV_VIE_INFO;

KDRV_VIE_STATUS kdrv_vie_get_status(KDRV_VIE_PROC_ID id, KDRV_VIE_VDO_CH ch);
KDRV_VIE_INFO *kdrv_vie_get_hdl(KDRV_VIE_PROC_ID id);
VIE_ENGINE_ID __conv2_vie_ssdrv_id(KDRV_VIE_PROC_ID id);
VIE_CHANNEL_ID __conv2_vie_ssdrv_ch(KDRV_VIE_VDO_CH ch);
UINT32 __conv2_vie_ssdrv_funcen(KDRV_VIE_VDO_CH ch, UINT32 func);
VIE_WAIT_EVENT_SEL __conv2_vie_ssdrv_waitvd(KDRV_VIE_VDO_CH ch);

#define _vie_ssdrv_id(id) __conv2_vie_ssdrv_id(id)
#define _vie_ssdrv_ch(ch) __conv2_vie_ssdrv_ch(ch)
#define _vie_ssdrv_funcen(ch, func) __conv2_vie_ssdrv_funcen(ch, func)
#define _vie_ssdrv_waitvd(ch) __conv2_vie_ssdrv_waitvd(ch)

BOOL kdrv_vie_chk_id_valid(UINT32 kdrv_id);
BOOL kdrv_vie_chk_vieid_valid(KDRV_VIE_PROC_ID id);
KDRV_VIE_HDL_CONTEXT *kdrv_vie_get_ctx(void);
KDRV_VIE_CLK_HDL *kdrv_vie_get_clk_hdl(KDRV_VIE_PROC_ID id);
UINT32 kdrv_vie_get_apb_clk(void);

extern ULONG (*kdrv_vie_cert_func)(ULONG, UINT32, UINT32);

#endif //_KDRV_VIE_INT_H_
