/**
    Public header file for VIE utility

    This file is the header file that define the API and data type for VIE utility tool.

    @file      ctl_vie_utility.h
    @ingroup    mILibVIECom
    @note      Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2009.  All rights reserved.
*/
#ifndef _CTL_VIE_UTILITY_H_
#define _CTL_VIE_UTILITY_H_

#include "kwrap/type.h"
#include "kwrap/error_no.h"
#include "kflow_common/type_vdo.h"
#include "kflow_common/isp_if.h"
#include "ctl_sen.h"
#include "ctl_vie_event.h"
#include "ctl_vie_isp.h"

/**
    CTL VIE error type
*/
#define CTL_VIE_E_OK            (0)
#define CTL_VIE_E_ID            (-1)    //illegal vie handle
#define CTL_VIE_E_SYS           (-2)    //sysem error
#define CTL_VIE_E_HDL           (-3)    //null handle
#define CTL_VIE_E_NULL_FP       (-4)    //null fp
#define CTL_VIE_E_PAR           (-5)    //parameter error
#define CTL_VIE_E_NOSPT         (-6)    //not support function
#define CTL_VIE_E_TMOUT         (-7)    //time out
#define CTL_VIE_E_QOVR          (-8)    //queue overflow
#define CTL_VIE_E_NOMEM         (-9)    //no memory
#define CTL_VIE_E_STATE         (-10)   //illegal state
#define CTL_VIE_E_IPP           (-11)   //ipp error
#define CTL_VIE_E_KDRV_GET      (-100)  //kdrv get error
#define CTL_VIE_E_KDRV_SET      (-200)  //kdrv set error
#define CTL_VIE_E_KDRV_TRIG     (-300)  //kdrv trigger error
#define CTL_VIE_E_KDRV_CLOSE    (-400)  //kdrv close error
#define CTL_VIE_E_SEN           (-1000) //sensor error


#define CTL_VIE_PARAM_NONE          0xffffffff

#define CTL_VIE_RATIO(w, h) (((UINT32)(UINT16)(w) << 16) | (UINT32)(UINT16)(h))

/*
    for CTL_VIE default parameters, only mark [CTL_VIE_DFT] parameters can used
*/
#define CTL_VIE_DFT 0xffffcccc

/**
    VIE Engine ID
*/
typedef enum  {

	/* vie engine 0, video ch 0 ~ 3 */
	CTL_VIE_ID_1,   ///< process id 1
	CTL_VIE_ID_2,   ///< process id 2
	CTL_VIE_ID_3,   ///< process id 3
	CTL_VIE_ID_4,   ///< process id 4

	/* vie engine 1, video ch 0 ~ 3 */
	CTL_VIE_ID_5,   ///< process id 5
	CTL_VIE_ID_6,   ///< process id 6
	CTL_VIE_ID_7,   ///< process id 7
	CTL_VIE_ID_8,   ///< process id 8

	CTL_VIE_ID_MAX_NUM,
	ENUM_DUMMY4WORD(CTL_VIE_ID)
} CTL_VIE_ID;

/*
    dram channel
*/
typedef enum {
	CTL_VIE_DRAM_CH0,
	CTL_VIE_DRAM_CH1,
	CTL_VIE_DRAM_CHDBG,
	CTL_VIE_DRAM_CH_MAX,
	ENUM_DUMMY4WORD(CTL_VIE_DRAM_CH)
} CTL_VIE_DRAM_CH;

/**
    VIE flip information
*/
typedef enum {
	CTL_VIE_FLIP_NONE   = 0x00000000,        ///< no flip
	CTL_VIE_FLIP_H      = 0x00000001,        ///< H flip
	CTL_VIE_FLIP_V      = 0x00000002,        ///< V flip
	CTL_VIE_FLIP_H_V    = 0x00000003,        ///< H & V flip(Flip_H | Flip_V) don't modify
	CTL_VIE_FLIP_MAX,
	ENUM_DUMMY4WORD(CTL_VIE_FLIP_TYPE)
} CTL_VIE_FLIP_TYPE;

