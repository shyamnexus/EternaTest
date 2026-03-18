/**
    Header file for KDRV TGE module

    This file is the header file that define the API for KDRV TGE.

    @file       kdrv_tge.h
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2017.  All rights reserved.
*/

#ifndef _KDRV_TGE_H
#define _KDRV_TGE_H

#include "kwrap/type.h"
#ifdef __KERNEL__
#include <linux/types.h>
#endif
#include "kdrv_type.h"

#define KDRV_TGE_VDHD_CH_MAX 	4	// max vdhd channel number (KDRV_TGE_VDHD_CH)
#define KDRV_TGE_SWAP_CH_MAX 	2	// (NoSup) max vdhd channel number that can be swap (KDRV_TGE_SWAP_FUNC_ID)
#define KDRV_TGE_SIE_SRC_CH_MAX	2	// (NoSup) max sie that vdhd source can be sent from tge (KDRV_TGE_SIE_IN_ID)
#define KDRV_TGE_CLK_ID_MAX 	5	// (NoSup) max tge clock source have been used (KDRV_TGE_CLK_ID)
#define KDRV_TGE_FLSH_ID_MAX 	1	// max flashlight number have been used (KDRV_TGE_FLSH_ID)
#define KDRV_TGE_MSH_ID_MAX 	2	// max mechanical-shutter number have been used (KDRV_TGE_MSH_ID). one msh set has two ID(close/open)

typedef enum {
	KDRV_TGE_CH_SFT0, // VD0/HD0 generator shift & idx
	KDRV_TGE_CH_SFT1, // VD0/HD1 generator shift & idx
	KDRV_TGE_CH_SFT2, // VD0/HD2 generator shift & idx
	KDRV_TGE_CH_SFT3, // VD0/HD3 generator shift & idx
	KDRV_TGE_CH_SFT4, // VD0/HD4 generator shift & idx
	KDRV_TGE_CH_SFT5, // VD0/HD5 generator shift & idx
	KDRV_TGE_CH_SFT6, // VD0/HD6 generator shift & idx
	KDRV_TGE_CH_SFT7, // VD0/HD7 generator shift & idx
	KDRV_TGE_CH_SFT_MAX,
} KDRV_TGE_CH_SFT;

typedef enum {
	KDRV_TGE_VDHD_CH1 = (1 << KDRV_TGE_CH_SFT0),
	KDRV_TGE_VDHD_CH2 = (1 << KDRV_TGE_CH_SFT1),
	KDRV_TGE_VDHD_CH3 = (1 << KDRV_TGE_CH_SFT2),
	KDRV_TGE_VDHD_CH4 = (1 << KDRV_TGE_CH_SFT3),
	KDRV_TGE_VDHD_CH5 = (1 << KDRV_TGE_CH_SFT4),
	KDRV_TGE_VDHD_CH6 = (1 << KDRV_TGE_CH_SFT5),
	KDRV_TGE_VDHD_CH7 = (1 << KDRV_TGE_CH_SFT6),
	KDRV_TGE_VDHD_CH8 = (1 << KDRV_TGE_CH_SFT7),
} KDRV_TGE_VDHD_CH;

typedef enum {						// (530 NoSup)
	KDRV_TGECLK1 = (1 << 0),
	KDRV_TGECLK2 = (1 << 1),
	KDRV_TGECLK3 = (1 << 2),
	KDRV_TGECLK4 = (1 << 3),
	KDRV_TGECLK5 = (1 << 4),
	KDRV_TGECLK6 = (1 << 5),
	KDRV_TGECLK7 = (1 << 6),
	KDRV_TGECLK8 = (1 << 7),
} KDRV_TGE_CLK_ID;

typedef enum {						// (520 NoSup)
	KDRV_TGE_SWAP_CH15 = (1 << 0),
	KDRV_TGE_SWAP_CH26 = (1 << 1),
	KDRV_TGE_SWAP_CH37 = (1 << 2),
	KDRV_TGE_SWAP_CH48 = (1 << 3),
} KDRV_TGE_SWAP_FUNC_ID;

