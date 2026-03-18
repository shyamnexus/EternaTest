/*
    DAI internal header file

    This file is the header file that define register for DAI module


    @file       dai_int.h
    @ingroup    mISYSAud
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2016.  All rights reserved.
*/


#ifndef _AUDIO_INT_H
#define _AUDIO_INT_H

#ifdef __KERNEL__
#include "kwrap/util.h"
#include "Audio.h"
#include "dai.h"
#include "eac.h"
//#include "audio_dbg.h"
#define dma_flushWriteCache(parm,parm2)
#define dma_flushReadCache(parm,parm2)
#define Delay_DelayMs(ms) vos_util_delay_ms(ms)
#define Delay_DelayUs(us) vos_util_delay_us(us)
#elif defined(__FREERTOS)
#define __MODULE__ rtos_audio
#define __DBGLVL__ 8 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
#define __DBGFLT__ "*"


#include "kwrap/flag.h"
#include "kwrap/debug.h"
#include "kwrap/util.h"

#include "Audio.h"
#include "dai.h"
#include "eac.h"
#include "pll_protected.h"
#include "dma_protected.h"
#include <string.h>
#include "comm/timer.h"

#define Delay_DelayMs(ms) delay_us(ms*1000)
#define Delay_DelayUs(us) delay_us(us)

#else
#include "Debug.h"
#include "pll.h"
#include "pll_protected.h"
#include "eac.h"
#endif
#include "kwrap/spinlock.h"
#include "kwrap/error_no.h"
#include <nvt_api_ver.h>


// Default debug level
#ifndef __DBGLVL__
#define __DBGLVL__  2       // Output all message by default. __DBGLVL__ will be set to 1 via make parameter when release code.
#endif

// Default debug filter
#ifndef __DBGFLT__
#define __DBGFLT__  "*"     // Display everything when debug level is 2
#endif

#ifndef _EMULATION_
#define _EMULATION_ 0 // no emulation macro in linux, manually define here.
#endif

#if defined(CONFIG_NVT_FPGA_EMULATION) || defined(_NVT_FPGA_)
#define _FPGA_EMULATION_ 1
#else
#define _FPGA_EMULATION_ 0
#endif

// Audio / kdrv_audioio / audio_tsx 
extern vk_spinlock_t aud_my_lock_rx;
extern vk_spinlock_t aud_my_lock_tx1;
extern vk_spinlock_t aud_my_lock_tx2;
extern vk_spinlock_t aud_my_lock_qu;
extern vk_spinlock_t aud_my_lock_txlb;

//extern void loc_cpu(myflags)   vk_spin_lock_irqsave(&mylock,vk_spinlock_t myflags);
//extern void unl_cpu(myflags)   vk_spin_unlock_irqrestore(&mylock,vk_spinlock_t myflags);


extern BOOL (*audio_drv_init_valid)(UINT32 value);
extern NVT_API_CHK_DECLARE(audio_drv);

extern UINT32          uibuffer_insert_number_tx1;
extern UINT32          uibuffer_insert_number_tx2;
extern UINT32          uibuffer_insert_number_rx;
extern UINT32          uibuffer_insert_number_txlb;

//#include "DebugModule.h"


/*
    @addtogroup mISYSAud
*/
//@{

/* Audio Buffer Queue Number */
#define AUDIO_BUF_BLKNUM                0x00000005

/* Driver implementation Max Codec Number */
#define AUDIO_MAX_CODEC_NUM             (2)

/* Driver internal usage interrupt define*/
#define AUDIO_RX_FLAGALL                (DAI_RXSTOP_INT|DAI_RX1BWERR_INT|DAI_RX2BWERR_INT|DAI_RX1DMADONE_INT|DAI_RX2DMADONE_INT|DAI_RXTCHIT_INT|DAI_RXTCLATCH_INT|DAI_RXDMALOAD_INT)
#define AUDIO_TX1_FLAGALL               (DAI_TX1DMADONE_INT|DAI_TX1DMALOAD_INT|DAI_TX1STOP_INT|DAI_TX1BWERR_INT|DAI_TX1TCHIT_INT)
#define AUDIO_TX2_FLAGALL               (DAI_TX2DMADONE_INT|DAI_TX2DMALOAD_INT|DAI_TX2STOP_INT|DAI_TX2BWERR_INT)