typedef enum {
	CTL_VIE_IOSIZE_AUTO = 0, ///< skip CTL_VIE_SIZE_INFO set information, ctl_vie auto gen vie crop size(according to CTL_VIE_IOSIZE_AUTO_INFO)
	CTL_VIE_IOSIZE_MANUAL,   ///< vie crop size according to CTL_VIE_SIZE_INFO
	CTL_VIE_IOSIZE_SEL_MAX,
	ENUM_DUMMY4WORD(CTL_VIE_IOSIZE_SEL)
} CTL_VIE_IOSIZE_SEL;

typedef struct {
	UINT32 ratio_h_v;           ///< hv ratio (H:bit[31:16], V:bit[15:0]), set [CTL_VIE_DFT] for sensor ori ratio
	UINT32 factor;              ///< max 1000, for no digital zoom effect (w = w * factor / 1000, h = h * factor / 1000)
	UPOINT crp_sft;                 ///< based on vie crop window, unit: pixel
} CTL_VIE_IOSIZE_AUTO_INFO;

typedef struct {
	URECT   vie_crp;        ///< vie crop window
	USIZE   vie_out;        ///< vie out size
	URECT   dest_crp;       ///< dest crop window
} CTL_VIE_SIZE_INFO;

typedef struct {
	CTL_VIE_IOSIZE_SEL iosize_sel;
	CTL_VIE_IOSIZE_AUTO_INFO auto_info;     ///< only CTL_VIE_IOSIZE_AUTO need to set
	CTL_VIE_SIZE_INFO size_info;            ///< ctl_vie_set only support when CTL_VIE_IOSIZE_MANUAL.
	USIZE align;                            ///< [OPTION] Must be aligned to a multiple of 4, set [CTL_VIE_DFT] for default align 4.
	USIZE dest_align;                       ///< destination align
} CTL_VIE_IO_SIZE_INFO;


typedef enum {
	CTL_VIE_PAT_REV = 1,      		///< N.S.
	CTL_VIE_PAT_RANDOM, 	        ///< Random, frame based
	CTL_VIE_PAT_FIXED,              ///< Fixed, set pixel value by pat_gen_val
	CTL_VIE_PAT_HINCREASE,          ///< 1D Increment, set pixel value by pat_gen_val and reset for every line head
	CTL_VIE_PAT_HVINCREASE,			///< 2D increment, set to line number count for every line head
	CTL_VIE_PAT_COLORBAR,    		///< N.S.
	ENUM_DUMMY4WORD(CTL_VIE_PATGEN_SEL)
} CTL_VIE_PATGEN_SEL;

/**
    vie pattern gen info
    Pattern Gen MCLK fixed to 240Mhz(520/525)
    pattern gen source window always start with cfa_r
    pattern gen frame time = 240Mhz / ((act_win.x + act_win_w + 8) * (act_win.y + act_win_h))
    CTL_VIE_PAT_COLORBAR: pat_gen_val = width of each color
    CTL_VIE_PAT_HINCREASE: pat_gen_val = 12bit value of start luminance
*/
typedef struct {
	URECT               crp_win;        //vie crop window
	CTL_VIE_PATGEN_SEL  pat_gen_mode;
	UINT32              pat_gen_val;
	UINT32              frame_rate;     //fps * 100
} CTL_VIE_PAG_GEN_INFO;

/**
    vie interrupt type
*/
typedef enum {
	CTL_VIE_CLR                 = 0,
	CTL_VIE_INTE_VD             = 0x00000001,   ///< enable interrupt: VD
	CTL_VIE_INTE_BP3            = 0x00000002,   ///< enable interrupt: Break point
	CTL_VIE_INTE_CRPST          = 0x00000004,   ///< enable interrupt: Crop window start
	CTL_VIE_INTE_DRAM_OUT0_END  = 0x00000008,   ///< enable interrupt: Dram output channel 0 end
	CTL_VIE_INTE_DRAM_OUT1_END  = 0x00000010,   ///< enable interrupt: Dram output channel 1 end
	CTL_VIE_INTE_CROPEND        = 0x00000020,   ///< enable interrupt: Crop window crop end
	CTL_VIE_INTE_DRAM_OUTD_END  = 0x00000040,   ///< enable interrupt: Dram output channel deubg end
	CTL_VIE_INTE_ALL            = 0xffffffff,   ///< enable interrupt: all
} CTL_VIE_INTE;

