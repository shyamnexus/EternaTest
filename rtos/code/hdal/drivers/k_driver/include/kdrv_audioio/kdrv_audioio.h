/**
    KDRV Header file of Audio Input(Capture)/Output.

    Exported KDRV header file of Audio Input(Capture)/Output.

    @file       kdrv_audioio.h
    @ingroup
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#ifndef _KDRV_AUDIOIO_H
#define _KDRV_AUDIOIO_H

#if 1//__KERNEL__
#include "kdrv_type.h"
#else
#endif

#include "kdrv_audioio/audlib_src.h"
#include "kdrv_audioio/audlib_aec.h"

#define VERSION     "1.00.16"

#define AUDIO_DSP_CHAN      32
#define DEFAULT_SAMPLE_SZ   16
#define DSP_INT_MAX_VAL     0x7FFFFFFF

#define PLATFORM_MAX_SSP_CNT 1
#define PLATFORM_MAX_CHIP_CNT 1
#define PLATFORM_TOATL_SSP_CNT PLATFORM_MAX_SSP_CNT * PLATFORM_MAX_CHIP_CNT
#define KDRV_HANDLE_OFS 20

extern int audio_i2s_num[PLATFORM_TOATL_SSP_CNT];
extern int audio_enable[PLATFORM_TOATL_SSP_CNT];
extern int audio_tdm_chan[PLATFORM_TOATL_SSP_CNT];
extern int audio_sample_size[PLATFORM_TOATL_SSP_CNT];
extern int audio_sample_rate[PLATFORM_TOATL_SSP_CNT];
extern int audio_bit_clock[PLATFORM_TOATL_SSP_CNT];
extern int audio_i2s_master[PLATFORM_TOATL_SSP_CNT];
extern int audio_is_stereo[PLATFORM_TOATL_SSP_CNT];
extern int audio_i2s_dataorder_type[PLATFORM_TOATL_SSP_CNT];
extern int audio_source[PLATFORM_TOATL_SSP_CNT];
extern int audio_i2s_mclk[PLATFORM_TOATL_SSP_CNT];

typedef struct {
  int ch_cnt;  		// total ch cnt
  int offset[20];   // ch <-> ofs mapping, 
  					// Ex: ch_cnt = 4			      ______
  					//     offset[0] = 0      |______|      |
  					//     offset[1] = 2      ch0 ch2 ch1 ch3
  					//     offset[2] = 1
  					//     offset[3] = 3
  					//
  					// Ex: ch_cnt = 16			                                 __________________________________
  					//     offset[0]  = 0     |_________________________________|                                  |
  					//     offset[1]  = 2     ch0 ch2 ch4 ch6 ch8 ch10 ch12 ch14 ch1 ch3 ch5 ch7 ch9 ch11 ch13 ch15
  					//     offset[2]  = 4
  					//     offset[3]  = 6 
  					//     offset[4]  = 8
  					//     offset[5]  = 10
  					//     offset[6]  = 12
  					//     offset[7]  = 14
  					//     offset[8]  = 1
  					//     offset[9]  = 3  		
  					//     offset[10] = 5
  					//     offset[11] = 7
  					//     offset[12] = 9
  					//     offset[13] = 11 
  					//     offset[14] = 13
  					//     offset[15] = 15   					
}KDRV_AUDIO_CH_OFS;


typedef enum {
	KDRV_AUDIOLIB_ID_SRC,
	KDRV_AUDIOLIB_ID_AEC,
	KDRV_AUDIOLIB_ID_MAX,
	ENUM_DUMMY4WORD(KDRV_AUDIOLIB_ID)
} KDRV_AUDIOLIB_ID;

typedef struct {
	int      (*get_version)(void);
	int      (*pre_init)(int ch, int in_count, int out_count, int one_frame_mode);
	int      (*init)(int *handle, int ch, int in_count, int out_count, int one_frame_mode, short *out_mem);
	int      (*run)(int handle, void *p_buffer_in, void *p_buffer_out);
	void	 (*destroy)(int handle);
} KDRV_AUDIO_SRC_FUNC;

typedef struct {
	ER       (*open)(void);
	BOOL     (*is_opened)(void);
	ER       (*close)(void);
	void     (*set_config)(AEC_CONFIG_ID aec_sel, ULONG aec_cfg_value);
	BOOL     (*init)(void);
	BOOL     (*run)(PAEC_BITSTREAM p_aec_io);
	ULONG   (*get_config)(AEC_CONFIG_ID aec_sel);
	INT32	 (*get_required_buffer_size)(AEC_BUFINFO_ID buffer_id);
} KDRV_AUDIO_AEC_FUNC;


typedef enum
{
	/*chip 0*/
    KDRV_AUD_HANDLE_DAI0_RX,
	KDRV_AUD_HANDLE_DAI0_TX,
	KDRV_AUD_HANDLE_DAI0_TXLB,
	KDRV_AUD_HANDLE_DAI0_TX2,
    KDRV_AUD_HANDLE_DAI1_RX,
    KDRV_AUD_HANDLE_DAI1_TX,
	KDRV_AUD_HANDLE_DAI1_TXLB,
	KDRV_AUD_HANDLE_DAI1_TX2,
    KDRV_AUD_HANDLE_DAI2_RX,
    KDRV_AUD_HANDLE_DAI2_TX,
	KDRV_AUD_HANDLE_DAI2_TXLB,
	KDRV_AUD_HANDLE_DAI2_TX2,
    KDRV_AUD_HANDLE_DAI3_RX,
    KDRV_AUD_HANDLE_DAI3_TX,
	KDRV_AUD_HANDLE_DAI3_TXLB,
	KDRV_AUD_HANDLE_DAI3_TX2,
	KDRV_AUD_HANDLE_DAIHDMI_TX = 17,

	/*chip 1*/
    KDRV_AUD_HANDLE_DAI0_RX_CHIP1 =20,
	KDRV_AUD_HANDLE_DAI0_TX_CHIP1,
	KDRV_AUD_HANDLE_DAI0_TXLB_CHIP1,
	KDRV_AUD_HANDLE_DAI0_TX2_CHIP1,
    KDRV_AUD_HANDLE_DAI1_RX_CHIP1,
    KDRV_AUD_HANDLE_DAI1_TX_CHIP1,
	KDRV_AUD_HANDLE_DAI1_TXLB_CHIP1,
	KDRV_AUD_HANDLE_DAI1_TX2_CHIP1,
    KDRV_AUD_HANDLE_DAI2_RX_CHIP1,
    KDRV_AUD_HANDLE_DAI2_TX_CHIP1,
	KDRV_AUD_HANDLE_DAI2_TXLB_CHIP1,
	KDRV_AUD_HANDLE_DAI2_TX2_CHIP1,
	KDRV_AUD_HANDLE_DAI3_RX_CHIP1,
    KDRV_AUD_HANDLE_DAI3_TX_CHIP1,
	KDRV_AUD_HANDLE_DAI3_TXLB_CHIP1,
	KDRV_AUD_HANDLE_DAI3_TX2_CHIP1,
	KDRV_AUD_HANDLE_DAIHDMI_TX_CHIP1 = 37,

	/*chip 2*/
	KDRV_AUD_HANDLE_DAI0_RX_CHIP2 =40,
	KDRV_AUD_HANDLE_DAI0_TX_CHIP2,
	KDRV_AUD_HANDLE_DAI0_TXLB_CHIP2,
	KDRV_AUD_HANDLE_DAI0_TX2_CHIP2,
    KDRV_AUD_HANDLE_DAI1_RX_CHIP2,
    KDRV_AUD_HANDLE_DAI1_TX_CHIP2,
	KDRV_AUD_HANDLE_DAI1_TXLB_CHIP2,
	KDRV_AUD_HANDLE_DAI1_TX2_CHIP2,
    KDRV_AUD_HANDLE_DAI2_RX_CHIP2,
    KDRV_AUD_HANDLE_DAI2_TX_CHIP2,
	KDRV_AUD_HANDLE_DAI2_TXLB_CHIP2,
	KDRV_AUD_HANDLE_DAI2_TX2_CHIP2,
    KDRV_AUD_HANDLE_DAI3_RX_CHIP2,
    KDRV_AUD_HANDLE_DAI3_TX_CHIP2,
    KDRV_AUD_HANDLE_DAI3_TXLB_CHIP2,
	KDRV_AUD_HANDLE_DAI3_TX2_CHIP2,
	KDRV_AUD_HANDLE_DAIHDMI_TX_CHIP2 = 57,

	/*chip 3*/
    KDRV_AUD_HANDLE_DAI0_RX_CHIP3 =60,
	KDRV_AUD_HANDLE_DAI0_TX_CHIP3,
	KDRV_AUD_HANDLE_DAI0_TXLB_CHIP3,
	KDRV_AUD_HANDLE_DAI0_TX2_CHIP3,
    KDRV_AUD_HANDLE_DAI1_RX_CHIP3,
    KDRV_AUD_HANDLE_DAI1_TX_CHIP3,
    KDRV_AUD_HANDLE_DAI1_TXLB_CHIP3,
	KDRV_AUD_HANDLE_DAI1_TX2_CHIP3,
    KDRV_AUD_HANDLE_DAI2_RX_CHIP3,
    KDRV_AUD_HANDLE_DAI2_TX_CHIP3,
    KDRV_AUD_HANDLE_DAI2_TXLB_CHIP3,
	KDRV_AUD_HANDLE_DAI2_TX2_CHIP3,
    KDRV_AUD_HANDLE_DAI3_RX_CHIP3,
    KDRV_AUD_HANDLE_DAI3_TX_CHIP3,
    KDRV_AUD_HANDLE_DAI3_TXLB_CHIP3,
	KDRV_AUD_HANDLE_DAI3_TX2_CHIP3,
	KDRV_AUD_HANDLE_DAIHDMI_TX_CHIP3 = 77,

	KDRV_AUD_HANDLE_MAX,
    ENUM_DUMMY4WORD(KDRV_AUD_HANDLE)

}KDRV_AUD_HANDLE;