/* Identification Flag to mean use default codec API for HDMI Codec */
#define AUDIO_CODEC_FUNC_USE_DEFAULT    (0xE0000000)


#define dim(array) (sizeof(array)/sizeof(array[0]))

//
// MPLL setting
//
#if 1 //for 530
#define AUDIO_MPLL_44P1K_RATIO          (0xB07238) // 338.688 MHZ
#define AUDIO_MPLL_48K_RATIO            (0x3D70A3) // 368.64  MHZ
#endif

#define AUDIO_MPLL_44P1K_SRCFREQ        (12000000ll*AUDIO_MPLL_44P1K_RATIO/131072)
#define AUDIO_MPLL_48K_SRCFREQ          (12000000ll*AUDIO_MPLL_48K_RATIO/131072)

#if _FPGA_EMULATION_
#define FPGA_OSC_RATE  12000 // = (12MHz)
#if defined __KERNEL__
#define FPGA_CLK_RATIO(x) x
#else
#define FPGA_CLK_RATIO(x) x * 344064ll / FPGA_OSC_RATE
#endif
#endif


#if 1//_EMULATION_
#define AUDIO_EAC_GAIN_MONITOR          DISABLE
#define AUDIO_DAI_ADDR_MONITOR          DISABLE
#else
#define AUDIO_EAC_GAIN_MONITOR          DISABLE
#define AUDIO_DAI_ADDR_MONITOR          DISABLE
#endif

/*
    Audio Tranceive Channels ID
*/

typedef enum {
	TSRX1,                          // Record Channel 1
	TSRX2,                          // Record Channel 1
	TSTXLB,                         // Playback Loopback Channel

	TSTX1,                          // Playback Channel 1
	TSTX2,                          // Playback Channel 2

	AUDTSCH_TOT,
	ENUM_DUMMY4WORD(AUDTSCH)
} AUDTSCH;

/*
    Audio operation state
*/

typedef enum {
	AUDIO_STATE_IDLE = 0x0,         // Idle state
	AUDIO_STATE_TX1 = 0x1,          // Playback 1 state
	AUDIO_STATE_TX2 = 0x2,          // Playback 2 state
	AUDIO_STATE_RX = 0x4,           // Record state
	AUDIO_STATE_TXLB = 0x8,           // Record state

    AUDIO_STATE_TX_START = 0x1,      	// Playback start state
	AUDIO_STATE_TX_STOP  = 0x2,			// Playback stop state
	AUDIO_STATE_RX_START = 0x10,        // Record start state
	AUDIO_STATE_RX_STOP  = 0x20,        // Record stop state

	ENUM_DUMMY4WORD(AUDIO_STATE)
} AUDIO_STATE;


/*
    Audio Internal Flag
*/
typedef enum {
	AUDIO_INT_FLAG_NONE             = 0x00000000,
	AUDIO_INT_FLAG_LINEPWR_ALWAYSON = 0x00000001,
	AUDIO_INT_FLAG_HPPWR_ALWAYSON   = 0x00000002,
	AUDIO_INT_FLAG_SPKPWR_ALWAYSON  = 0x00000004,

	AUDIO_INT_FLAG_AUTOPINMUX       = 0x00000010,
	AUDIO_INT_FLAG_MCLKAUTOPINMUX   = 0x00000020,

	AUDIO_INT_FLAG_CLKALWAYS_ON     = 0x00000100,

	ENUM_DUMMY4WORD(AUDIO_INT_FLAG)
} AUDIO_INT_FLAG;