/**
    type for CTL_VIE_SET_DATAFORMAT
    1. YUV420 only support split output
    2. ycc encode only suppot split
*/
typedef enum {
	CTL_VIE_YUV_422_NOSPT,
	CTL_VIE_YUV_422_SPT,
	CTL_VIE_YUV_422_SPT_YCCENC,

	CTL_VIE_YUV_420_SPT,
	CTL_VIE_YUV_420_SPT_YCCENC,

	ENUM_DUMMY4WORD(CTL_VIE_DATAFORMAT)
} CTL_VIE_DATAFORMAT;

/**
    struct for ctl_vie_open
*/
typedef enum {
	CTL_VIE_FLOW_UNKNOWN = 0,
	CTL_VIE_FLOW_SEN_IN,    //sensor input
	CTL_VIE_FLOW_PATGEN,    //VIE pattern gen mode
	CTL_VIE_FLOW_SIG_DUPL,  //VIE Signal duplicate from dupl_id
	CTL_VIE_FLOW_MAX,
	ENUM_DUMMY4WORD(CTL_VIE_FLOW_TYPE)
} CTL_VIE_FLOW_TYPE;

/**
    vie clk source select
*/
typedef enum {
	CTL_VIE_CLKSRC_CURR,        ///< VIE clock source as current setting(no setting, no changing)
	CTL_VIE_CLKSRC_192,         ///< VIE clock source as 192MHz
	CTL_VIE_CLKSRC_320,         ///< VIE clock source as 320MHz
	CTL_VIE_CLKSRC_480,         ///< VIE clock source as 480MHz, PATGEN force use 480
	CTL_VIE_CLKSRC_PLL19,       ///< VIE clock source as PLL19
	CTL_VIE_CLKSRC_PLL23,       ///< VIE clock source as PLL123
	CTL_VIE_CLKSRC_MAX,
	CTL_VIE_CLKSRC_DEFAULT = CTL_VIE_CLKSRC_PLL23,    ///< VIE clock source use default setting
	ENUM_DUMMY4WORD(CTL_VIE_CLKSRC_SEL)
} CTL_VIE_CLKSRC_SEL;

typedef struct {
	CTL_VIE_ID          id;
	CTL_VIE_FLOW_TYPE   flow_type;
	CTL_VIE_ID          dupl_src_id;    //source duplicate id, valid only when flow_type is CTL_VIE_FLOW_SIG_DUPL
	UINT32          	sen_id;
	ISP_ID              isp_id;
	CTL_VIE_CLKSRC_SEL  clk_src_sel;    //select vie clock source
} CTL_VIE_OPEN_CFG;

/**
    struct for vie cb fp
*/
typedef struct {
	UINT32 sts;
	CTL_VIE_EVENT_FP cb_fp;
} CTL_VIE_CB_INFO;

/**
    type for CTL_VIE_CHGSENMODE_INFO
*/
typedef enum {
	CTL_VIE_OUTPUT_DEST_VIE1    = (1 << CTL_VIE_ID_1),
	CTL_VIE_OUTPUT_DEST_VIE2    = (1 << CTL_VIE_ID_2),
	CTL_VIE_OUTPUT_DEST_VIE3    = (1 << CTL_VIE_ID_3),
	CTL_VIE_OUTPUT_DEST_VIE4    = (1 << CTL_VIE_ID_4),
	CTL_VIE_OUTPUT_DEST_VIE5    = (1 << CTL_VIE_ID_5),
	CTL_VIE_OUTPUT_DEST_VIE6    = (1 << CTL_VIE_ID_6),
	CTL_VIE_OUTPUT_DEST_VIE7    = (1 << CTL_VIE_ID_7),
	CTL_VIE_OUTPUT_DEST_VIE8    = (1 << CTL_VIE_ID_8),
	CTL_VIE_OUTPUT_DEST_ALL     = 0xff,
	ENUM_DUMMY4WORD(CTL_VIE_OUTPUT_DEST)
} CTL_VIE_OUTPUT_DEST;