typedef enum {						// (520 NoSup)
	KDRV_TGE_SIE1_IN = (1 << 0), 	// Only used in parallel + slave sensor
	KDRV_TGE_SIE3_IN = (1 << 1),	// Only used in parallel + slave sensor
} KDRV_TGE_SIE_IN_ID;

typedef enum {
	KDRV_TGE_FLSH_1 = (1 << 0),
} KDRV_TGE_FLSH_ID;

typedef enum {
	KDRV_TGE_MSH_1_CLOSE = (1 << 0),
	KDRV_TGE_MSH_1_OPEN = (1 << 1),
} KDRV_TGE_MSH_ID;

typedef enum {
	TGE_WAIT_VD 		= (1 << 0),
	TGE_WAIT_VD2 		= (1 << 1),
	TGE_WAIT_VD3 		= (1 << 2),
	TGE_WAIT_VD4 		= (1 << 3),
	TGE_WAIT_VD5 		= (1 << 4),
	TGE_WAIT_VD6 		= (1 << 5),
	TGE_WAIT_VD7 		= (1 << 6),
	TGE_WAIT_VD8 		= (1 << 7),
	TGE_WAIT_VD_BP1 	= (1 << 8),
	TGE_WAIT_VD2_BP1 	= (1 << 9),
	TGE_WAIT_VD3_BP1 	= (1 << 10),
	TGE_WAIT_VD4_BP1 	= (1 << 11),
	TGE_WAIT_VD5_BP1 	= (1 << 12),
	TGE_WAIT_VD6_BP1 	= (1 << 13),
	TGE_WAIT_VD7_BP1 	= (1 << 14),
	TGE_WAIT_VD8_BP1 	= (1 << 15),
	TGE_WAIT_FLSH_TRG      = (1 << 16),
	TGE_WAIT_MSH_CLOSE_TRG = (1 << 17),
	TGE_WAIT_MSH_OPEN_TRG  = (1 << 18),
	//19~20
	TGE_WAIT_FLSH_END      = (1 << 21),
	TGE_WAIT_MSH_CLOSE_END = (1 << 22),
	TGE_WAIT_MSH_OPEN_END  = (1 << 23),
} TGE_WAIT_EVENT_SEL;

typedef enum {
	KDRV_TGE_INT_VD1 		= (1 << 0),
	KDRV_TGE_INT_VD2 		= (1 << 1),
	KDRV_TGE_INT_VD3 		= (1 << 2),
	KDRV_TGE_INT_VD4 		= (1 << 3),
	KDRV_TGE_INT_VD5 		= (1 << 4),
	KDRV_TGE_INT_VD6 		= (1 << 5),
	KDRV_TGE_INT_VD7 		= (1 << 6),
	KDRV_TGE_INT_VD8 		= (1 << 7),
	KDRV_TGE_INT_BP1 		= (1 << 8),
	KDRV_TGE_INT_BP2 		= (1 << 9),
	KDRV_TGE_INT_BP3 		= (1 << 10),
	KDRV_TGE_INT_BP4 		= (1 << 11),
	KDRV_TGE_INT_BP5 		= (1 << 12),
	KDRV_TGE_INT_BP6 		= (1 << 13),
	KDRV_TGE_INT_BP7 		= (1 << 14),
	KDRV_TGE_INT_BP8 		= (1 << 15),
	KDRV_TGE_INT_FLSH_TRG 	= (1 << 16),
	KDRV_TGE_INT_MSH_CLOSE_TRG = (1 << 17),
	KDRV_TGE_INT_MSH_OPEN_TRG  = (1 << 18),
	//19~20
	KDRV_TGE_INT_FLSH_END 	= (1 << 21),
	KDRV_TGE_INT_MSH_CLOSE_END = (1 << 22),
	KDRV_TGE_INT_MSH_OPEN_END  = (1 << 23)
} KDRV_TGE_ISR_EVENT;

typedef void (*KDRV_TGE_ISRCB)(KDRV_TGE_ISR_EVENT, void*);