/*
    Audio Default Settings Struct
*/
typedef struct {
	//
	//  Record Related
	//

	/* aud_set_feature() part: ALC & Boost default are enabled. */
	BOOL    BoostCompEn;    // AUDIO_FEATURE_NG_WITH_MICBOOST
	BOOL    NoiseGateEn;    // AUDIO_FEATURE_NOISEGATE_EN

	/* aud_set_parameter() part: */
	UINT32  BoostGain;      // AUDIO_PARAMETER_BOOST_GAIN
	UINT32  AlcMaxGain;     // AUDIO_PARAMETER_ALC_MAXGAIN
	UINT32  AlcMinGain;     // AUDIO_PARAMETER_ALC_MINGAIN
	UINT32  AlcAttack;      // AUDIO_PARAMETER_ALC_ATTACK_TIME
	UINT32  AlcDecay;       // AUDIO_PARAMETER_ALC_DECAY_TIME
	UINT32  AlcHold;        // AUDIO_PARAMETER_ALC_HOLD_TIME
	UINT32  AlcTimeReso;    // AUDIO_PARAMETER_ALC_TIME_RESOLUTION
	UINT32  NgTimeReso;     // AUDIO_PARAMETER_NOISEGATE_TIME_RESOLUTION
	UINT32  NgTHD;          // AUDIO_PARAMETER_NOISETHD_WITH_BOOST
	UINT32  NgTarget;       // AUDIO_PARAMETER_NOISEGAIN
	UINT32  NgAttack;       // AUDIO_PARAMETER_ALCNG_ATTACK_TIME
	UINT32  NgDecay;        // AUDIO_PARAMETER_ALCNG_DECAY_TIME

} AUDIO_DEFAUUT_SETTING;

//
//  Extern Parameters from Audio.c
//
extern AUDIO_STATE          AudioState;

extern AUDIO_MSG_LVL        audioMsgLevel;
extern ULONG                uiDbgBufAddr;
extern UINT32               uiDbgBufSize;
extern AUDIO_CODEC_FUNC     AudioCodecFunc[AUDIO_MAX_CODEC_NUM];
extern AUDIO_CODECSEL       AudioSelectedCodec;
extern BOOL                 bPlay2AllCodec;
extern BOOL                 bConnHDMI;
extern AUDIO_CB             pAudEventHandleTx1, pAudEventHandleTx2, pAudEventHandleRx, pAudEventHandleTxlb;

extern BOOL                 aud_is_opened(void);
extern void                 audcodec_get_fp(PAUDIO_CODEC_FUNC pAudCodecFunc);
extern void                 audcodecHDMI_getFP(PAUDIO_CODEC_FUNC pAudCodecFunc);
extern ER                   aud_set_codec_samplerate(PAUDIO_CODEC_FUNC pAudCodec, AUDIO_SR SamplingRate);
extern ER                   aud_set_codec_channel(AUDTSCH tsCH, AUDIO_CH Channel);

//
//  Extern Parameters for audio_tstx1.c
//
extern AUDIO_SR             AudSrcSRTx1;
extern AUDIO_CH             AudioTx1CH;
extern UINT32               uibuffer_insert_number_tx1;

extern ER                   aud_open_tx1(AUD_DAI_ID dai_id);
extern ER                   aud_close_tx1(AUD_DAI_ID dai_id);
extern BOOL                 aud_is_opened_tx1(AUD_DAI_ID dai_id);
extern BOOL                 aud_is_busy_tx1(AUD_DAI_ID dai_id);
extern ER                   aud_playback_tx1(AUD_DAI_ID dai_id);
extern ER                   aud_record_preset_tx1(AUD_DAI_ID dai_id);
extern ER                   aud_record_tx1(AUD_DAI_ID dai_id);
extern ER                   aud_stop_tx1(AUD_DAI_ID dai_id);
extern ER                   aud_pause_tx1(AUD_DAI_ID dai_id);
extern ER                   aud_resume_tx1(AUD_DAI_ID dai_id);
extern void                 aud_set_channel_tx1(AUD_DAI_ID dai_id,AUDIO_CH Channel);
extern void 				aud_set_tdm_channel_tx1(AUD_DAI_ID dai_id,AUDIO_TDMCH TDM);
extern ER                   aud_set_samplerate_tx1(AUD_DAI_ID dai_id,AUDIO_SR SamplingRate);
extern BOOL                 aud_set_resampleinfo_tx1(PAUDIO_RESAMPLE_INFO pResampleInfo);
extern void                 aud_set_feature_tx1(AUD_DAI_ID dai_id,AUDTS_FEATURE Feature, BOOL bEnable);
extern void                 aud_set_config_tx1(AUD_DAI_ID dai_id,AUDTS_CFG_ID CfgSel, ULONG uiSetting);
extern UINT32               aud_get_config_tx1(AUD_DAI_ID dai_id,AUDTS_CFG_ID Parameter);
extern void                 aud_reset_buf_queue_tx1(AUD_DAI_ID dai_id,AUDIO_QUEUE_SEL QueueSel);
extern BOOL                 aud_add_buf_to_queue_tx1(AUD_DAI_ID dai_id,AUDIO_QUEUE_SEL QueueSel, PAUDIO_BUF_QUEUE pAudioBufQueue);
extern BOOL                 aud_is_buf_queue_full_tx1(AUD_DAI_ID dai_id,AUDIO_QUEUE_SEL QueueSel);
extern PAUDIO_BUF_QUEUE     aud_get_done_buf_from_queue_tx1(AUD_DAI_ID dai_id,AUDIO_QUEUE_SEL QueueSel);
extern PAUDIO_BUF_QUEUE     aud_get_cur_buf_from_queue_tx1(AUD_DAI_ID dai_id,AUDIO_QUEUE_SEL QueueSel);
extern void                 aud_print_tstx1(void);

