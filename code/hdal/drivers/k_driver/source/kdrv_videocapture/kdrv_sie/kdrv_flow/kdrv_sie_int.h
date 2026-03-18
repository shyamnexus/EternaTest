#ifndef _KDRV_SIE_INT_H_
#define _KDRV_SIE_INT_H_

/* common */
#include "kwrap/stdio.h"
#include "kwrap/type.h"
#include "kwrap/cpu.h"
#include <nvt_api_ver.h>
/* kdrv & ssdrv */
#include "kdrv_sie.h"
#include "sie_eng.h"
#include "sie_eng_base.h"
#include "sie_eng_handle.h"
#include "sie_eng_limit.h"

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

#define RAW_SCL_LPF 		50  ///< low-pass valumn, 0~100
#define RAW_SCL_BIN_PWR		0   ///< binning, 0~2
#define RAW_SCL_ADAPT_LPF   1	///< 0: LPF is absolute value; 1: LPF is adaptive value (suggestion: 1)

#define KDRV_SIE_ERR_INT_EN		(KDRV_SIE_INT_ERR_RAWENC | KDRV_SIE_INT_ERR_DPC|KDRV_SIE_INT_BEHAVIOR|KDRV_SIE_INT_SRC_BUF_OVFL|KDRV_SIE_INT_RAWENC_FIFO_OVFL|KDRV_SIE_INT_RAWENC_DATA_OVLAP)
#define KDRV_SIE_ERR_INT_DIS	(KDRV_SIE_INT_ERR_DRAMIO)
#define KDRV_SIE_DFT_INT		(KDRV_SIE_INT_VD | KDRV_SIE_INT_CROPEND | KDRV_SIE_ERR_INT_EN)

#define KDRV_SIE_CLK_MAX_FOR_BCC_480		765000000	//BCC clock 480Mhz only for sie clock <= 765Mhz
#define KDRV_SIE_TIMEOUT_MS 				2000
#define KDRV_SIE_TSEN_CFG_TIMEOUT_MS 		5

// funtion type in sie driver
#define KDRV_SIE_DRV_ISR_FP SIE_ISR_CB

// funtion type for dal_sie
typedef INT32 (*KDRV_SIE_SET_FP)(KDRV_SIE_PROC_ID, void*);
typedef INT32 (*KDRV_SIE_GET_FP)(KDRV_SIE_PROC_ID, void*);


/**
	SIE KDRV STATE MACHINE

  					      ----- SUSPEND -----
 				         / --->         <--- \
				 resume / /suspend   suspend\ \ resume
                       / /		              \ \
			Open	  < /	     Trig start    \ >
	CLOSE	----->	READY	----------------->  RUN
			<-----	  	 	<-----------------
			Close	   	  	     Trig stop

*/
typedef enum {
	KDRV_SIE_STS_CLOSE = 0,
	KDRV_SIE_STS_READY,
	KDRV_SIE_STS_RUN,
	CTL_SIE_STS_SUSPEND,
	KDRV_SIE_STS_MAX,
	ENUM_DUMMY4WORD(KDRV_SIE_STATUS)
} KDRV_SIE_STATUS;

typedef enum {
	KDRV_SIE_STS_SSDRV_CLOSE = 0,
	KDRV_SIE_STS_SSDRV_READY,
	KDRV_SIE_STS_SSDRV_RUN,
	KDRV_SIE_STS_SSDRV_MAX,
	ENUM_DUMMY4WORD(KDRV_SIE_STATUS_SSDRV)
} KDRV_SIE_STATUS_SSDRV;

typedef enum {
	KDRV_SIE_OP_OPEN = 0,
	KDRV_SIE_OP_CLOSE,
	KDRV_SIE_OP_TRIG_START,
	KDRV_SIE_OP_TRIG_STOP,
	KDRV_SIE_OP_SET,
	KDRV_SIE_OP_GET,
	KDRV_SIE_OP_MAX,
	ENUM_DUMMY4WORD(KDRV_SIE_OP)
} KDRV_SIE_OP;

typedef enum {
	KDRV_SIE_PAR_CTL_SET = 0,	//user set parameters
	KDRV_SIE_PAR_CTL_CUR,		//current frame latch parameters
	KDRV_SIE_PAR_CTL_RDY,		//output ready for get
	KDRV_SIE_PAR_CTL_MAX,
	ENUM_DUMMY4WORD(KDRV_SIE_PAR_CTL)
} KDRV_SIE_PAR_CTL;