/**
	sie kdrv trig type, start/stop sie
*/
typedef enum {
	KDRV_TGE_TRIG_UNKNOWN,
	KDRV_TGE_TRIG_VDHD,  			///< VD/HD Generator
    KDRV_TGE_TRIG_VDHD_STOP,		///< VD/HD stop
    KDRV_TGE_TRIG_FLSH_IMD,			///< flash light immediately
    KDRV_TGE_TRIG_FLSH_WAT_VD,		///< flash light wait until next vd
    KDRV_TGE_TRIG_FLSH_WAT_EXT,		///< flash light wait until next vd then wait external signal (note. vd only used to load parameter, not for sync flashlight signal)
    KDRV_TGE_TRIG_FLSH_WAT_CONT,	///< flash light wait until next vd and continuously trigger every N vd
    KDRV_TGE_TRIG_FLSH_STOP,		///< flash light stop

	KDRV_TGE_TRIG_MSH_IMD,			///< Mechanical-Shutter immediately
    KDRV_TGE_TRIG_MSH_WAT_VD,		///< Mechanical-Shutter wait until next vd
    KDRV_TGE_TRIG_MSH_WAT_EXT,		///< Mechanical-Shutter until next vd then wait external signal (note. vd only used to load parameter, not for sync flashlight signal)
    KDRV_TGE_TRIG_MSH_WAT_CONT,		///< Mechanical-Shutter wait until next vd and continuously trigger every N vd
    KDRV_TGE_TRIG_MSH_STOP,			///< Mechanical-Shutter stop

    KDRV_TGE_TRIG_MAX,

	// [deprecated] backward compatible
	KDRV_TGE_TRIG_STOP = KDRV_TGE_TRIG_VDHD_STOP,
} KDRV_TGE_TRIG_TYPE;

typedef enum {
	TGE_RESERVED = 0,
} KDRV_TGE_TRIGGER_PARAM;

typedef struct {
	INT32 (*callback)(void *callback_info, void *user_data);
	INT32 (*reserve_buf)(ULONG phy_addr);
	INT32 (*free_buf)(ULONG phy_addr);
} KDRV_TGE_CALLBACK_FUNC;

/**
    TGE clock source
*/
typedef enum {
	KDRV_TGE_CLK_PCLK = 0,			///< (NoSup) both tge clock and flash clock use mclk
	KDRV_TGE_CLK_MCLK1,
	KDRV_TGE_CLK_MCLK2,
	KDRV_TGE_CLK_MCLK3,
	KDRV_TGE_CLK_MCLK4,
	KDRV_TGE_CLK_MCLK5,				///< (NoSup)
	KDRV_TGE_CLK_MAX,
} KDRV_TGE_CLK_SRC;

typedef struct {
	KDRV_TGE_CLK_SRC clk_src_info;
} KDRV_TGE_CLK_SRC_SEL;

/**
    KDRV TGE structure - Engine open object
*/
typedef struct {						// (530 NoSup)
	KDRV_TGE_CLK_SRC tge_clock_sel;		///< (NoSup) Engine clock selection. 530 use fix clock
	KDRV_TGE_CLK_SRC tge_clock_sel2;	///< (NoSup) Engine clock selection. 530 use fix clock
} KDRV_TGE_OPENCFG;

/**
    TGE VD HD signal I/O
*/
typedef enum {
	KDRV_MODE_MASTER = 0, 			///< output by TGE generator
	KDRV_MODE_SLAVE_TO_PAD,			///< (NoSup) input by parallel sensor
	KDRV_MODE_SLAVE_TO_CSI,			///< (NoSup) input by CSI sensor
	KDRV_MODE_SLAVE_TO_SLVSEC,		///< (NoSup) input by slvsec sensor
	KDRV_MODE_MAX,
} KDRV_TGE_MODE_SEL;

/**
    TGE latch parameters in VD edge rising or falling
*/
typedef enum {
	KDRV_TGE_PHASE_RISING  = 0, 	///< rising edge latch/trigger
	KDRV_TGE_PHASE_FALLING, 		///< falling edge latch/trigger
	KDRV_TGE_PHASE_MAX,
} KDRV_TGE_PHASE_SEL;