#if 0
// DBG Mode Usage
extern ER                   aud_playbackTxDual(void);
#endif

//
//  Extern Parameters for audio_tstx2.c
//
extern AUDIO_SR             AudSrcSRTx2;
extern AUDIO_CH             AudioTx2CH;
extern UINT32               uibuffer_insert_number_tx2;

extern ER                   aud_open_tx2(AUD_DAI_ID dai_id);
extern ER                   aud_close_tx2(AUD_DAI_ID dai_id);
extern BOOL                 aud_is_opened_tx2(AUD_DAI_ID dai_id);
extern BOOL                 aud_is_busy_tx2(AUD_DAI_ID dai_id);
extern ER                   aud_playback_tx2(AUD_DAI_ID dai_id);
extern ER                   aud_record_preset_tx2(AUD_DAI_ID dai_id);
extern ER                   aud_record_tx2(AUD_DAI_ID dai_id);
extern ER                   aud_stop_tx2(AUD_DAI_ID dai_id);
extern ER                   aud_pause_tx2(AUD_DAI_ID dai_id);
extern ER                   aud_resume_tx2(AUD_DAI_ID dai_id);
extern void                 aud_set_channel_tx2(AUD_DAI_ID dai_id,AUDIO_CH Channel);
extern void 				aud_set_tdm_channel_tx2(AUD_DAI_ID dai_id,AUDIO_TDMCH TDM);
extern ER                   aud_set_samplerate_tx2(AUD_DAI_ID dai_id,AUDIO_SR SamplingRate);
extern BOOL                 aud_set_resampleinfo_tx2(PAUDIO_RESAMPLE_INFO pResampleInfo);
extern void                 aud_set_feature_tx2(AUD_DAI_ID dai_id,AUDTS_FEATURE Feature, BOOL bEnable);
extern void                 aud_set_config_tx2(AUD_DAI_ID dai_id,AUDTS_CFG_ID CfgSel, ULONG uiSetting);
extern UINT32               aud_get_config_tx2(AUD_DAI_ID dai_id,AUDTS_CFG_ID Parameter);
extern void                 aud_reset_buf_queue_tx2(AUD_DAI_ID dai_id,AUDIO_QUEUE_SEL QueueSel);
extern BOOL                 aud_add_buf_to_queue_tx2(AUD_DAI_ID dai_id,AUDIO_QUEUE_SEL QueueSel, PAUDIO_BUF_QUEUE pAudioBufQueue);
extern BOOL                 aud_is_buf_queue_full_tx2(AUD_DAI_ID dai_id,AUDIO_QUEUE_SEL QueueSel);
extern PAUDIO_BUF_QUEUE     aud_get_done_buf_from_queue_tx2(AUD_DAI_ID dai_id,AUDIO_QUEUE_SEL QueueSel);
extern PAUDIO_BUF_QUEUE     aud_get_cur_buf_from_queue_tx2(AUD_DAI_ID dai_id,AUDIO_QUEUE_SEL QueueSel);
extern void                 aud_print_tstx2(void);

//
//  Extern Parameters for audio_tsrx.c
//
extern AUDIO_SR             AudSrcSRRx;
extern AUDIO_CH             AudioRxCH;
extern UINT32               uibuffer_insert_number_rx;