typedef struct {
	UINT32 ctx_num;
	UINT32 ctx_used;	//current used context number
	UINT32 ctx_idx[KDRV_SIE_MAX_ENG];
	UINT32 ctx_size;;	//each context buf size
	ULONG  start_addr;
	UINT32 req_size;	//total kdrv ctx require size (ctx_num*ctx_size)
} KDRV_SIE_HDL_CONTEXT;

typedef struct {
	KDRV_SIE_DRAM_CH ch_id;
	UINT32 lofs;
} KDRV_SIE_OUT_LOFS;

/**
	Collection of sie dal param

	b_encode_en:
		Only control Enable/Disable
		Enable will cause ch5 output sideinfo, and Channel lineoffset can set to 70%
		note that encode can only enable when data_fmt = KDRV_SIE_BAYER_12

		Channel 5 Setting
		lineoffset = align_ceil_32(scl_size.w) * 16/8
		buffer_size = lineoffset * scl_size.h

*/
typedef struct {
	KDRV_SIE_OPENCFG open_cfg;				///< sie open config, no runtime change
	KDRV_SIE_ISRCB isrcb_fp;				///< isr cb function pointer
	KDRV_SIE_INT inte;						///< interrupt enable
	UINT32 func_en;							///< sie function enable, ex:SIE_PATGEN_EN
	KDRV_SIE_OUT_DEST out_dest;				///< sie output destination
	KDRV_SIE_SERIAL_RAW_PARAM	serial_raw_param;		///< serial raw param
	KDRV_SIE_DATA_FMT data_fmt[KDRV_SIE_DRAM_CH_MAX];	///< sie dramout data format
	KDRV_SIE_CCIR_INFO ccir_info;			///< ccir information
	KDRV_SIE_SIGNAL signal;					///< signal info
	KDRV_SIE_ACT_CRP_WIN act_window;		///< active window
	KDRV_SIE_ACT_CRP_WIN crp_window;		///< crop window
	USIZE scl_size;							///< raw scale size
	KDRV_SIE_IN_CH3_INFO in_ch3_info;
	UINT32 out_ch_lof[KDRV_SIE_DRAM_CH_MAX];			///< line offset of each output channel
	KDRV_SIE_ADDR_INFO out_ch_addr[KDRV_SIE_DRAM_CH_MAX];	///< address of each output channel
	KDRV_SIE_PATGEN_INFO pat_gen_info;		///< pattern mode info
	KDRV_SIE_FLIP flip;						///< flip type
	KDRV_SIE_OB_PARAM ob_param;				///< ob info
	KDRV_SIE_CA_PARAM ca_param;				///< ca info
	KDRV_SIE_CA_ROI ca_roi;                 ///< ca ROI, unit: pixel
	KDRV_SIE_LA_PARAM la_param;				///< la info
	KDRV_SIE_LA_ROI la_roi;				    ///< la ROI, unit: pixel
	KDRV_SIE_ROI_ACC_PARAM roi_acc_param;	///< roi acc info
	KDRV_SIE_ROI_ACC_ROI roi_acc_roi;		///< roi_acc ROI, unit: pixel
	KDRV_SIE_RAW_ENCODE encode_info;        ///< raw encode info
	KDRV_SIE_DGAIN dgain;					///< digital gain
	KDRV_SIE_CGAIN cgain;					///< color gain, deprecated after 530
	KDRV_SIE_DPC dpc_info;					///< dpc info
	KDRV_SIE_ECS ecs_info;					///< ecs info
	KDRV_SIE_DMA_OUT_EN dma_out_en;			///< dma output enable
	KDRV_SIE_BP_PARAM bp_param;               ///< bp info
	KDRV_SIE_COMPANDING comp_info;          ///< companding info
	KDRV_SIE_SINGLE_OUT_CTRL singleout_info;///< single-out info for all channel
	KDRV_SIE_DRAM_OUT_CTRL ch_output_mode;  ///< channel output mode
	KDRV_SIE_RINGBUF_INFO ring_buf_info;
	KDRV_SIE_PROC_ID ref_load_id;			///< load reference
	BOOL dma_abort;							///< dma abort
	KDRV_SIE_DVS_CODE dvs_code;				///< dvs code
	KDRV_SIE_DVS_INFO dvs_info;				///< dvs en and window size
	KDRV_SIE_EVS_INFO evs_info;				///< evs out threshold
	KDRV_SIE_VD_DLY vd_dly;					///< vd delay
	KDRV_SIE_MASK mask;						///< mask
	KDRV_SIE_EXTDATA extdata;				///< extern data
	BOOL dbg_en;									///< enable output dbg data

	//thermal sensor
	KDRV_SIE_TSEN_INFO 		thermal_info;		///< thermal info
	KDRV_SIE_TSEN_TX_CFG  	tsen_tx_cfg;

	/* internal control parameters */
	SIE_STCS_CALASIZE_INFO ca_la_info[KDRV_SIE_PAR_CTL_MAX];
	SIE_LA_WIN_PARAM la_win[KDRV_SIE_PAR_CTL_MAX];
	BOOL bcc_share;							///< bcc share
	UINT32 enc_frm_rst_id;					///< bcc frame reset id

	/* ctl param */
	SIE_ENG_HANDLE *ssdrv_hdl; // ssdrv handle
	SIE_LOAD_SRC ssdrv_load_src;

	/* chip info */
    UINT32           chip_id;            ///< engine platform id,      for function support check
    UINT32           chip_ver;           ///< engine platform version, for function support check
} KDRV_SIE_INFO;