/**
    KDRV TGE VD/HD structure - VD HD signal waveform
*/
typedef struct {
    KDRV_TGE_MODE_SEL mode;
	UINT32 vd_period;				///< range: 0 ~ 0xFFFFFFF(268435455)
	UINT32 vd_assert;				///< range: 0 ~ 0xFFFFFFF(268435455)
	UINT32 vd_frontblnk;			///< range: 0 ~ 0xFFFFFFF(268435455)
	UINT32 hd_period;				///< range: 0 ~ 0xFFFF(65535)
	UINT32 hd_assert;				///< range: 0 ~ 0xFFFF(65535)
	UINT32 hd_cnt;					///< range: 0 ~ 0x7FFFF(524287)
	KDRV_TGE_PHASE_SEL vd_phase; 	///< latch vd by rising edge or falling edge
    KDRV_TGE_PHASE_SEL hd_phase; 	///< latch hd by rising edge or falling edge
    BOOL vd_inverse; 				///< if TGE is master, (inverse = false) is vd_assert = low, (inverse = true) is vd_assert = high. if TGE is slave, just invert the input signal
    BOOL hd_inverse;
} KDRV_TGE_VDHD_INFO;

typedef struct {
	UINT32 bp_line; 				///< range: 0 ~ 0xFFFFFFF(268435455)
} KDRV_TGE_BP_INFO;

typedef struct {
	BOOL swap_enble;
} KDRV_TGE_SWAP_INFO;

/************************ flashlight trigger timing ****************************
trig cmd:           v
ext sig:            |      v                                 v
VD:           _|-|__|______|_____________________|-|_________|_____________________|-|_|-|_..._|-|______________________
                    |      |                     |           |                                 |
IMD           ______|______|__|-------------|___ |           |                                 |
                    |<- dly ->|<- assert  ->|    |           |                                 |
                           |                     |           |                                 |
WAT_VD        _____________|_____________________|_________|-------------|___                  |
                           |                     |<- dly ->|<- assert  ->|                     |
                           |                     |           |                                 |
WAT_EXT       _____________|_____________________|___________|_________|-------------|___      |
                                                 |           |<- dly ->|<- assert  ->|         |
                                                 |                                             |
WAT_CONT      ___________________________________|_________|-------------|_________________..._|_________|-------------|_
                                                 |<- dly ->|<- assert  ->|                     |<- dly ->|<- assert  ->|

*) Note that WAT_EXT flashlight signal don't sync with sie vd. vd only used to load parameter
   In WAT_EXT mode, operating order is: trigger tge -> wait sie vd -> trigger external signal -> flashlight on
*******************************************************************************/
typedef struct {
	KDRV_TGE_CLK_SRC clk_src;	///< input clock source (only support mclk), use to convert time length to tge count. if clk_src is disable while set, kdrv will do enable and set clock rate as 27M
	UINT64 delay;				///< delay length in us (delay is applied for every trigger type, including KDRV_TGE_TRIG_FLSH_IMD)
	UINT64 assert;				///< assert length in us
	UINT32 period;				///< range: 0 ~ 0xFF(255). period in vd count (only for KDRV_TGE_TRIG_FLSH_WAT_CONT). 0~255 (period) means 1~256 (vd counts)

	BOOL ctrl_pin_signal_inv;	///< FLSH_CTRL_INV
	BOOL ext_trig_pin_signal_inv;///<FLSH_EXT_TRG_INV
} KDRV_TGE_FLSH_INFO;

typedef enum {
	KDRV_TGE_FLSH_VD_SRC_SIE1,	///< refer SIE1 VD to be the sync signal of flashlight
	KDRV_TGE_FLSH_VD_SRC_SIE2,	///< refer SIE2 VD to be the sync signal of flashlight
	KDRV_TGE_FLSH_VD_SRC_SIE3,	///< refer SIE3 VD to be the sync signal of flashlight
	KDRV_TGE_FLSH_VD_SRC_SIE4,	///< refer SIE4 VD to be the sync signal of flashlight
	KDRV_TGE_FLSH_VD_SRC_SIE5,	///< refer SIE5 VD to be the sync signal of flashlight
	KDRV_TGE_FLSH_VD_SRC_MAX,
} KDRV_TGE_FLSH_VD_SRC;