extern ER                   aud_open_rx(AUD_DAI_ID dai_id);
extern ER                   aud_close_rx(AUD_DAI_ID dai_id);
extern BOOL                 aud_is_opened_rx(AUD_DAI_ID dai_id);
extern BOOL                 aud_is_busy_rx(AUD_DAI_ID dai_id);
extern ER                   aud_playback_rx(AUD_DAI_ID dai_id);
extern ER                   aud_record_preset_rx(AUD_DAI_ID dai_id);
extern ER                   aud_record_rx(AUD_DAI_ID dai_id);
extern ER                   aud_stop_rx(AUD_DAI_ID dai_id);
extern ER                   aud_pause_rx(AUD_DAI_ID dai_id);
extern ER                   aud_resume_rx(AUD_DAI_ID dai_id);
extern void                 aud_set_channel_rx(AUD_DAI_ID dai_id, AUDIO_CH Channel);
extern void 				aud_set_tdm_channel_rx(AUD_DAI_ID dai_id, AUDIO_TDMCH TDM);
extern ER                   aud_set_samplerate_rx(AUD_DAI_ID dai_id, AUDIO_SR SamplingRate);
extern BOOL                 aud_set_resampleinfo_rx(PAUDIO_RESAMPLE_INFO pResampleInfo);
extern void                 aud_set_feature_rx(AUD_DAI_ID dai_id, AUDTS_FEATURE Feature, BOOL bEnable);
extern void                 aud_set_config_rx(AUD_DAI_ID dai_id, AUDTS_CFG_ID CfgSel, ULONG uiSetting);
extern UINT32               aud_get_config_rx(AUD_DAI_ID dai_id, AUDTS_CFG_ID CfgSel);
extern void                 aud_reset_buf_queue_rx(AUD_DAI_ID dai_id, AUDIO_QUEUE_SEL QueueSel);
extern BOOL                 aud_add_buf_to_queue_rx(AUD_DAI_ID dai_id, AUDIO_QUEUE_SEL QueueSel, PAUDIO_BUF_QUEUE pAudioBufQueue);
extern BOOL                 aud_is_buf_queue_full_rx(AUD_DAI_ID dai_id, AUDIO_QUEUE_SEL QueueSel);
extern PAUDIO_BUF_QUEUE     aud_get_done_buf_from_queue_rx(AUD_DAI_ID dai_id, AUDIO_QUEUE_SEL QueueSel);
extern PAUDIO_BUF_QUEUE     aud_get_cur_buf_from_queue_rx(AUD_DAI_ID dai_id, AUDIO_QUEUE_SEL QueueSel);
extern void                 aud_print_tsrx(void);

//
//  Extern Parameters for audio_tstxlb.c
//
extern AUDIO_CH             AudioTxlbCH;
extern UINT32               uibuffer_insert_number_txlb;

extern ER                   aud_open_txlb(AUD_DAI_ID dai_id);
extern ER                   aud_close_txlb(AUD_DAI_ID dai_id);
extern BOOL                 aud_is_opened_txlb(AUD_DAI_ID dai_id);
extern BOOL                 aud_is_busy_txlb(AUD_DAI_ID dai_id);
extern ER                   aud_playback_txlb(AUD_DAI_ID dai_id);
extern ER                   aud_record_preset_txlb(AUD_DAI_ID dai_id);
extern ER                   aud_record_txlb(AUD_DAI_ID dai_id);
extern ER                   aud_stop_txlb(AUD_DAI_ID dai_id);
extern ER                   aud_pause_txlb(AUD_DAI_ID dai_id);
extern ER                   aud_resume_txlb(AUD_DAI_ID dai_id);
extern void                 aud_set_channel_txlb(AUD_DAI_ID dai_id, AUDIO_CH Channel);
extern void 				aud_set_tdm_channel_txlb(AUD_DAI_ID dai_id, AUDIO_TDMCH TDM);
extern ER                   aud_set_samplerate_txlb(AUD_DAI_ID dai_id, AUDIO_SR SamplingRate);
extern BOOL                 aud_set_resampleinfo_txlb(PAUDIO_RESAMPLE_INFO pResampleInfo);
extern void                 aud_set_feature_txlb(AUD_DAI_ID dai_id, AUDTS_FEATURE Feature, BOOL bEnable);
extern void                 aud_set_config_txlb(AUD_DAI_ID dai_id, AUDTS_CFG_ID CfgSel, ULONG uiSetting);
extern UINT32               aud_get_config_txlb(AUD_DAI_ID dai_id, AUDTS_CFG_ID CfgSel);
extern void                 aud_reset_buf_queue_txlb(AUD_DAI_ID dai_id, AUDIO_QUEUE_SEL QueueSel);
extern BOOL                 aud_add_buf_to_queue_txlb(AUD_DAI_ID dai_id, AUDIO_QUEUE_SEL QueueSel, PAUDIO_BUF_QUEUE pAudioBufQueue);
extern BOOL                 aud_is_buf_queue_full_txlb(AUD_DAI_ID dai_id, AUDIO_QUEUE_SEL QueueSel);
extern PAUDIO_BUF_QUEUE     aud_get_done_buf_from_queue_txlb(AUD_DAI_ID dai_id, AUDIO_QUEUE_SEL QueueSel);
extern PAUDIO_BUF_QUEUE     aud_get_cur_buf_from_queue_txlb(AUD_DAI_ID dai_id, AUDIO_QUEUE_SEL QueueSel);