typedef enum
{
    KDRV_RETURN_STATUS_SUCESS,
	KDRV_RETURN_STATUS_FAIL,
	KDRV_RETURN_STATUS_BUFFULL,
	KDRV_RETURN_STATUS_BUFEMPTY,
	KDRV_RETURN_STATUS_CLOCK_LOST,
	KDRV_RETURN_STATUS_MAX,
    ENUM_DUMMY4WORD(KDRV_RETURN_STATUS)

}KDRV_RETURN_STATUS;

typedef enum {
    KDRV_AUDIO_TRIGGER_TYPE_RX,
    KDRV_AUDIO_TRIGGER_TYPE_TX,
    KDRV_AUDIO_TRIGGER_TYPE_TXLB,
    KDRV_AUDIO_TRIGGER_TYPE_MAX,
} KDRV_AUDIO_TRIGGER_TYPE;

typedef struct {
	KDRV_BUFFER_INFO *ctrl;
	ULONG user_data;
	KDRV_CALLBACK_FUNC	*cb_func;
}KDRV_AUDIO_TRIGGER_INFO;

typedef struct {
	KDRV_RETURN_STATUS ret;
	unsigned long time_stamp;
}KDRV_AUDIO_CALLBACK_INFO;

typedef struct {
	union {
		KDRV_AUDIO_SRC_FUNC src;
		KDRV_AUDIO_AEC_FUNC aec;
	};
} KDRV_AUDIOLIB_FUNC;


/**
    Audio channel data type
    Used in kdrv_audioio_set(KDRV_AUDIOIO_CAP_CHANNEL_DATATYPE, KDRV_AUDIO_CHANNEL_DATA_*).
    		kdrv_audioio_set(KDRV_AUDIOIO_OUT_CHANNEL_DATATYPE, KDRV_AUDIO_CHANNEL_DATA_*)
*/
typedef enum {
	KDRV_AUDIO_CHANNEL_DATA_MONO,		///< mono data
	KDRV_AUDIO_CHANNEL_DATA_STEREO,		///< stereo data

	ENUM_DUMMY4WORD(KDRV_AUDIO_CHANNEL_DATA)
} KDRV_AUDIO_CHANNEL_DATA;



typedef enum {
	KDRV_AUDIO_CONFIG_TYPE_NORMAL,
	KDRV_AUDIO_CONFIG_TYPE_ARRAY,
	KDRV_AUDIO_CONFIG_TYPE_MAX,
}KDRV_AUDIO_CONFIG_TYPE;


typedef struct _kdrv_aud_buf {
    unsigned dai_idx;
    unsigned bitmap;    /* vch_bitmap  */
    unsigned length[AUDIO_DSP_CHAN];
    unsigned offset[AUDIO_DSP_CHAN];
    unsigned buf_pos[AUDIO_DSP_CHAN];
    unsigned buf_size;
    int channel_type;   /* 1: mono, 2: stereo*/
    dma_addr_t buf_ptr;
    uintptr_t kbuf_ptr; //++ TODO 636
    unsigned long time_stamp;
} kdrv_aud_buf;

typedef enum {
    AU_PARM_BUF_NUM        = 0,
    AU_PARM_BUF_SZ,
    AU_PARM_DMA_ROUTE_SEL,
    AU_PARM_SSP_USED,
    AU_PARM_MAXREC_CH,
    AU_PARM_REC_OWNER,
    AU_PARM_MAXPB_CH,
    AU_PARM_PB_OWNER,
    AU_PARM_PLAY_MODE,
    AU_PARM_REC_CH,
    AU_PARM_PLAY_SSP,
    AU_PARM_PLAYOUT_CH,
    AU_PARM_ADC_CH,
    AU_PARM_DAC_CH,
    AU_PARM_REC_OVERCNT,
    AU_PARM_PB_UNDERCNT,
    AU_PARM_DRV_DBG,
    AU_PARM_SSP_DUMP,
    AU_PARM_ID_MAX            = DSP_INT_MAX_VAL,
} au_param_t;