typedef struct {
	KDRV_TGE_FLSH_VD_SRC vd_src;	///< flashlight input sync source
} KDRV_TGE_FLSH_VD_SRC_INFO;

typedef struct {
	KDRV_TGE_CLK_SRC clk_src;	///< input clock source (only support mclk), use to convert time length to tge count. if clk_src is disable while set, kdrv will do enable and set clock rate as 27M
	UINT64 delay;			///< delay length in us (delay is applied for every trigger type, including KDRV_TGE_TRIG_FLSH_IMD);
	UINT64 assert;		///< assert length in us;
	UINT32 period;		///< range: 0 ~ 0xFF(255). period in vd count (only for KDRV_TGE_TRIG_FLSH_WAT_CONT). 0~255 (period) means 1~256 (vd counts)
} KDRV_TGE_MSH_INFO;

typedef struct {
	BOOL ctrl_pin_signal_inv;	///< MSH_CTRL_INV: set 1 inverting mechanical-shutter control wire
	BOOL swap_ctrl_pin;			///< MSH_SWAP: set 1 swap mechanical-shutter control two wires
	BOOL ctrl_mode_pin;			///< Mechanical-shutter control-mode selection.(0: Pulse mode 1: Level mode)
	BOOL ext_trig_pin_signal_inv;///< MSH_EXT_TRG_INV: set 1 inverting flashlight external trigger wire
} KDRV_TGE_MSH_PIN_CTRL;

typedef enum {
	KDRV_TGE_MSH_VD_SRC_SIE1,	///< refer SIE1  VD to be the sync signal of Mechanical-Shutter
	KDRV_TGE_MSH_VD_SRC_SIE2,	///< refer SIE2  VD to be the sync signal of Mechanical-Shutter
	KDRV_TGE_MSH_VD_SRC_SIE3,	///< refer SIE3  VD to be the sync signal of Mechanical-Shutter
	KDRV_TGE_MSH_VD_SRC_SIE4,	///< refer SIE4  VD to be the sync signal of Mechanical-Shutter
	KDRV_TGE_MSH_VD_SRC_SIE5,	///< refer SIE5  VD to be the sync signal of Mechanical-Shutter
	KDRV_TGE_MSH_VD_SRC_MAX,
} KDRV_TGE_MSH_VD_SRC;


typedef struct {
	KDRV_TGE_MSH_VD_SRC vd_src;	///< Mechanical-Shutter input sync source
} KDRV_TGE_MSH_VD_SRC_INFO;


typedef struct {
	KDRV_TGE_TRIG_TYPE trig_type; 	///< trigger operation
	BOOL ch_enable; 				///< change VD/HD channel enable/disable in this trigger (trig_type: KDRV_TGE_TRIG_VDHD)
	BOOL wait_end_enable; 			///< set if VD/HD trigger cmd will wait until wait_event occur when ch_enable=1 (flash don't wait vd, because flash use vd from sie, not tge)
    TGE_WAIT_EVENT_SEL wait_event;	///< set VD/HD trigger wait event when wait_end_enable=1
} KDRV_TGE_TRIG_INFO;

typedef struct {
	BOOL vd_pause;
	BOOL hd_pause;
} KDRV_TGE_TIMING_PAUSE_INFO;

/**
    SIE 1/3 VD/HD signal source (only in parallel mode + slave sensor)
*/
typedef enum _KDRV_TGE_SIE_VD_SRC {
	KDRV_TGE_SIE1_VD_SRC_CH1 = 0, 	// use ch1 to be SIE1 VD source
	KDRV_TGE_SIE1_VD_SRC_CH3, 		// use ch3 to be SIE1 VD source
	KDRV_TGE_SIE3_VD_SRC_CH5, 		// use ch5 to be SIE3 VD source
	KDRV_TGE_SIE3_VD_SRC_CH7, 		// use ch7 to be SIE3 VD source
} KDRV_TGE_SIE_VD_SRC;

typedef struct {
    KDRV_TGE_SIE_VD_SRC vd_src;
} KDRV_TGE_SIE_VD_INFO;