//
//  Extern Parameters for audio_queue.c
//

extern AUDIO_BUF_QUEUE  	AudioRealBufferQueue_aud[AUDTSCH_TOT][AUDIO_BUF_BLKNUM];
extern PAUDIO_BUF_QUEUE     pAudioRealBufferFront_aud[AUDTSCH_TOT];
extern PAUDIO_BUF_QUEUE     pAudioRealBufferRear_aud[AUDTSCH_TOT];
extern PAUDIO_BUF_QUEUE     pAudioRealBufferDone_aud[AUDTSCH_TOT];

extern AUDIO_BUF_QUEUE  	AudioPseudoBufferQueue_aud[AUDTSCH_TOT][AUDIO_BUF_BLKNUM];
extern PAUDIO_BUF_QUEUE     pAudioPseudoBufferFront_aud[AUDTSCH_TOT];
extern PAUDIO_BUF_QUEUE     pAudioPseudoBufferRear_aud[AUDTSCH_TOT];
extern PAUDIO_BUF_QUEUE     pAudioPseudoBufferDone_aud[AUDTSCH_TOT];


extern void                 aud_init_queue(void);
extern ER                   aud_reset_queue(AUDTSCH tsCH);
extern ER                   aud_is_queue_full(AUDTSCH tsCH);
extern ER                   aud_add_to_queue(AUDTSCH tsCH, PAUDIO_BUF_QUEUE pAudioBufQueue);
extern PAUDIO_BUF_QUEUE     aud_get_done_queue(AUDTSCH tsCH);
extern PAUDIO_BUF_QUEUE     aud_get_cur_queue(AUDTSCH tsCH);



// Hidden API from DAI
extern DAI_INTERRUPT        dai3_wait_interrupt(int id , DAI_INTERRUPT WaitedFlag);
extern void                 dai3_select_pinmux(int id ,BOOL bEnable);
extern void                 dai3_select_mclk_pinmux(int id ,BOOL bEnable);


extern ER aud_set_codec_samplerate(PAUDIO_CODEC_FUNC pAudCodec, AUDIO_SR SamplingRate);
extern ER aud_set_codec_channel(AUDTSCH tsCH, AUDIO_CH Channel);
extern BOOL aud_is_opened(void);

// slave clock lost
extern int                  aud_clock_lost_status_rx[AUD_DAI_ID_TOT];
extern int                  aud_clock_lost_status_tx[AUD_DAI_ID_TOT];
extern int                  aud_clock_lost_status_tx2[AUD_DAI_ID_TOT];
extern int                  aud_clock_lost_status_txlb[AUD_DAI_ID_TOT];


extern vk_spinlock_t aud_my_lock_rx;
extern vk_spinlock_t aud_my_lock_tx1;
extern vk_spinlock_t aud_my_lock_tx2;
extern vk_spinlock_t aud_my_lock_txlb;
extern vk_spinlock_t mylock_aud_qu;

extern UINT32        uibuffer_insert_number_tx1;
extern UINT32        uibuffer_insert_number_tx2;
extern UINT32        uibuffer_insert_number_rx;
extern UINT32        uibuffer_insert_number_txlb;

// Hidden API from CKGEN
//extern ER                   pll_setDrvPLL(PLL_ID id, UINT32 uiSetting);

#endif
//@}