typedef enum {
    AU_PARM_SSP_IDX        = 0,
    AU_PARM_REC_EN,
    AU_PARM_PB_EN,
    AU_PARM_SSP_CHAN,
    AU_PARM_SAMPLE_SZ,
    AU_PARM_SAMPLE_RATE,
    AU_PARM_SSP_CLK,
    AU_PARM_BIT_CLK,
    AU_PARM_IS_MASTER,
    AU_PARM_NR_EN,
    AU_PARM_IS_STEREO,
    AU_PARM_RESAMPLE_RATIO,
    AU_PARM_LV_SUND_CH,
    AU_PARM_PB_CHMAP,
    AU_PARM_PB_MODE,
    AU_PARM_GM_LOG_BASE,
    AU_PARM_ARRAY_MAX      = DSP_INT_MAX_VAL,
} au_param_array_t;

typedef enum {
    AU_DBG_PB_INDEX        = 0,
    AU_DBG_REC_INDEX,
    AU_DBG_ARRAY_MAX      = DSP_INT_MAX_VAL,
} au_DBG_t;

typedef struct {
	au_param_t  			param;
	int						value;
	au_param_array_t  param_array;
	int				 	 *p_array;
	int						  len;
}KDRV_AUDIO_CONFIG_INFO;

// FOR DAI2  //

INT32 kdrv_audioio_reg_audiolib(KDRV_AUDIOLIB_ID id, KDRV_AUDIOLIB_FUNC *p_func);
KDRV_AUDIOLIB_FUNC* kdrv_audioio_get_audiolib(KDRV_AUDIOLIB_ID id);



/*************************
*	Capture related items:
**************************/

/**
    Audio capture path

    Select the audio capture path.
    Used in kdrv_audioio_set(KDRV_AUDIOIO_CAP_PATH, KDRV_AUDIO_CAP_PATH_*) / kdrv_audioio_get(KDRV_AUDIOIO_CAP_PATH)
*/
typedef enum {
	KDRV_AUDIO_CAP_PATH_AMIC,			///< Analog Microphone (Default)
	KDRV_AUDIO_CAP_PATH_DMIC,			///< Digital Microphone
	KDRV_AUDIO_CAP_PATH_I2S,			///< Digital Microphone

	ENUM_DUMMY4WORD(KDRV_AUDIO_CAP_PATH)
} KDRV_AUDIO_CAP_PATH;


/**
    Audio Capture Default Settings Select

    This is used at kdrv_audioio_set(KDRV_AUDIOIO_CAP_DEFAULT_CFG, KDRV_AUDIO_CAP_DEFSET_*)
    to select audio default configurations. Different characteristics for each select option is shown as below.
*/
typedef enum {
	KDRV_AUDIO_CAP_DEFSET_10DB,			///< Boost Gain 10dB. NoiseGate-Enable. ALC-Range(+1.5 ~ +25.5dB).
										///< Atk=0.08ms. Dcy=0.16s. NG-Atk=0.72s. NG-Dcy=0.36s. NG-Target=2.
										///< NoiseGate-TH= -67.5-10=-77.5dB from MIC input.
	KDRV_AUDIO_CAP_DEFSET_10DB_HP_8K,	///< The Same as KDRV_AUDIO_DEFSET_10DB but with HighPass-Filter Enabled.
										///< HighPass Filter 3dB is about 200Hz for SR8KHz.
	KDRV_AUDIO_CAP_DEFSET_10DB_HP_16K,  ///< The Same as KDRV_AUDIO_DEFSET_10DB but with HighPass-Filter Enabled.
										///< HighPass Filter 3dB is about 200Hz for SR16KHz.
	KDRV_AUDIO_CAP_DEFSET_10DB_HP_32K,	///< The Same as KDRV_AUDIO_DEFSET_10DB but with HighPass-Filter Enabled.
										///< HighPass Filter 3dB is about 200Hz for SR32KHz.
	KDRV_AUDIO_CAP_DEFSET_10DB_HP_48K,  ///< The Same as KDRV_AUDIO_DEFSET_10DB but with HighPass-Filter Enabled.
										///< HighPass Filter 3dB is about 200Hz for SR48KHz.

	KDRV_AUDIO_CAP_DEFSET_20DB,         ///< Boost Gain 20dB. NoiseGate-Enable. ALC-Range(-9 ~ +21dB).
										///< Atk=0.08ms. Dcy=0.16s. NG-Atk=0.72s. NG-Dcy=0.36s. NG-Target=1.
										///< NoiseGate-TH= -58.5-20=-78.5dB from MIC input.
	KDRV_AUDIO_CAP_DEFSET_20DB_HP_8K,   ///< The Same as KDRV_AUDIO_DEFSET_20DB but with HighPass-Filter Enabled.
										///< HighPass Filter 3dB is about 200Hz for SR8KHz.
	KDRV_AUDIO_CAP_DEFSET_20DB_HP_16K,  ///< The Same as KDRV_AUDIO_DEFSET_20DB but with HighPass-Filter Enabled.
										///< HighPass Filter 3dB is about 200Hz for SR16KHz.
	KDRV_AUDIO_CAP_DEFSET_20DB_HP_32K,  ///< The Same as KDRV_AUDIO_DEFSET_20DB but with HighPass-Filter Enabled.
										///< HighPass Filter 3dB is about 200Hz for SR32KHz.
	KDRV_AUDIO_CAP_DEFSET_20DB_HP_48K,  ///< The Same as KDRV_AUDIO_DEFSET_20DB but with HighPass-Filter Enabled.
										///< HighPass Filter 3dB is about 200Hz for SR48KHz.

	KDRV_AUDIO_CAP_DEFSET_30DB,         ///< Boost Gain 30dB. NoiseGate-Enable. ALC-Range(-9 ~ +13.5dB).
										///< Atk=0.08ms. Dcy=0.16s. NG-Atk=2.88s. NG-Dcy=0.36s. NG-Target=1.
										///< NoiseGate-TH= -40-30=-70dB from MIC input.
	KDRV_AUDIO_CAP_DEFSET_30DB_HP_8K,   ///< The Same as KDRV_AUDIO_DEFSET_30DB but with HighPass-Filter Enabled.
										///< HighPass Filter 3dB is about 200Hz for SR8KHz.
	KDRV_AUDIO_CAP_DEFSET_30DB_HP_16K,  ///< The Same as KDRV_AUDIO_DEFSET_30DB but with HighPass-Filter Enabled.
										///< HighPass Filter 3dB is about 200Hz for SR16KHz.
	KDRV_AUDIO_CAP_DEFSET_30DB_HP_32K,  ///< The Same as KDRV_AUDIO_DEFSET_30DB but with HighPass-Filter Enabled.
										///< HighPass Filter 3dB is about 200Hz for SR32KHz.
	KDRV_AUDIO_CAP_DEFSET_30DB_HP_48K,  ///< The Same as KDRV_AUDIO_DEFSET_30DB but with HighPass-Filter Enabled.
										///< HighPass Filter 3dB is about 200Hz for SR48KHz.

	KDRV_AUDIO_CAP_DEFSET_0DB,          ///< Boost Gain 0dB. NoiseGate-Enable. ALC-Range(-9 ~ +13.5dB).
										///< Atk=0.08ms. Dcy=0.16s. NG-Atk=2.88s. NG-Dcy=0.36s. NG-Target=1.
										///< NoiseGate-TH= -40-0=-40dB from MIC input.
	KDRV_AUDIO_CAP_DEFSET_0DB_HP_8K,    ///< The Same as KDRV_AUDIO_DEFSET_0DB but with HighPass-Filter Enabled.
										///< HighPass Filter 3dB is about 200Hz for SR8KHz.
	KDRV_AUDIO_CAP_DEFSET_0DB_HP_16K,   ///< The Same as KDRV_AUDIO_DEFSET_0DB but with HighPass-Filter Enabled.
										///< HighPass Filter 3dB is about 200Hz for SR16KHz.
	KDRV_AUDIO_CAP_DEFSET_0DB_HP_32K,   ///< The Same as KDRV_AUDIO_DEFSET_0DB but with HighPass-Filter Enabled.
										///< HighPass Filter 3dB is about 200Hz for SR32KHz.
	KDRV_AUDIO_CAP_DEFSET_0DB_HP_48K,   ///< The Same as KDRV_AUDIO_DEFSET_0DB but with HighPass-Filter Enabled.
										///< HighPass Filter 3dB is about 200Hz for SR48KHz.

	KDRV_AUDIO_CAP_DEFSET_ALCOFF,       ///< Boost Gain 0dB. NoiseGate-Disable. ALC-Disable
	KDRV_AUDIO_CAP_DEFSET_ALCOFF_HP_8K, ///< The Same as KDRV_AUDIO_DEFSET_ALCOFF but with HighPass-Filter Enabled.
										///< HighPass Filter 3dB is about 200Hz for SR8KHz.
	KDRV_AUDIO_CAP_DEFSET_ALCOFF_HP_16K,///< The Same as KDRV_AUDIO_DEFSET_ALCOFF but with HighPass-Filter Enabled.
										///< HighPass Filter 3dB is about 200Hz for SR16KHz.
	KDRV_AUDIO_CAP_DEFSET_ALCOFF_HP_32K,///< The Same as KDRV_AUDIO_DEFSET_ALCOFF but with HighPass-Filter Enabled.
										///< HighPass Filter 3dB is about 200Hz for SR32KHz.
	KDRV_AUDIO_CAP_DEFSET_ALCOFF_HP_48K,///< The Same as KDRV_AUDIO_DEFSET_ALCOFF but with HighPass-Filter Enabled.
										///< HighPass Filter 3dB is about 200Hz for SR48KHz.
	KDRV_AUDIO_CAP_DEFSET_DMIC,         ///< Default setting for Digital mic.
										///< Atk=0.08ms. Dcy=0.16s. NG-Atk=2.88s. NG-Dcy=0.36s. NG-Target=1.
										///< NoiseGate-TH= -40-30=-70dB from MIC input.
	KDRV_AUDIO_CAP_DEFSET_DMIC_LP_8K,  	///< The Same as AUDIO_DEFSET_DMIC but with HighPass-Filter Enabled.
										///< Low Pass Filter 3dB is about 2000Hz for SR8KHz.
	KDRV_AUDIO_CAP_DEFSET_DMIC_LP_16K, 	///< The Same as AUDIO_DEFSET_DMIC but with HighPass-Filter Enabled.
										///< Low Pass Filter 3dB is about 4500Hz for SR16KHz.
	KDRV_AUDIO_CAP_DEFSET_DMIC_LP_32K, 	///< The Same as AUDIO_DEFSET_DMIC but with HighPass-Filter Enabled.
										///< Low Pass Filter 3dB is about 8000Hz for SR32KHz.
	KDRV_AUDIO_CAP_DEFSET_DMIC_LP_48K, 	///< The Same as AUDIO_DEFSET_DMIC but with HighPass-Filter Enabled.
										///< Low Pass Filter 3dB is about 12000Hz for SR48KHz.

	KDRV_AUDIO_CAP_DEFSET_MAX,
	ENUM_DUMMY4WORD(KDRV_AUDIO_CAP_DEFSET)
} KDRV_AUDIO_CAP_DEFSET;