typedef struct {
	CTL_VIE_OUTPUT_DEST         output_dest;
	CTL_SEN_MODE                sen_mode;
} CTL_VIE_CHGSENMODE_INFO;

/**
    type for CTL_VIE_SET_CCIR
*/
typedef enum {
	CTL_VIE_FIELD_DISABLE = 0,
	CTL_VIE_FIELD_EN_0,
	CTL_VIE_FIELD_EN_1,
	ENUM_DUMMY4WORD(CTL_VIE_FIELD_SEL)
} CTL_VIE_FIELD_SEL;

typedef struct {
	CTL_VIE_FIELD_SEL field_sel;    ///< for CCIR field select
} CTL_VIE_CCIR_INFO;

/**
    type for CTL_VIE_MUX_DATA_INFO
*/
typedef struct {
	UINT8 data_idx; ///< select data idx 0/1 when mux sensor input
} CTL_VIE_MUX_DATA_INFO;

/**
    type for CTL_VIE_SET_SIGNAL
*/
typedef enum {
	CTL_VIE_PAD_SEL_OFF,
	CTL_VIE_PAD_SEL_A,
	CTL_VIE_PAD_SEL_B,
	CTL_VIE_PAD_SEL_AB,
	ENUM_DUMMY4WORD(CTL_VIE_PAD_SEL)
} CTL_VIE_PAD_SEL;

typedef enum {
	CTL_VIE_SMP_SINGLE_EDGE = 0,   // clock sampling with single edge of pxclk
	CTL_VIE_SMP_DUAL_EDGE,         // clock sampling with dual edge of pxclk
	ENUM_DUMMY4WORD(CTL_VIE_SMP_EDGE)
} CTL_VIE_SMP_EDGE;

typedef enum {
	CTL_VIE_PHASE_RISING = 0,
	CTL_VIE_PHASE_FALLING,
	ENUM_DUMMY4WORD(CTL_VIE_SIGNAL_PHASE)
} CTL_VIE_SIGNAL_PHASE;

typedef enum {
	CTL_VIE_DATA_SWITCH_MODE_NONE 	= 0x00UL,
	CTL_VIE_DATA_SWITCH_MODE_A 		= 0x01UL, // HI_LO_BYTE_SWAP	, input = [15:8][7:0], the swap result = [7:0][15:8]
	CTL_VIE_DATA_SWITCH_MODE_B 		= 0x02UL, // LO_BYTE_INV		, input = [15:8][7:0], the swap result = [15:8][0:7]
	CTL_VIE_DATA_SWITCH_MODE_C 		= 0x04UL, // HI_BYTE_INV		, input = [15:8][7:0], the swap result = [8:15][7:0]
	CTL_VIE_DATA_SWITCH_MODE_D 		= 0x08UL, // LO_4BIT_SWAP		, input = [15:12][11:8][7:4][3:0], the swap result = [15:12][11:8][3:0][7:4]
	CTL_VIE_DATA_SWITCH_MODE_E 		= 0x10UL, // HI_4BIT_SWAP		, input = [15:12][11:8][7:4][3:0], the swap result = [11:8][15:12][7:4][3:0]
	ENUM_DUMMY4WORD(CTL_VIE_DATA_SWITCH_MODE)
} CTL_VIE_DATA_SWITCH_MODE;

