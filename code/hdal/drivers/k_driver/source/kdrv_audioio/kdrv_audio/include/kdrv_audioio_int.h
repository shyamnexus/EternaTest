/*
    DAI internal header file

    This file is the header file that define register for DAI module


    @file       dai_int.h
    @ingroup    mISYSAud
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2016.  All rights reserved.
*/


#ifndef _KDRV_AUDIO_INT_H
#define _KDRV_AUDIO_INT_H

#ifdef __KERNEL__
#include <linux/delay.h>
#include <linux/list.h>
#include "kdrv_audioio/kdrv_audioio.h"
//#include "../audio_common/include/audio_int.h"
#elif defined(__FREERTOS)
#include "kdrv_audioio/kdrv_audioio.h"
#include "kdrv_audioio_list.h"
#endif
#include "audio_int.h"
#include "AudioCodec.h"

//#include "audio_dbg.h"
extern vk_spinlock_t mylock_aud_qu; // Audio

#define KDRV_LIST_NODE_NUM 5


#define KDRV_ERRFLAG_TX  (AUDIO_EVENT_BUF_FULL|AUDIO_EVENT_BUF_EMPTY|AUDIO_EVENT_FIFO_ERROR|AUDIO_EVENT_BUF_RTEMPTY|AUDIO_EVENT_BUF_FULL2|AUDIO_EVENT_DONEBUF_FULL2|AUDIO_EVENT_FIFO_ERROR2)
#define KDRV_ERRFLAG_RX  (AUDIO_EVENT_BUF_FULL|AUDIO_EVENT_BUF_EMPTY|AUDIO_EVENT_DONEBUF_FULL|AUDIO_EVENT_FIFO_ERROR|AUDIO_EVENT_BUF_RTEMPTY|AUDIO_EVENT_BUF_FULL2|AUDIO_EVENT_DONEBUF_FULL2|AUDIO_EVENT_FIFO_ERROR2)

#define KDRV_FLG_ID_AUD_STS_RX_ISR_IDLE     0x10000000
#define KDRV_FLG_ID_AUD_STS_TX1_ISR_IDLE    0x20000000
#define KDRV_FLG_ID_AUD_STS_TX2_ISR_IDLE    0x40000000
#define KDRV_FLG_ID_AUD_STS_TXLB_ISR_IDLE   0x80000000

typedef struct list_head LIST_HEAD;


typedef struct _KDRV_AUDIO_INFO {
	PAUDTS_OBJ  paud_drv_obj;
	UINT32      sample_rate;
	UINT32      channel;
	DAI_I2SCHLEN bit_per_sample;
	UINT32      bclk_rate;
	UINT32      volume;
	AUDIO_TDMCH tdm_channel;
	DAI_OPMODE  bmode;
	BOOL        opened_Tx;
	BOOL        opened_Rx;
    BOOL        opened_Txlb;
    DAI_I2S_DATAORDER i2s_data_order;   
    BOOL        opened;
	UINT32      audio_source; // 0: i2s , 1: EAC , 2:HDMI
}KDRV_AUDIO_INFO;



typedef struct _KDRV_AUDIO_LIST_NODE{
    BOOL                b_used;
    KDRV_BUFFER_INFO    buffer_addr;
	KDRV_CALLBACK_FUNC  cb_func;
    ULONG               user_data;
	LIST_HEAD           list;
}KDRV_AUDIO_LIST_NODE;

typedef enum
{
    KDRV_HANDLE_RX1,
    KDRV_HANDLE_TXLB,
    KDRV_HANDLE_TX1,
    KDRV_HANDLE_TX2,
    KDRV_HANDLE_MAX,
    ENUM_DUMMY4WORD(KDRV_HANDLE)

}KDRV_HANDLE;

/*
    Audio Internal Flag
*/
typedef enum {
	KDRV_AUDIO_INT_FLAG_NONE             = 0x00000000,
	KDRV_AUDIO_INT_FLAG_LINEPWR_ALWAYSON = 0x00000001,
	KDRV_AUDIO_INT_FLAG_SPKPWR_ALWAYSON  = 0x00000002,
	KDRV_AUDIO_INT_FLAG_ADVCMPWR_ALWAYSON= 0x00000004,
	KDRV_AUDIO_INT_FLAG_CLKALWAYS_ON     = 0x00000100,

    KDRV_AUDIO_INT_FLAG_RX_EXPAND        = 0x00001000,
    KDRV_AUDIO_INT_FLAG_TX_EXPAND        = 0x00002000,

    KDRV_AUDIO_INT_FLAG_RX_TIMECODE_HIT  = 0x00010000,
    KDRV_AUDIO_INT_FLAG_TX_TIMECODE_HIT  = 0x00020000,

	ENUM_DUMMY4WORD(KDRV_AUDIO_INT_FLAG)
} KDRV_AUDIO_INT_FLAG;

// slave clock lost detect
extern int                  aud_clock_lost_status_rx[AUD_DAI_ID_TOT];
extern int                  aud_clock_lost_status_tx[AUD_DAI_ID_TOT];
extern int                  aud_clock_lost_status_tx2[AUD_DAI_ID_TOT];
extern int                  aud_clock_lost_status_txlb[AUD_DAI_ID_TOT];

#ifdef __KERNEL__
#define audio_msg printk
#elif defined(__FREERTOS)
#define audio_msg vk_printk
#endif

#endif