/**
    Audio capture mono select

    When KDRV_AUDIOIO_CAP_CHANNEL_NUMBER is set to 1, this is used to select this 1 channel is from left or right on physical path.
    Used in kdrv_audioio_set(KDRV_AUDIOIO_CAP_MONO_SEL, KDRV_AUDIO_CAP_MONO_*) / kdrv_audioio_get(KDRV_AUDIOIO_CAP_MONO_SEL)
*/
typedef enum {
	KDRV_AUDIO_CAP_MONO_LEFT,			///< LEFT channel (Default)
	KDRV_AUDIO_CAP_MONO_RIGHT,			///< RIGHT channel

	ENUM_DUMMY4WORD(KDRV_AUDIO_CAP_MONO)
} KDRV_AUDIO_CAP_MONO;


/**
    Auto Level Control and Noise Gate Time Resolution basis
*/
typedef enum {
	KDRV_AUDIO_CAP_TRESO_BASIS_800US   = 0x40457,  ///< Time Basis as  0.8ms for ALC Attack/Decay Time adjustment.
	KDRV_AUDIO_CAP_TRESO_BASIS_1000US  = 0x5056C,  ///< Time Basis as  1.0ms for ALC Attack/Decay Time adjustment.
	KDRV_AUDIO_CAP_TRESO_BASIS_2000US  = 0xA0000,  ///< Time Basis as  2.0ms for ALC Attack/Decay Time adjustment.
	KDRV_AUDIO_CAP_TRESO_BASIS_5000US  = 0x120000, ///< Time Basis as  5.0ms for ALC Attack/Decay Time adjustment.
	KDRV_AUDIO_CAP_TRESO_BASIS_10000US = 0x1FFFFF, ///< Time Basis as 10.0ms for ALC Attack/Decay Time adjustment.
	KDRV_AUDIO_CAP_TRESO_BASIS_15000US = 0x2FFFFF, ///< Time Basis as 15.0ms for ALC Attack/Decay Time adjustment.
	KDRV_AUDIO_CAP_TRESO_BASIS_45000US = 0x7FFFFF, ///< Time Basis as 45.0ms for ALC Attack/Decay Time adjustment.

	ENUM_DUMMY4WORD(KDRV_AUDIO_CAP_TRESO_BASIS)
} KDRV_AUDIO_CAP_TRESO_BASIS;

/**
    AUDIO Total Record Gain Level
*/
typedef enum {
	KDRV_AUDIO_CAP_GAIN_LEVEL8  =  8, ///< total  8 recordgain level
	KDRV_AUDIO_CAP_GAIN_LEVEL16 = 16, ///< total 16 recordgain level
	KDRV_AUDIO_CAP_GAIN_LEVEL32 = 32, ///< total 32 recordgain level

	ENUM_DUMMY4WORD(KDRV_AUDIO_CAP_GAIN_LEVEL)
} KDRV_AUDIO_CAP_GAIN_LEVEL;