/**
    TGE KDRV ITEM
*/
typedef enum {
	KDRV_TGE_PARAM_IPL_OPENCFG, 				///< (530 NoSup)
	KDRV_TGE_PARAM_IPL_VDHD, 					///< [Set/Get] 	id: KDRV_TGE_VDHD_CH, 		data_type: KDRV_TGE_VDHD_INFO, 			VD/HD generator's parameters
	KDRV_TGE_PARAM_IPL_VD_BP, 					///< [Set/Get] 	id: KDRV_TGE_VDHD_CH, 		data_type: KDRV_TGE_BP_INFO, 			the break-point line to change TGE's interrupt timing
	KDRV_TGE_PARAM_IPL_PAUSE,					///< [Set/Get] 	id: KDRV_TGE_VDHD_CH, 		data_type: KDRV_TGE_TIMING_PAUSE_INFO, 	pause VD/HD counter
	KDRV_TGE_PARAM_IPL_SWAP, 					///< (520 NoSup)
	KDRV_TGE_PARAM_IPL_SIE_VDHD_SRC, 			///< (520 NoSup)
	KDRV_TGE_PARAM_IPL_TGE_CLK_SRC, 			///< (530 NoSup) (use KDRV_TGE_PARAM_IPL_MCLK_SEL)
	KDRV_TGE_PARAM_IPL_FLSH_CTRL,				///< [Set/Get]  id: KDRV_TGE_FLSH_ID, 		data_type: KDRV_TGE_FLSH_INFO, 			flash-light control
	KDRV_TGE_PARAM_IPL_FLSH_VD_SRC,				///< [Set/Get]  id: KDRV_TGE_FLSH_ID, 		data_type: KDRV_TGE_FLSH_VD_SRC_INFO,	flash-light reference VD source from which SIE
	KDRV_TGE_PARAM_IPL_MSH_CTRL,				///< [Set/Get]  id: KDRV_TGE_MSH_ID, 		data_type: KDRV_TGE_MSH_INFO, 			mechanical-shutter control
	KDRV_TGE_PARAM_IPL_MSH_PIN_CTRL,			///< [Set/Get]  id: KDRV_TGE_MSH_ID, 		data_type: KDRV_TGE_MSH_PIN_CTRL,
	KDRV_TGE_PARAM_IPL_MSH_VD_SRC,				///< [Set/Get]  id: KDRV_TGE_MSH_ID, 		data_type: KDRV_TGE_FLSH_VD_SRC_INFO,
	KDRV_TGE_PARAM_IPL_SET_ISR_CB, 				///< [Set] 		id: NA, 					data_type: KDRV_TGE_ISRCB, 				set tge external isr cb
	KDRV_TGE_PARAM_IPL_MCLK_SEL, 				///< [Set] 		id: KDRV_TGE_VDHD_CH, 		data_type: KDRV_TGE_CLK_SRC_SEL,		set tge clock source of vdhd channel n
	KDRV_TGE_PARAM_IPL_RESET_VD,				///< [Set] 		id: KDRV_TGE_VDHD_CH
	KDRV_TGE_PARAM_MAX, 						///<
} KDRV_TGE_PARAM_ID;


extern INT32 kdrv_tge_init(void);	// rtos only. linux use kdrv_tge_platform_create_resource()
extern INT32 kdrv_tge_uninit(void);
extern INT32 kdrv_tge_open(UINT32 chip, UINT32 engine);
extern INT32 kdrv_tge_close(UINT32 chip, UINT32 engine);
extern INT32 kdrv_tge_set(UINT32 id, KDRV_TGE_PARAM_ID item, void* data);
extern INT32 kdrv_tge_get(UINT32 id, KDRV_TGE_PARAM_ID item, void* data);
extern INT32 kdrv_tge_trigger(UINT32 id, KDRV_TGE_TRIGGER_PARAM *p_rpc_param, KDRV_TGE_CALLBACK_FUNC *p_cb_func, void *p_user_data);
extern void kdrv_tge_dump_info(int (*dump)(const char *fmt, ...));
int kdrv_tge_module_printout_handler(uintptr_t data);

#endif