typedef struct {
	CTL_VIE_PAD_SEL         pad_sel;        // by channel, VIE pad selection
	CTL_VIE_SMP_EDGE        smp_edge;       // by engine
	BOOL                    b_vd_inverse;   // by engine
	BOOL                    b_hd_inverse;   // by engine
	CTL_VIE_SIGNAL_PHASE    vd_phase;       // by engine & CTL_VIE_PAD_SEL, only valid when smp_edge=CTL_VIE_SMP_SINGLE_EDGE
	CTL_VIE_SIGNAL_PHASE    hd_phase;       // by engine & CTL_VIE_PAD_SEL, only valid when smp_edge=CTL_VIE_SMP_SINGLE_EDGE
	CTL_VIE_SIGNAL_PHASE    data_phase;     // by engine & CTL_VIE_PAD_SEL, only valid when smp_edge=CTL_VIE_SMP_SINGLE_EDGE
	CTL_VIE_DATA_SWITCH_MODE data_switch_mode;
} CTL_VIE_SIGNAL;

/*
	MASK
*/
typedef enum {
	CTL_VIE_MASK_IDX0,
	CTL_VIE_MASK_IDX1,
	CTL_VIE_MASK_IDX2,
	CTL_VIE_MASK_IDX3,
	CTL_VIE_MASK_IDX_MAX,
	ENUM_DUMMY4WORD(CTL_VIE_MASK_IDX)
} CTL_VIE_MASK_IDX;

typedef struct {
	BOOL en;
	URECT win;
	UINT8 y;	// 8bits
	UINT8 cb;	// 8bits
	UINT8 cr;	// 8bits
} CTL_VIE_MASK_INFO;

typedef struct {
	CTL_VIE_MASK_INFO info[CTL_VIE_MASK_IDX_MAX];
} CTL_VIE_MASK;

/*
	CROP_MAX
*/
typedef struct {
	UINT32  w;
	UINT32  h;
} CTL_VIE_CROP_MAX;

/**
    vie trig type, start/stop vie
*/
typedef enum {
	CTL_VIE_TRIG_STOP = 0,      ///< Stop vie
	CTL_VIE_TRIG_START,         ///< Start vie
	ENUM_DUMMY4WORD(CTL_VIE_TRIG_TYPE)
} CTL_VIE_TRIG_TYPE;

typedef struct {
	CTL_VIE_TRIG_TYPE trig_type;
	UINT32 trig_frame_num;  ///< user define vie output frame, vie will stop after reach target frame, only effect when trig_type = CTL_VIE_TRIG_START
	///< set to 0xffffffff for continuous mode
	BOOL b_wait_end;        ///< wait trigger end
} CTL_VIE_TRIG_INFO;

/**
    type for CTL_VIE_SUS_RES_LVL
*/
typedef enum {
	CTL_VIE_SUS_RES_NONE    =   0x00000000, //SUSPEND: stop vie     /RESUME: start vie
	CTL_VIE_SUS_RES_PXCLK   =   0x00000001, //SUSPEND: disable pxclk/RESUME: enable pxclk
	CTL_VIE_SUS_RES_ALL     =   0xffffffff,
	ENUM_DUMMY4WORD(CTL_VIE_SUS_RES_LVL)
} CTL_VIE_SUS_RES_LVL;

/**
    type for CTL_VIE_DMA_STS
*/
typedef enum {
	CTL_VIE_DMA_STS_IDLE = 0,
	CTL_VIE_DMA_STS_RUN = 1,
	ENUM_DUMMY4WORD(CTL_VIE_DMA_STS)
} CTL_VIE_DMA_STS;

typedef enum {
#if 0 //KFLOW_VIE_READY
#endif
	CTL_VIE_ALG_TYPE_SYNC_FC, 	// simulation CTL_SIE_ALG_TYPE_SHDR, not ready
	CTL_VIE_ALG_TYPE_LOWLATENCY,//vcap out low latency mode, push buf out when bp3
	CTL_VIE_ALG_TYPE_MAX,
} CTL_VIE_ALG_TYPE;

typedef struct {
	CTL_VIE_ALG_TYPE type;
	BOOL func_en;
} CTL_VIE_ALG_FUNC;