/**
    Embedded codec audio Capture Second Order IIR Filter Coeficients

    The second order IIR equation is H(z) =  SectionGain x ((B0 + B1*Z1 + B2*Z2) / (A0 + A1*Z1 + A2*Z2)) x (Total-Gain).
    This can be used to implement audio notch/lowpass/highpass filter as needed.

    @note For KDRV_AUDIOIO_CAP_IIRCOEF_L / KDRV_AUDIOIO_CAP_IIRCOEF_R
*/
typedef struct {
	INT32   total_gain;     ///< Total Gain
	INT32   section_gain;   ///< Section Gain

	INT32   coef_b0;        ///< Filter coefficient B0
	INT32   coef_b1;        ///< Filter coefficient B1
	INT32   coef_b2;        ///< Filter coefficient B2

	INT32   coef_a0;        ///< Filter coefficient A0
	INT32   coef_a1;        ///< Filter coefficient A1
	INT32   coef_a2;        ///< Filter coefficient A2
} KDRV_AUDIO_CAP_IIRCOEF, *PKDRV_AUDIO_CAP_IIRCOEF;


/*************************
*	Output related items:
**************************/

/**
    Audio output path

    Select the audio output path.
    Used in kdrv_audioio_set(KDRV_AUDIOIO_OUT_PATH,KDRV_AUDIO_OUT_PATH_*) / kdrv_audioio_get(KDRV_AUDIOIO_OUT_PATH)
*/
typedef enum {
	KDRV_AUDIO_OUT_PATH_NONE,		///< Turn off all the output path. (Default)
	KDRV_AUDIO_OUT_PATH_SPEAKER,	///< Output to speaker path
	KDRV_AUDIO_OUT_PATH_LINEOUT,	///< Output to lineout path
	KDRV_AUDIO_OUT_PATH_I2S,		///< Output to I2S path
	KDRV_AUDIO_OUT_PATH_ALL,        ///< Output to speaker + lineout path

	KDRV_AUDIO_OUT_PATH_MAX,
	ENUM_DUMMY4WORD(KDRV_AUDIO_OUT_PATH)
} KDRV_AUDIO_OUT_PATH;

/**
    Audio output mono select

    When KDRV_AUDIOIO_OUT_CHANNEL_NUMBER is set to 1, this is used to select this 1 channel is from left or right on physical path.
    Used in kdrv_audioio_set(KDRV_AUDIOIO_OUT_MONO_SEL, KDRV_AUDIO_OUT_MONO_*) / kdrv_audioio_get(KDRV_AUDIOIO_OUT_MONO_SEL)
*/
typedef enum {
	KDRV_AUDIO_OUT_MONO_LEFT,			///< LEFT channel (Default)
	KDRV_AUDIO_OUT_MONO_RIGHT,			///< RIGHT channel

	ENUM_DUMMY4WORD(KDRV_AUDIO_OUT_MONO)
} KDRV_AUDIO_OUT_MONO;


/*************************
*	Global items
**************************/

typedef struct {
	UINT32 handler;			///< the handler of hardware
	UINT32 curr_timecode;	///< timestamp of output buffer
	UINT32 channel_number;	///< the current count of channels
} KDRV_AUDIOIO_CB_INFO;

/**
    Audio i2s operate mode
    Used in kdrv_audioio_set(KDRV_AUDIOIO_GLOBAL_I2S_OPMODE, KDRV_AUDIO_I2S_OPMODE_*)
*/
typedef enum {
	KDRV_AUDIO_I2S_OPMODE_SLAVE,			///< i2s work at slave mode
	KDRV_AUDIO_I2S_OPMODE_MASTER,		///< i2s work at master mode

	ENUM_DUMMY4WORD(KDRV_AUDIO_I2S_OPMODE)
} KDRV_AUDIO_I2S_OPMODE;

/**
    Audio i2s data order type
    Used in kdrv_audioio_set(KDRV_AUDIOIO_GLOBAL_I2S_DATA_ORDER, KDRV_AUDIO_I2S_DATA_ORDER_*)
*/
typedef enum {
	KDRV_AUDIO_I2S_DATA_ORDER_TYPE1,		///< i2s data order type 1. Ex: TDM_ch = 8 [data_0][data_1][data_2][data_3][data_4][data_5][data_6][data_7]
											///<                            TDM_ch = 6 [data_0][data_1][data_2][data_3][data_4][data_5]
											///<                            TDM_ch = 4 [data_0][data_1][data_2][data_3]
											///<                            TDM_ch = 2 [data_0][data_1]

	KDRV_AUDIO_I2S_DATA_ORDER_TYPE2,		///< i2s data order type 2. Ex: TDM_ch = 8 [data_0][data_2][data_4][data_6][data_1][data_3][data_5][data_7]
											///<                            TDM_ch = 6 [data_0][data_2][data_4][data_1][data_3][data_5]
											///<                            TDM_ch = 4 [data_0][data_2][data_1][data_3]
											///<                            TDM_ch = 2 [data_0][data_1]
	ENUM_DUMMY4WORD(KDRV_AUDIO_I2S_DATA_ORDER)
} KDRV_AUDIO_I2S_DATA_ORDER;