typedef struct {
	SIE_ENG_MCLKSRC		mclk_src;
	SIE_ENG_PXCLKSRC	pxclk_src;
	SIE_ENG_CLKSRC		clk_src;
	SIE_ENG_BCCCLKSRC	bccclk_src;
	SIE_ENG_INTCLKSRC	intclk_src;
} KDRV_SIE_FB_CLK_INFO;

typedef enum {
	KDRV_SIE_INT_DATA_FMT_RAW		= 0x00000001, // KDRV_SIE_BAYER_xx
	KDRV_SIE_INT_DATA_FMT_CCIR		= 0x00000002, // KDRV_SIE_YUV_xx
	KDRV_SIE_INT_DATA_FMT_DVS		= 0x00000004, // KDRV_SIE_Y_8
} KDRV_SIE_INT_DATA_FMT;

typedef enum {
	KDRV_SIE_DMA_FLUSH_DIR_FROM_DEV,
	KDRV_SIE_DMA_FLUSH_DIR_TO_DEV,
} KDRV_SIE_DMA_FLUSH_DIR;

typedef enum {
	KDRV_SIE_DMA_FLUSH_BUFTYPE_NORMAL,
	KDRV_SIE_DMA_FLUSH_BUFTYPE_VDOBLK,
} KDRV_SIE_DMA_FLUSH_BUFTYPE;

#define KDRV_SIE_BCC_MAX_CLK_FREQ SIE_BCC_MAX_CLK_FREQ

#define KDRV_SIE_HW_SPT_DRAMOUT1_LOF DISABLE // dram output channel 1, only support ca data (no need for line_of_set)
#define KDRV_SIE_HW_SPT_DRAMOUT2_LOF DISABLE // dram output channel 2, only support la data (no need for line_of_set)

KDRV_SIE_STATUS kdrv_sie_get_state_machine(KDRV_SIE_PROC_ID id);
KDRV_SIE_INFO *kdrv_sie_get_hdl(KDRV_SIE_PROC_ID id);
BOOL kdrv_sie_chk_fmt(ULONG hdl, KDRV_SIE_DATA_FMT src, KDRV_SIE_INT_DATA_FMT chk);
BOOL kdrv_sie_chk_id_valid(KDRV_SIE_PROC_ID id);
INT32 kdrv_sie_get_sie_limit(KDRV_SIE_PROC_ID id, void *data);
KDRV_SIE_HDL_CONTEXT *kdrv_sie_get_ctx(void);
KDRV_SIE_CLK_HDL *kdrv_sie_get_clk_hdl(KDRV_SIE_PROC_ID id);
UINT32 kdrv_sie_get_apb_clk(void);

extern ULONG (*kdrv_sie_cert_func)(ULONG, UINT32, UINT32);

INT32 kdrv_sie_sys_init(UINT32 chip_num, UINT32 eng_num);
INT32 kdrv_sie_sys_uninit(void);

INT32 kdrv_sie_module_init(void);
INT32 kdrv_sie_module_uninit(void);

void *kdrv_sie_os_malloc_wrap(UINT32 want_size);
void kdrv_sie_os_mfree_wrap(void *p_buf);

void kdrv_sie_os_dma_flush(ULONG addr_va, UINT32 size, KDRV_SIE_DMA_FLUSH_DIR dir, KDRV_SIE_DMA_FLUSH_BUFTYPE type);

INT32 kdrv_sie_rtos_init(void);
INT32 kdrv_sie_rtos_uninit(void);

#endif //_KDRV_SIE_INT_H_