/**
    VIE limitation infor
    sync to KDRV_VIE_LIMIT
*/
typedef struct {
	UINT32 max_clk_rate;        // maximum vie clock rate
	UINT32 max_spt_id;          // [for ctl layer] maximum vie engine * vie input video channel
	UINT32 max_spt_egine;       // maximum vie engine
	UINT32 max_spt_vdo_ch;      // maximum vie input video channel, per vie engine
	UINT32 max_dramout_ch;      // maximum dram output channel
	USIZE pat_gen_src_win_align;
	URECT crp_win_align_nosplit;      // crop window limit with split=0 (ycc_encode=0)
	URECT crp_win_align_split;        // crop window limit with split=1 (ycc_encode=0)
	URECT crp_win_align_split_yccenc; // crop window limit with ycc_encode=1 (ycc_encode=1 only support split)
	USIZE crp_win_min;
	USIZE crp_win_max;
	USIZE crp_win_max_four_k;
	UINT32 out_lofs_align[CTL_VIE_DRAM_CH_MAX];    //output lineoofset align, proposal[DRAM_OUT0_OFSO & DRAM_OUT1_OFSO & DRAM_OUT2_OFSO]
	UINT64 support_func;        //reference to KDRV_VIE_FUNC_SUPPORT or CTL_VIE_FUNC_SPT
	UINT32 support_output_fmt;  // KDRV_VIE_DATAFORMAT_SPT or CTL_VIE_DATAFORMAT_SPT
} CTL_VIE_LIMIT;

/**
    data for ctl_vie Set/Get Item
*/

typedef enum {
	CTL_VIE_ITEM_DATAFORMAT,        ///< [set/get],  data_type: CTL_VIE_DATAFORMAT
	CTL_VIE_ITEM_CHGSENMODE,        ///< [set/get],  data_type: CTL_VIE_CHGSENMODE_INFO
	CTL_VIE_ITEM_FLIP,              ///< [set/get],  data_type: CTL_VIE_FLIP_TYPE
	CTL_VIE_ITEM_CH0_LOF,           ///<     [get],  data_type: UINT32, need 4 byte align
	CTL_VIE_ITEM_CH1_LOF,           ///<     [get],  data_type: UINT32, need 4 byte align
	CTL_VIE_ITEM_IO_SIZE,           ///< [set/get],  data_type: CTL_VIE_IO_SIZE_INFO
	CTL_VIE_ITEM_PATGEN_INFO,       ///< [set/get],  data_type: CTL_VIE_PAG_GEN_INFO
	CTL_VIE_ITEM_CCIR,              ///< backward compatible
	CTL_VIE_ITEM_MUX_DATA,          ///< [set/get],  data_type: CTL_VIE_MUX_DATA_INFO
	CTL_VIE_ITEM_TRIG_IMM,          ///< [set],      data_type: CTL_VIE_TRIG_INFO
	CTL_VIE_ITEM_REG_CB_IMM,        ///< [set],      data_type: CTL_VIE_REG_CB_INFO
	CTL_VIE_ITEM_ALG_FUNC_IMM,      ///< [set/get],  data_type: CTL_VIE_ALG_FUNC
	CTL_VIE_ITEM_RESET_FC_IMM,      ///< [set],      data_type: NONE, reset frame counter(for shdr using)
	CTL_VIE_ITEM_DMA_ABORT,         ///< [set],      data_type: NONE, set DMA ABORT, need to check CTL_VIE_ITEM_DMA_STS, not need to load
	CTL_VIE_ITEM_DMA_STS,           ///< [N.S.   ],  data_type: CTL_VIE_DMA_STS, get dma status
	CTL_VIE_ITEM_MASK,				///< [set/get],  data_type: CTL_VIE_MASK
	CTL_VIE_ITEM_MAX_CROP,			///< [set/get],  data_type: CTL_VIE_CROP_MAX, cannot runtime change
	CTL_VIE_ITEM_LOAD,              ///< [set],      data_type: NULL or CTL_VIE_SET_ITEM
	CTL_VIE_ITEM_MAX,
	ENUM_DUMMY4WORD(CTL_VIE_ITEM)
} CTL_VIE_ITEM;

//@}
#endif //_CTL_VIE_UTILITY_H_