typedef enum {
	/*
		Global GROUP settings are validate for both the audio capture/output path.
	*/
	KDRV_AUDIOIO_GLOBAL_BASE = 0x01000000,

	KDRV_AUDIOIO_GLOBAL_SAMPLE_RATE,		///< [capt/out][set/get][embd/i2s]:	Audio Sample Rate. Valid setting value are 8000/11025/12000/16000/24000/32000/44100/48000.
	KDRV_AUDIOIO_GLOBAL_IS_BUSY,			///< [capt/out][get][embd/i2s]:		Get the current engine is in busy because of record/playback ongoing. TRUE: is BUSY. FALSE: is IDLE.
	KDRV_AUDIOIO_GLOBAL_I2S_BIT_WIDTH,		///< [capt/out][set/get][i2s]:		I2S interafce bit-width per audio data channel. Valid input is 16 or 32.
	KDRV_AUDIOIO_GLOBAL_I2S_BITCLK_RATIO,	///< [capt/out][set/get][i2s]:		I2S interface bit-clk vs sample-rae ratio. Valid input is 32/64/128/256. others is illegal. default is 32.
	KDRV_AUDIOIO_GLOBAL_I2S_OPMODE,         ///< [capt/out][set/get][i2s]:      I2S interface is master or slave mode. Valid input is KDRV_AUDIO_I2S_OPMODE for master/slave mode. Default is master mode.
	KDRV_AUDIOIO_GLOBAL_I2S_DATA_ORDER,     ///< [capt/out][set/get][i2s]:		I2S interface data order setting. set detail please refer to KDRV_AUDIO_I2S_DATA_ORDER. Default is KDRV_AUDIO_I2S_DATA_ORDER_TYPE1.
											///<								get config detail please refer to KDRV_AUDIO_CH_OFS
	KDRV_AUDIOIO_GLOBAL_CLOCK_ALWAYS_ON,	///< [capt/out][set/get][embd/i2s]:	Audio clock always on control. TRUE to enable always ON. FALSE to dynamic ON/OFF.
	KDRV_AUDIOIO_GLOBAL_ISR_CB,
	KDRV_AUDIOIO_GLOBAL_CLK_RATIO_MATCH,


	/*
		Capture GROUP settings:
	*/
	KDRV_AUDIOIO_CAP_BASE = 0x02000000,
	KDRV_AUDIOIO_CAP_PATH,					///< [cap][set/get][embd/i2s]:	Switch the audio capture path source. Please use KDRV_AUDIO_CAP_PATH as path select parameter.
	KDRV_AUDIOIO_CAP_DEFAULT_CFG,			///< [cap][set][embd]:			Set the audio embedeed codec capture path default configurations. Please use KDRV_AUDIO_CAP_DEFSET as setting select number.
	KDRV_AUDIOIO_CAP_CHANNEL_NUMBER,		///< [cap][set/get][embd/i2s]:	Audio capture channel number.
											///<							Embedded codec capture path valid settings is 1/2.
											///<							I2S capture path valid settings is 1/2/4/6/8.
    KDRV_AUDIOIO_CAP_CHANNEL_DATATYPE,      ///<
	KDRV_AUDIOIO_CAP_MUTI_CH_MODE,			///< [cap][set/get][embd/i2s]:  1: Rx use muti-ch dma mode 0: Rx use 1ch dma mode
	KDRV_AUDIOIO_CAP_DMIC_CH,				///< [cap][set/get][embd/i2s]:  Digital mic channel number. valid setting is 2/4.
	KDRV_AUDIOIO_CAP_MONO_SEL,				///< [cap][set/get][embd/i2s]:	When KDRV_AUDIOIO_CAP_CHANNEL_NUMBER set to 1.
											///<							Use to to select this 1 channel from Left or Right on physical path.
											///<							Please use KDRV_AUDIO_CAP_MONO_LEFT or KDRV_AUDIO_CAP_MONO_RIGHT as input param.
	KDRV_AUDIOIO_CAP_MONO_EXPAND,			///< [cap][set/get][embd/i2s]:	When KDRV_AUDIOIO_CAP_CHANNEL_NUMBER set to 1, Expand the same mono data output to 2 channels on the DRAM buffer.
											///<							Set TRUE to ENABLE. FALSE to DISABLE.
	KDRV_AUDIOIO_CAP_BIT_WIDTH,				///< [cap][set/get][embd/i2s]:	Setting audio capture bit-width per channel. Valid setting value is 8/16/32.(Means PCM8/PCM16/PCM32)
	KDRV_AUDIOIO_CAP_VOLUME,				///< [cap][set/get][embd]:		Set audio capture volume. Normal input range is 0~100.<PGA gain>
	KDRV_AUDIOIO_CAP_VOLUME_LEFT,			///< [cap][set/get][embd]:		Same fucntion as KDRV_AUDIOIO_CAP_VOLUME but only set left channel
	KDRV_AUDIOIO_CAP_VOLUME_RIGHT,			///< [cap][set/get][embd]:		Same fucntion as KDRV_AUDIOIO_CAP_VOLUME but only set right channel											///<							0 is mute. 100 is normal maximum volume by using PGA gain.
											///<							101 ~ 200 is adding additional digital gain.
	KDRV_AUDIOIO_CAP_DCCAN_RESOLUTION,      ///< [cap][set/get][embd]:		Recording DC Cancellation Resolution. Valid setting range is 0~7.
	KDRV_AUDIOIO_CAP_TRIGGER_DELAY,         ///< [cap][set/get][embd]:		Embedded codec capture start delay, the MIC_BAIS would be enabled and the external regulator needs some stable time before record really start.
											///<							This delay is used to prevent recording this pop noise. The default value is 200 (mini-seconds). This setting unit is mini-seconds.
	KDRV_AUDIOIO_CAP_AMIC_BOOST,			///< [cap][set/get][embd]:		Embedded codec analog microphone pga boost gain. Valid setting value is 0/10/20/30.
	KDRV_AUDIOIO_CAP_ALC_EN,				///< [cap][set/get][embd]:		Embedded codec Auto level control ENABLE/DISABLE.
											///<							Default is Enabled after KDRV_AUDIOIO_CAP_DEFAULT_CFG is invoked.
	KDRV_AUDIOIO_CAP_ALC_MAXGAIN,			///< [cap][set/get][embd]:		Embedded codec ALC maximum PGA gain. Valid setting range is -21 ~ +26 (dB).
	KDRV_AUDIOIO_CAP_ALC_MINGAIN,           ///< [cap][set/get][embd]:		Embedded codec ALC minimum PGA gain. Valid setting range is -21 ~ +26 (dB).

	KDRV_AUDIOIO_CAP_ALC_ATTACK_TIME,       ///< [cap][set/get][embd]:		Embedded codec ALC Attack Time length.  Valid setting range is 0~10.
											///<							Time length would be "(2 ^ KDRV_AUDIOIO_CAP_ALC_ATTACK_TIME) x KDRV_AUDIOIO_CAP_ALC_TIME_RESOLUTION-Basis".
	KDRV_AUDIOIO_CAP_ALC_DECAY_TIME,        ///< [cap][set/get][embd]:		Embedded codec ALC Decay Time length. Valid setting range is 0~10.
											///<							Time length would be "(2 ^ KDRV_AUDIOIO_CAP_ALC_DECAY_TIME) x KDRV_AUDIOIO_CAP_ALC_TIME_RESOLUTION-Basis".
	KDRV_AUDIOIO_CAP_ALC_HOLD_TIME,         ///< [cap][set/get][embd]:		Embedded codec ALC Hold Time length. Valid setting range is 0~15.
											///<							Time length would be "(2 ^ (uiCfgValue-1)) x 117".
	KDRV_AUDIOIO_CAP_ALC_TIME_RESOLUTION,   ///< [cap][set/get][embd]:		Embedded codec ALC Basis Time Resolution for Attack/Decay Time Calculation.
											///<							Please use "KDRV_AUDIO_CAP_TRESO_BASIS" as input paramter.
	KDRV_AUDIOIO_CAP_NOISEGATE_EN,			///< [cap][set/get][embd]:		Embedded codec Noise Gate function ENABLE/DISABLE.
											///<							This setting is valid only if KDRV_AUDIOIO_CAP_ALC_EN is enabled.
											///<							Default is Enabled after KDRV_AUDIOIO_CAP_DEFAULT_CFG is invoked.
	KDRV_AUDIOIO_CAP_NOISEGATE_THRESHOLD,	///< [cap][set/get][embd]:		Embedded codec Noise gate threshold value. Valid setting range -30 ~ -77 (dB).
	KDRV_AUDIOIO_CAP_NG_BOOST_COMPENSATION,	///< [cap][set/get][embd]:		Embedded codec Noise gate function with boost gain compensation. 1/0 for enalbe/disable boost gain compensation. default is enable.
	KDRV_AUDIOIO_CAP_NOISEGAIN,				///< [cap][set/get][embd]:		Embedded codec Noise gain. (Valid value: 0x0~0xF). If the KDRV_AUDIOIO_CAP_NOISEGATE_EN enabled
											///<							The Noise gate Target Level = ALC_LEVEL - (NOISE_GATE_THRESHOLD - SIGNAL_LEVEL)*(1 + KDRV_AUDCAP_PARAM_NOISEGAIN).
	KDRV_AUDIOIO_CAP_ALCNG_ATTACK_TIME,     ///< [cap][set/get][embd]:		Embedded codec ALC Attack Time length in NoiseGate State. Valid setting range is 0~10.
											///<							Time length would be "(2 ^ KDRV_AUDIOIO_CAP_ALCNG_ATTACK_TIME) x KDRV_AUDIOIO_CAP_NOISEGATE_TIME_RESOLUTION-Basis".
	KDRV_AUDIOIO_CAP_ALCNG_DECAY_TIME,      ///< [cap][set/get][embd]:		Embedded codec ALC Decay Time length in NoiseGate State. Valid setting range is 0~10.
											///<							Time length would be "(2 ^ KDRV_AUDIOIO_CAP_ALCNG_DECAY_TIME) x KDRV_AUDIOIO_CAP_NOISEGATE_TIME_RESOLUTION-Basis".
	KDRV_AUDIOIO_CAP_NOISEGATE_TIME_RESOLUTION,///< [cap][set/get][embd]:	Embedded codec ALC Time Resolution in the NoiseGate state. Please use "KDRV_AUDIO_CAP_TRESO_BASIS" as input paramter.
	KDRV_AUDIOIO_CAP_ALC_IIR_EN,            ///< [cap][set/get][embd]:		Second Order IIR filter for the ALC function. (Default Disabled)
	KDRV_AUDIOIO_CAP_OUTPUT_IIR_EN,         ///< [cap][set/get][embd]:		Second Order IIR filter for the Recorded Output Data. (Default Disabled)
	KDRV_AUDIOIO_CAP_IIRCOEF_L,             ///< [cap][set][embd]:			Left Channel Embedded Second Order IIR filter Coeficients. Please use the address of  PKDRV_AUDIO_CAP_IIRCOEF as input parameter.
	KDRV_AUDIOIO_CAP_IIRCOEF_R,             ///< [cap][set][embd]:			Right Channel Embedded Second Order IIR filter Coeficients. Please use the address of  PKDRV_AUDIO_CAP_IIRCOEF as input parameter.

	KDRV_AUDIOIO_CAP_TIMECODE_HIT_EN,		///< [cap][set/get][embd/i2s]:	Set Embedded codec capture Time Code Hit Event enable.
	KDRV_AUDIOIO_CAP_TIMECODE_HIT_CB,       ///< [cap][set][embd/i2s]:		The callback event handle for dedicate capture channel.
											///<							The callback prototype is KDRV_CALLBACK_FUNC..
	KDRV_AUDIOIO_CAP_TIMECODE_TRIGGER,      ///< [cap][set/get][embd/i2s]:	The timecode trigger value.
											///<							Timecode is sample value. When is time code trigger value is hit, the timecode Callback would be called.
	KDRV_AUDIOIO_CAP_TIMECODE_OFFSET,       ///< [cap][set/get][embd/i2s]:	Assign the timecode start counting offset value.
	KDRV_AUDIOIO_CAP_TIMECODE_VALUE,        ///< [cap][get][embd/i2s]:		Get codec capture current timecode value..

    KDRV_AUDIOIO_CAP_UNDERRUN_COUNT,		///< [cap][get][i2s]:			Get record buffer underrun counter.
    KDRV_AUDIOIO_CAP_MAXQUEUE_COUNT,		///< [cap][get][i2s]:			Get record buffer queue maximum counter.
    KDRV_AUDIOIO_CAP_QUEUE_COUNT,			///< [cap][get][i2s]:			Get record buffer queue inserted counter.
    KDRV_AUDIOIO_CAP_DELAY_QUEUE_COUNT, 	///< [cap][set/get][i2s]:		Get record buffer queue delay start counter. delay = N, means engine will start when driver gets N+1 buffer.
    KDRV_AUDIOIO_CAP_FORCE_CALLBACK,		///< [cap][set][i2s]:			Force record un-start buffer to start and return callback.
    KDRV_AUDIOIO_CAP_RESET,

    KDRV_AUDIOIO_CAP_PDVCMBIAS_ALWAYS_ON,	///< [cap][set][embd]:			Let ADVCM always power-on for faster Mic stable time. [Default disabled]
	KDRV_AUDIOIO_CAP_GAIN_LEVEL,	        ///< [cap][set/get][embd]:		Embedded codec record gain level.


	/*
		Output GROUP settings:
	*/
	KDRV_AUDIOIO_OUT_BASE = 0x04000000,
	KDRV_AUDIOIO_OUT_PATH,					///< [out][set/get][embd/i2s]:	Switch the audio output path source. Please use KDRV_AUDIO_OUT_PATH as path select parameter.
	KDRV_AUDIOIO_OUT_HDMI_PATH_EN,			///< [out][set/get][embd/i2s]:	Select HDMI output path ENABLE/DISABLE.
	KDRV_AUDIOIO_OUT_CHANNEL_NUMBER,		///< [out][set/get][embd/i2s]:	Audio output channel number.
											///<							Embedded codec output path valid settings is 1/2.
											///<							I2S output path valid settings is 1/2/4/6/8.
    KDRV_AUDIOIO_OUT_CHANNEL_DATATYPE,		///<
	KDRV_AUDIOIO_OUT_MUTI_CH_MODE,			///< [out][set/get][embd/i2s]:  1: Tx use muti-ch dma mode 0: Tx use 1ch dma mode
	KDRV_AUDIOIO_OUT_MONO_SEL,				///< [out][set/get][embd/i2s]:	When KDRV_AUDIOIO_OUT_CHANNEL_NUMBER set to 1.
											///<							Use to to select this 1 channel from Left or Right on physical path.
											///<							Please use KDRV_AUDIO_OUT_MONO_LEFT or KDRV_AUDIO_OUT_MONO_RIGHT as input param.
	KDRV_AUDIOIO_OUT_MONO_EXPAND,			///< [out][set/get][embd/i2s]:	When KDRV_AUDIOIO_OUT_CHANNEL_NUMBER set to 1, Expand the same mono data output to 2 channels output path.
											///<							This can be used in the HDMI application.
											///<							Set TRUE to ENABLE. FALSE to DISABLE.
	KDRV_AUDIOIO_OUT_BIT_WIDTH,				///< [out][set/get][embd/i2s]:	Setting audio output source data bit-width per channel. Valid setting value is 8/16/32.(Means PCM8/PCM16/PCM32)
	KDRV_AUDIOIO_OUT_VOLUME,				///< [out][set/get][embd]:		Set audio output volume. Normal input range is 0~100.<PGA gain>
											///<							0 is mute. 100 is normal maximum volume by using PGA gain.
											///<							101 ~ 200 is adding additional digital gain.
	KDRV_AUDIOIO_OUT_VOLUME_LEFT,			///< [out][set/get][embd]:		Set audio output left channel volume. 0 is mute. 1 ~ 200 is  digital gain.
	KDRV_AUDIOIO_OUT_VOLUME_RIGHT,			///< [out][set/get][embd]:		Set audio output right channel volume. 0 is mute. 1 ~ 200 is  digital gain.
	KDRV_AUDIOIO_OUT_SPK_PWR_ALWAYS_ON,		///< [out][set/get][embd]:		Speaker output path power always ON control. TRUE to enable always ON. FALSE to dynamic ON/OFF.(Default disabled)
	KDRV_AUDIOIO_OUT_LINE_PWR_ALWAYS_ON,	///< [out][set/get][embd]:		Lineout output path power always ON control. TRUE to enable always ON. FALSE to dynamic ON/OFF.(Default disabled)

	KDRV_AUDIOIO_OUT_TIMECODE_HIT_EN,       ///< [out][set/get][embd/i2s]:	Set Time Code Hit Event ENABLE/DISABLE.
											///<							Timecode is sample value. When is time code trigger value is hit, the timecode Callback would be called.
	KDRV_AUDIOIO_OUT_TIMECODE_HIT_CB,       ///< [out][set][embd/i2s]:		The callback event handle for dedicate output channel.
											///<							The callback prototype is KDRV_CALLBACK_FUNC.
	KDRV_AUDIOIO_OUT_TIMECODE_TRIGGER,      ///< [out][set/get][embd/i2s]:	The timecode trigger value.
											///<							Timecode is sample value. When is time code trigger value is hit, the timecode Callback would be called.
	KDRV_AUDIOIO_OUT_TIMECODE_OFFSET,       ///< [out][set/get][embd/i2s]:	Assign the timecode start counting offset value.
	KDRV_AUDIOIO_OUT_TIMECODE_VALUE,        ///< [out][get][embd/i2s]:		Get codec output current timecode value.

    KDRV_AUDIOIO_OUT_UNDERRUN_COUNT,		///< [out][get][i2s]:			Get playback buffer underrun counter.
    KDRV_AUDIOIO_OUT_MAXQUEUE_COUNT,		///< [out][get][i2s]:			Get playback buffer queue maximum counter.
    KDRV_AUDIOIO_OUT_QUEUE_COUNT,			///< [out][get][i2s]:			Get playback buffer queue inserted counter.
    KDRV_AUDIOIO_OUT_DELAY_QUEUE_COUNT,		///< [out][set/get][i2s]:		Get playback buffer queue delay start counter. delay = N, means engine will start when driver gets N+1 buffer.
    KDRV_AUDIOIO_OUT_FORCE_CALLBACK,		///< [out][set][i2s]:			Force playback un-start buffer to start and return callback.	KDRV_AUDIOIO_OUT_RESET,
    KDRV_AUDIOIO_OUT_RESET,

	KDRV_AUDIOIO_OUT_STOP_TX,               ///< [out][set/get][embd]:      Stop playback operation.
	KDRV_AUDIOIO_PARAM_ID_MAX,
	ENUM_DUMMY4WORD(KDRV_AUDIOIO_PARAM_ID)
} KDRV_AUDIOIO_PARAM_ID;


/*!
 * @fn INT32 kdrv_audioio_open(UINT32 chip, UINT32 engine)
 * @brief open hardware engine
 * @param chip		the chip id of hardware
 * @param engine	the engine id of hardware. Please use KDRV_AUDCAP_ENGINE0 / KDRV_AUDCAP_ENGINE1 / KDRV_AUDOUT_ENGINE0 / KDRV_AUDOUT_ENGINE1.
 * @return return 0 on success, -1 on error
 */
KDRV_AUD_HANDLE kdrv_audioio_open(UINT32 chip, KDRV_DEV_ENGINE engine);


/*!
 * @fn INT32 kdrv_audioio_close(UINT32 chip, UINT32 engine)
 * @brief close hardware engine
 * @param chip		the chip id of hardware
 * @param engine	the engine id of hardware. Please use KDRV_AUDCAP_ENGINE0 / KDRV_AUDCAP_ENGINE1 / KDRV_AUDOUT_ENGINE0 / KDRV_AUDOUT_ENGINE1.
 * @return return 0 on success, -1 on error
 */
INT32 kdrv_audioio_close(KDRV_AUD_HANDLE hdl_kdrv);


/*!
 * @fn INT32 kdrv_audioio_set(UINT32 handler, KDRV_AUDIOIO_PARAM_ID id, VOID *param)
 * @brief set parameters to hardware engine
 * @param id	    the handler of hardware
 * @param param_id	the param_id of parameters
 * @param param		the parameters
 * @return return 0 on success, -1 on error
 */
KDRV_RETURN_STATUS __kdrv_audioio_set(KDRV_AUD_HANDLE hdl_kdrv, KDRV_AUDIOIO_PARAM_ID param_id, VOID *param);
#define kdrv_audioio_set(hdl_kdrv, param_id, param)  __kdrv_audioio_set((KDRV_AUD_HANDLE)(hdl_kdrv), (KDRV_AUDIOIO_PARAM_ID)(param_id), (VOID *)(param))


/*!
 * @fn INT32 kdrv_audioio_get(UINT32 handler, KDRV_AUDIOIO_PARAM_ID param_id, VOID *param)
 * @brief set parameters to hardware engine
 * @param id	    the handler of hardware
 * @param param_id	the param_id of parameters
 * @param param		the parameters
 * @return return 0 on success, -1 on error
 */
KDRV_RETURN_STATUS __kdrv_audioio_get(KDRV_AUD_HANDLE hdl_kdrv, KDRV_AUDIOIO_PARAM_ID param_id, VOID *param);
#define kdrv_audioio_get(hdl_kdrv, param_id, param)  __kdrv_audioio_get((KDRV_AUD_HANDLE)(hdl_kdrv), (KDRV_AUDIOIO_PARAM_ID)(param_id), (VOID *)(param))


/*!
 * @fn INT32 kdrv_audioio_trigger(UINT32 handler,
							KDRV_BUFFER_INFO *p_au_frame_buffer,
							KDRV_CALLBACK_FUNC *p_cb_func,
							VOID *user_data);
 * @brief trigger hardware engine
 * @param id					the handler of hardware
 * @param p_in_au_frame_buffer	the input audio frame buffer
 * @param p_cb_func				the callback function
 * @param user_data				the private user data
 * @return return 0 on success, -1 on error
 */
KDRV_RETURN_STATUS kdrv_audioio_trigger(KDRV_AUD_HANDLE hdl_kdrv,
                    KDRV_AUDIO_TRIGGER_TYPE trigger_type,
                    KDRV_AUDIO_TRIGGER_INFO *p_triger_info);


/*!
 * @fn INT32 kdrv_audioio_trigger_not_start(UINT32 handler,
							KDRV_BUFFER_INFO *p_au_frame_buffer,
							KDRV_CALLBACK_FUNC *p_cb_func,
							VOID *user_data);
 * @brief trigger hardware engine but only insert buffer and won't start engine
 * @param id     				the handler of hardware
 * @param p_in_au_frame_buffer	the input audio frame buffer
 * @param p_cb_func 			the callback function
 * @param user_data 			the private user data
 * @return return 0 on success, -1 on error
 */
KDRV_RETURN_STATUS kdrv_audioio_trigger_not_start(KDRV_AUD_HANDLE hdl_kdrv,
                    KDRV_AUDIO_TRIGGER_TYPE trigger_type,
                    KDRV_AUDIO_TRIGGER_INFO *p_triger_info);


void kdrv_audioio_abort(void);
void kdrv_audio_init(void);
void kdrv_audio_exit(void);


#endif
