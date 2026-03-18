/**ADDR
    Digital Audio Interface(DAI) Controller global header

    This file is the header file that define the API and data type
    for DAI module

    @file       dai.h
    @ingroup    mIDrvAud_DAI
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2017.  All rights reserved.
*/



#ifndef _DAI_H
#define _DAI_H

#ifdef __KERNEL__
#include "kwrap/type.h"
#elif defined(__FREERTOS)
#include "kwrap/type.h"
#endif

typedef void (*DRV_CB)(ULONG event);

#define PLATFORM_MAX_SSP_CNT 1 // dai,dai2,dai3, dai4, dai5
#define PLATFORM_MAX_CHIP_CNT 1
#define PLATFORM_TOATL_SSP_CNT PLATFORM_MAX_SSP_CNT * PLATFORM_MAX_CHIP_CNT

/**
    @addtogroup mIDrvAud_DAI
*/
//@{



/**
    Digital Audio interface(DAI) general configuration Selection.

    This is used in dai_set_config() to specify which of the function is selected to assign new configuration.
*/
typedef enum {
	DAI_CONFIG_ID_EXTCODEC_EN,          ///< Enable/Disable External Codec. If external codec is disabled, the internal codec is enabled.
	DAI_CONFIG_ID_ISRCB,                ///< Assign ISR Callback event handler. Please use "DRV_CB" as callback prototype.
										///< This is valid only if dai driver is opened.
	DAI_CONFIG_ID_HDMI_TXEN,            ///< Set DAI Transmit to HDMI Enable/Disable. If disabled, the HDMI audio would be forced MUTE.
	DAI_CONFIG_ID_AVSYNC_EN,            ///< Set Record AV Sync Function Enable.
	DAI_CONFIG_ID_AVSYNC_SRC,           ///< Set Record AV Sync Source from SIE or SIE2.

	DAI_CONFIG_ID_CLKSRC,               ///< Select Codec system clock source. Please use "DAI_CODESCK" as input parameter.
										///< Only DAI_CODESCK_INT is valid for NT96680.
	DAI_CONFIG_ID_SET_INTEN,            ///< Set DAI controller interrupt enable. Please use "DAI_INTERRUPT" as bitwise input parameter.
	DAI_CONFIG_ID_SET_INTEN_MUTIDONE,   ///< Set DAI controller interrupt enable. Please use "DAI_INTERRUPT_MUTIDONE" as bitwise input parameter.
	DAI_CONFIG_ID_CLR_INTEN_MUTIDONE,
	DAI_CONFIG_ID_CLR_INTEN_MUTILOAD,
	DAI_CONFIG_ID_CLR_INTEN,            ///< Clear DAI controller interrupt enable. Please use "DAI_INTERRUPT" as bitwise input parameter.
	DAI_CONFIG_ID_CLR_INTSTS,           ///< Clear DAI controller interrupt status. Please use "DAI_INTERRUPT" as bitwise input parameter.

	DAI_CONFIG_ID_RX_SRC_SEL,			///< Select rx source from i2s or embedded codec. Please use "DAI_RX_SRC" as input parameter.
	DAI_CONFIG_ID_DMA_MODE_SEL,			///< Select DAI dma mode. Please use "DAI_DMA_MODE" as input parameter.

    DAI_CONFIG_ID_STADR2_EN,
    
	ENUM_DUMMY4WORD(DAI_CONFIG_ID)
} DAI_CONFIG_ID;

/**
    DAI Interrupt

    @note Used for dai_set_config(DAI_CONFIG_ID_SET_INTEN / DAI_CONFIG_ID_CLR_INTEN).
*/
typedef enum {
	DAI_TX1DMADONE_INT      = 0X00000001,///< DMA buffer done for Playback Channel 1 (TX1)
	DAI_TX2DMADONE_INT      = 0X00000002,///< DMA buffer done for Playback Channel 2 (TX2)
	DAI_RX1DMADONE_INT      = 0X00000004,///< DMA buffer done for   Record Channel 1 (RX1)
	DAI_RX2DMADONE_INT      = 0X00000008,///< DMA buffer done for   Record Channel 2 (RX2)

	DAI_TX1DMALOAD_INT      = 0x00000010,///< DMA start address loaded for Playback Channel 1 (TX1)
	DAI_TX2DMALOAD_INT      = 0x00000020,///< DMA start address loaded for Playback Channel 1 (TX1)
	DAI_RXDMALOAD_INT       = 0x00000040,///< DMA start address loaded for Record Channel (RX-1&2)

	DAI_TX1STOP_INT         = 0x00000100,///< DAI Playback Channel 1 stopped. (TX1)
	DAI_TX2STOP_INT         = 0x00000200,///< DAI Playback Channel 2 stopped. (TX2)
	DAI_RXSTOP_INT          = 0x00000400,///< DAI Record Channel stopped. (RX-1&2)

	DAI_TX1BWERR_INT        = 0x00001000,///< DAI Playback Channel 1 BandWidth Not enough.(TX1)
	DAI_TX2BWERR_INT        = 0x00002000,///< DAI Playback Channel 2 BandWidth Not enough.(TX2)
	DAI_RX1BWERR_INT        = 0x00004000,///< DAI Record Channel 1 BandWidth Not enough.(RX1)
	DAI_RX2BWERR_INT        = 0x00008000,///< DAI Record Channel 2 BandWidth Not enough.(RX2)

	DAI_TX1TCHIT_INT        = 0x00010000,///< Time Code Hit for Playback Channel 1.(TX1)
	DAI_RXTCHIT_INT         = 0x00040000,///< Time Code Hit for Record Channe.(RX-1&2)

	DAI_RXTCLATCH_INT       = 0x00100000,///< Time Code Latch for Record Channe.(RX-1&2)
	DAI_I2S_CLK_CHG_INT     = 0x10000000,///< I2S_CLK_CHG_INT in slave mode>

#if defined(__FREERTOS) //wait rtos flag issue fixed can be removed.
	DAI_TXLBDMADONE_INT 	= 0x00000080,///< DMA buffer done for Playback Loopback Channel(TXLB)
	DAI_TXLBDMALOAD_INT 	= 0x00000800,///< DMA start address loaded for Playback Loopback Channel(TXLB)
	DAI_TXLBSTOP_INT		= 0x00020000,///< DAI Playback Loopback Channel stopped. (TXLB)
	DAI_TXLBBWERR_INT		= 0x00080000,///< DAI Playback Loopback Channel BandWidth Not enough.(TXLB)

	DAI_INTERRUPT_ALL		= 0x101FFFFF,///< DAI All interrupt
	DAI_INTERRUPT_TXLB_ALL	= 0x000A0880,///< DAI TXLB All interrupt//0x000A0880

#else
	DAI_TXLBDMADONE_INT     = 0X01000000,///< DMA buffer done for Playback Loopback Channel(TXLB)
	DAI_TXLBDMALOAD_INT     = 0x02000000,///< DMA start address loaded for Playback Loopback Channel(TXLB)
	DAI_TXLBSTOP_INT        = 0x04000000,///< DAI Playback Loopback Channel stopped. (TXLB)
	DAI_TXLBBWERR_INT       = 0x08000000,///< DAI Playback Loopback Channel BandWidth Not enough.(TXLB)

	DAI_INTERRUPT_ALL       = 0x1F15F77F,///< DAI All interrupt
	DAI_INTERRUPT_TXLB_ALL  = 0x0F000000,///< DAI TXLB All interrupt
#endif
	ENUM_DUMMY4WORD(DAI_INTERRUPT)
} DAI_INTERRUPT;

/**
    DAI Interrupt

    @note Used for dai_set_config(DAI_CONFIG_ID_SET_INTEN / DAI_CONFIG_ID_CLR_INTEN).
*/
typedef enum {
	DAI_CH01_DMADONE_INT      = 0X00000001,///< DMA buffer done for DMA Channel 1
	DAI_CH02_DMADONE_INT      = 0X00000002,///< DMA buffer done for DMA Channel 2
	DAI_CH03_DMADONE_INT      = 0X00000004,///< DMA buffer done for DMA Channel 3
	DAI_CH04_DMADONE_INT      = 0X00000008,///< DMA buffer done for DMA Channel 4

	DAI_CH05_DMADONE_INT      = 0X00000010,///< DMA buffer done for DMA Channel 5
	DAI_CH06_DMADONE_INT      = 0X00000020,///< DMA buffer done for DMA Channel 6
	DAI_CH07_DMADONE_INT      = 0X00000040,///< DMA buffer done for DMA Channel 7
	DAI_CH08_DMADONE_INT      = 0X00000080,///< DMA buffer done for DMA Channel 8

	DAI_CH09_DMADONE_INT      = 0X00000100,///< DMA buffer done for DMA Channel 9
	DAI_CH10_DMADONE_INT      = 0X00000200,///< DMA buffer done for DMA Channel 10
	DAI_CH11_DMADONE_INT      = 0X00000400,///< DMA buffer done for DMA Channel 11
	DAI_CH12_DMADONE_INT      = 0X00000800,///< DMA buffer done for DMA Channel 12

	DAI_CH13_DMADONE_INT      = 0X00001000,///< DMA buffer done for DMA Channel 13
	DAI_CH14_DMADONE_INT      = 0X00002000,///< DMA buffer done for DMA Channel 14
	DAI_CH15_DMADONE_INT      = 0X00004000,///< DMA buffer done for DMA Channel 15
	DAI_CH16_DMADONE_INT      = 0X00008000,///< DMA buffer done for DMA Channel 16

	DAI_CH17_DMADONE_INT      = 0X00010000,///< DMA buffer done for DMA Channel 17
	DAI_CH18_DMADONE_INT      = 0X00020000,///< DMA buffer done for DMA Channel 18
	DAI_CH19_DMADONE_INT      = 0X00040000,///< DMA buffer done for DMA Channel 19
	DAI_CH20_DMADONE_INT      = 0X00080000,///< DMA buffer done for DMA Channel 20
	DAI_CH21_DMADONE_INT      = 0X00100000,///< DMA buffer done for DMA Channel 21


	DAI_CH_1CH_DMADONE_INT    = 0X00000001,///< DMA buffer done for ALL DMA Channel
	DAI_CH_20CH_DMADONE_INT   = 0x001FFFFE,///< DMA buffer done for ALL DMA Channel
	DAI_CH_ALL_DMADONE_INT    = 0x001FFFFF,///< DMA buffer done for ALL DMA Channel

	ENUM_DUMMY4WORD(DAI_INTERRUPT_MUTIDONE)
} DAI_INTERRUPT_MUTIDONE;


/**
    DAI Address Same Address2 

    @note Used for dai_set_rx_muti_ch_dma_para2
*/
typedef enum {
	DAI_CH01_SAM      = 0X00000001,///< DMA buffer done for DMA Channel 1
	DAI_CH02_SAM      = 0X00000002,///< DMA buffer done for DMA Channel 2
	DAI_CH03_SAM      = 0X00000004,///< DMA buffer done for DMA Channel 3
	DAI_CH04_SAM      = 0X00000008,///< DMA buffer done for DMA Channel 4

	DAI_CH05_SAM      = 0X00000010,///< DMA buffer done for DMA Channel 5
	DAI_CH06_SAM      = 0X00000020,///< DMA buffer done for DMA Channel 6
	DAI_CH07_SAM      = 0X00000040,///< DMA buffer done for DMA Channel 7
	DAI_CH08_SAM      = 0X00000080,///< DMA buffer done for DMA Channel 8

	DAI_CH09_SAM      = 0X00000100,///< DMA buffer done for DMA Channel 9
	DAI_CH10_SAM      = 0X00000200,///< DMA buffer done for DMA Channel 10
	DAI_CH11_SAM      = 0X00000400,///< DMA buffer done for DMA Channel 11
	DAI_CH12_SAM      = 0X00000800,///< DMA buffer done for DMA Channel 12

	DAI_CH13_SAM      = 0X00001000,///< DMA buffer done for DMA Channel 13
	DAI_CH14_SAM      = 0X00002000,///< DMA buffer done for DMA Channel 14
	DAI_CH15_SAM      = 0X00004000,///< DMA buffer done for DMA Channel 15
	DAI_CH16_SAM      = 0X00008000,///< DMA buffer done for DMA Channel 16

	DAI_CH17_SAM      = 0X00010000,///< DMA buffer done for DMA Channel 17
	DAI_CH18_SAM      = 0X00020000,///< DMA buffer done for DMA Channel 18
	DAI_CH19_SAM      = 0X00040000,///< DMA buffer done for DMA Channel 19
	DAI_CH20_SAM      = 0X00080000,///< DMA buffer done for DMA Channel 20
	DAI_CH21_SAM      = 0X00100000,///< DMA buffer done for DMA Channel 21


	DAI_CH_1CH_SAM   = 0X00000001,///< DMA buffer done for ALL DMA Channel
	DAI_CH_20CH_SAM  = 0x001FFFFE,///< DMA buffer done for ALL DMA Channel
	DAI_CH_ALL_SAM   = 0x001FFFFF,///< DMA buffer done for ALL DMA Channel

	ENUM_DUMMY4WORD(DAI_DAI_MULT_SAM)
} DAI_MULT_SAM;
/**
    DAI system clock source

    @note Used for dai_set_config(DAI_CONFIG_ID_CLKSRC).
*/
typedef enum {
	DAI_CODESCK_INT,                    ///< internal clock
	DAI_CODESCK_EXT,                    ///< external clock

	ENUM_DUMMY4WORD(DAI_CODESCK)
} DAI_CODESCK;

/**
    DAI Rx source select

    @note Used for dai_setConfig(DAI_CONFIG_ID_RX_SRC_SEL).
*/
typedef enum {
	DAI_RX_SRC_EMBEDDED,               ///< record source from embedded codec
	DAI_RX_SRC_I2S,                    ///< record source from i2s

	ENUM_DUMMY4WORD(DAI_RX_SRC)
} DAI_RX_SRC;

/**
    DAI DMA mode select

    @note Used for dai_setConfig(DAI_CONFIG_ID_DMA_MODE_SEL).
*/
typedef enum {
	DAI_DMA_MODE_1CHRX_20CHTX,         ///< DAI DMA = 1ch Rx + 20 ch Tx
	DAI_DMA_MODE_1CHTX_20CHRX,         ///< DAI DMA = 1ch Tx + 20 ch Rx

	ENUM_DUMMY4WORD(DAI_DMA_MODE)
} DAI_DMA_MODE;




/**
    DAI Playback Channel Selection ID

    @note Used for dai_set_tx_config() to select specified playback channel configuration.
*/
typedef enum {
	DAI_TXCH_TX1,                       ///< DAI Playback Channel 1 (TX1) Select ID
	DAI_TXCH_TX2,                       ///< DAI Playback Channel 2 (TX2) Select ID

	ENUM_DUMMY4WORD(DAI_TXCH)
} DAI_TXCH;

/**
    DAI playback configuration select

    @note Used for dai_set_tx_config(DAI_TXCH) and dai_get_tx_config(DAI_TXCH).
*/
typedef enum {
	DAI_TXCFG_ID_CHANNEL,               ///< Configure playback channel as mono-left/right or stereo on the audio interface.
										///< Please use "DAI_CH" as input parameter. DAI_CH_DUAL_MONO is not valid setting for playback.
	DAI_TXCFG_ID_TOTAL_CH,              ///< Configure Total Channel Number on interface. If embedded audio codec is selected, this field must be set to DAI_TOTCH_2CH.
										///< Please use "DAI_TOTCH" as input parameter.
	DAI_TXCFG_ID_PCMLEN,                ///< Configure playback source PCM bit length. Please use  "DAI_DRAMPCMLEN" as input parameter.
	DAI_TXCFG_ID_DRAMCH,                ///< Configure the playback source bitstream on the DRAM is stereo or mono audio data.
										///< Please use "DAI_DRAMSOUNDFMT" as input parameter.
	DAI_TXCFG_ID_TIMECODE_TRIG,         ///< Configure Playback Time Code Trigger Value. This is valid only for DAI_TXCH_TX1.
										///< This field is valid for the 32bits value.
	DAI_TXCFG_ID_TIMECODE_OFS,          ///< Configure Playback Time Code Offset Value. This is valid only for DAI_TXCH_TX1.
										///< This field is valid for the 32bits value.
	DAI_TXCFG_ID_TIMECODE_VAL,          ///< Get Playback TimeCode value. This is valid only for dai_get_tx_config(DAI_TXCH_TX1,DAI_TXCFG_ID_TIMECODE_VAL).

    DAI_TXCFG_ID_ADDR_LOAD,
    DAI_TXCFG_ID_DMA_DONE,

	ENUM_DUMMY4WORD(DAI_TXCFG_ID)
} DAI_TXCFG_ID;

/**
    DAI playback loopback configuration select

    @note Used for dai_set_txlb_config() and dai_get_txlb_config().
*/
typedef enum {
	DAI_TXLBCFG_ID_CHANNEL = 0x1000,    ///< Configure playback loopback channel as mono-left/right or stereo on the audio interface.
										///< Please use "DAI_CH" as input parameter. DAI_CH_DUAL_MONO is not valid setting for playback loopback.
	DAI_TXLBCFG_ID_TOTAL_CH,            ///< Configure Total Channel Number on interface. If embedded audio codec is selected, this field must be set to DAI_TOTCH_2CH.
										///< Please use "DAI_TOTCH" as input parameter.
	DAI_TXLBCFG_ID_PCMLEN,              ///< Configure playback loopback source PCM bit length. Please use  "DAI_DRAMPCMLEN" as input parameter.
	DAI_TXLBCFG_ID_DRAMCH,              ///< Configure the playback loopback source bitstream on the DRAM is stereo or mono audio data.
										///< Please use "DAI_DRAMSOUNDFMT" as input parameter.
	DAI_TXLBCFG_ID_RXSYNC,              ///< ENABLE/DISABLE the TXLB start operation sync to RX start.

    DAI_TXLBCFG_ID_ADDR_LOAD,
    DAI_TXLBCFG_ID_DMA_DONE,

	ENUM_DUMMY4WORD(DAI_TXLBCFG_ID)
} DAI_TXLBCFG_ID;

/**
    DAI record configuration select

    @note Used for dai_set_rx_config() and dai_get_rx_config().
*/
typedef enum {
	DAI_RXCFG_ID_CHANNEL = 0x2000,      ///< Configure record channel as mono-left/right, dual-mono or stereo on the audio interface.
										///< Please use "DAI_CH" as input parameter.
	DAI_RXCFG_ID_TOTAL_CH,              ///< Configure Total Channel Number on interface. If embedded audio codec is selected, this field must be set to DAI_TOTCH_2CH.
										///< Please use "DAI_TOTCH" as input parameter.
	DAI_RXCFG_ID_PCMLEN,                ///< Configure recorded destination PCM bit length on DRAM. Please use  "DAI_DRAMPCMLEN" as input parameter.
	DAI_RXCFG_ID_DRAMCH,                ///< Configure the recording destination bitstream on the DRAM is stereo or mono audio data.
										///< Please use "DAI_DRAMSOUNDFMT" as input parameter.
	DAI_RXCFG_ID_TIMECODE_TRIG,         ///< Configure Record Channel Time Code Trigger Value. This field is valid for the 32bits value.
	DAI_RXCFG_ID_TIMECODE_OFS,          ///< Configure Record Time Code Offset Value. This field is valid for the 32bits value.
	DAI_RXCFG_ID_TIMECODE_VAL,          ///< Get Record TimeCode value. This is valid only for dai_get_tx_config(DAI_RXCFG_ID_TIMECODE_VAL).

    DAI_RXCFG_ID_ADDR_LOAD,
    DAI_RXCFG_ID_DMA_DONE,
    DAI_RX2CFG_ID_DMA_DONE,

	ENUM_DUMMY4WORD(DAI_RXCFG_ID)
} DAI_RXCFG_ID;

/**
    DAI Playback(TX)/Record(RX) channel

    @note Used for dai_set_tx_config(DAI_TXCH, DAI_TXCFG_ID_CHANNEL) & dai_set_rx_config(DAI_RXCFG_ID_CHANNEL).
*/
typedef enum {
	DAI_CH_DUAL_MONO,                   ///< Dual Mono Channels. This is valid only for Record.
	DAI_CH_STEREO,                      ///< Stereo Chennels.
	DAI_CH_MONO_LEFT,                   ///< Mono Left Channel
	DAI_CH_MONO_RIGHT,                  ///< Mono Right Channel

	ENUM_DUMMY4WORD(DAI_CH)
} DAI_CH;

/**
    Total Audio Channels transceive on interface.

    If embedded audio codec is selected, only DAI_TOTCH_2CH is valid. 4/6/8-CH are valid for I2S only.

    @note Used for dai_set_tx_config(DAI_TXCH, DAI_TXCFG_ID_TOTAL_CH) & dai_set_rx_config(DAI_RXCFG_ID_TOTAL_CH).
*/
typedef enum {
	DAI_TOTCH_2CH,                      ///< Total 2 Audio Channels on interface.
	DAI_TOTCH_4CH,                      ///< Total 4 Audio Channels on interface. Valid only for I2S.
	DAI_TOTCH_6CH,                      ///< Total 6 Audio Channels on interface. Valid only for I2S.
	DAI_TOTCH_8CH,                      ///< Total 8 Audio Channels on interface. Valid only for I2S.
	DAI_TOTCH_10CH = 4,                      ///< Total 8 Audio Channels on interface. Valid only for I2S.
	DAI_TOTCH_16CH = 7,                     ///< Total16 Audio Channels on interface. Valid only for I2S.
	DAI_TOTCH_20CH = 9,                     ///< Total20 Audio Channels on interface. Valid only for I2S.

	ENUM_DUMMY4WORD(DAI_TOTCH)
} DAI_TOTCH;

/**
    DAI DRAM format

    @note Used for dai_set_tx_config(DAI_TXCH, DAI_TXCFG_ID_DRAMCH) & dai_set_rx_config(DAI_TXCFG_ID_DRAMCH).
*/
typedef enum {
	DAI_DRAMPCM_STEREO,                 ///< Stereo
	DAI_DRAMPCM_MONO,                   ///< Mono

	ENUM_DUMMY4WORD(DAI_DRAMSOUNDFMT)
} DAI_DRAMSOUNDFMT;

/**
    DAI PCM length

    @note Used for @note Used for dai_set_tx_config(DAI_TXCH, DAI_TXCFG_ID_PCMLEN) & dai_set_rx_config(DAI_RXCFG_ID_PCMLEN).
*/
typedef enum {
	DAI_DRAMPCM_8,                      ///< 8bits
	DAI_DRAMPCM_16,                     ///< 16bits
	DAI_DRAMPCM_32,                     ///< 32bits. Valid only for I2S Only.

	ENUM_DUMMY4WORD(DAI_DRAMPCMLEN)
} DAI_DRAMPCMLEN;



/**
    DAI I2S interface general configuration Selection

    This is used in dai_set_i2s_config() to specify which of the function is selected to assign new configuration.
*/
typedef enum {
	DAI_I2SCONFIG_ID_CLKRATIO,          ///< Select I2S clock ratio setting.
										///< Please use "DAI_I2SCLKR" as input parameter. Available values are below:
										///< \n DAI_I2SCLKR_256FS_32BIT:  SystemClk = 256 FrameSync, FrameSync =  32 BitClk
										///< \n DAI_I2SCLKR_256FS_64BIT:  SystemClk = 256 FrameSync, FrameSync =  64 BitClk
										///< \n DAI_I2SCLKR_256FS_128BIT: SystemClk = 256 FrameSync, FrameSync = 128 BitClk
										///< \n DAI_I2SCLKR_256FS_256BIT: SystemClk = 256 FrameSync, FrameSync = 256 BitClk
	DAI_I2SCONFIG_ID_FORMAT,            ///< Configure I2S Format. Please use "DAI_I2SFMT" as input parameter.
										///< Only DAI_I2SFMT_STANDARD is valid for NT96680.
	DAI_I2SCONFIG_ID_OPMODE,            ///< Configure I2S Operating mode as Master or Slave. Please use "DAI_OPMODE" as input parameter.
	DAI_I2SCONFIG_ID_CHANNEL_LEN,       ///< Configure I2S interface how many bit-clock cycles per audio channel.
										///< Please use "DAI_I2SCHLEN" as input parameter.
	DAI_I2SCONFIG_ID_HDMICH_SEL,        ///< During I2S TDM 4/6/8 CH playback, this field select which two channels to HDMI audio.
										///< Please use "DAI_I2SHDMI_SEL" as input parameters.
	DAI_I2SCONFIG_ID_SLAVEMATCH,        ///< Get I2S Slave Clock Ratio Setting is matched with the settings or not.
	DAI_I2SCONFIG_ID_CURRENT_CLKRATIO,  ///< Get I2S Slave Current Clock Ratio

	DAI_I2SCONFIG_ID_CLK_INV,			///< Set 1/0 to ENABLE/DISABLE inverse I2S clk phase. default is disable.
	DAI_I2SCONFIG_ID_DATA_ORDER,		///< Select the data order for TDM channel. Please use "DAI_I2S_DATAORDER" as input.

    DAI_I2SCONFIG_ID_CLK_DET,           /// I2S Slave mode Clock status  

	DAI_I2SCONFIG_ID_CLK_OFS,			///< Set the ASFCK (frame clk) offset, vlaid value 0x00~0xFE. default is 0x7f.
	ENUM_DUMMY4WORD(DAI_I2SCONFIG_ID)
} DAI_I2SCONFIG_ID;

/**
	DAI I2S data order select used.

	This is used in dai_setI2sConfig(DAI_I2SCONFIG_ID_DATA_ORDER)
*/

typedef enum {
	DAI_I2S_DATAORDER_TYPE1,       ///< TDM data order type 1, Ex: TDM 8ch data, [ch 0][ch 1][ch 2][ch 3][ch 4][ch 5][ch 6][ch 7]
	DAI_I2S_DATAORDER_TYPE2,       ///< TDM data order type 2, Ex: TDM 8ch data, [ch 0][ch 2][ch 4][ch 6][ch 1][ch 3][ch 5][ch 7]
								   ///< automatically set according to the (32 << DAI_I2SCONFIG_ID_CLKRATIO) / (16 << DAI_I2SCONFIG_ID_CHANNEL_LEN)

	ENUM_DUMMY4WORD(DAI_I2S_DATAORDER)
} DAI_I2S_DATAORDER;

/**
	DAI I2S data order select used.

	This is used in dai_set_i2s_order(DAI_I2S_DATAORDERPOS config_id, UINT32 config_value)
*/
typedef enum {
	DAI_I2S_DATAORDER_POS0,       ///< TDM data order POSITION 0
	DAI_I2S_DATAORDER_POS1,       ///< TDM data order POSITION 1
	DAI_I2S_DATAORDER_POS2,       ///< TDM data order POSITION 2
	DAI_I2S_DATAORDER_POS3,       ///< TDM data order POSITION 3
	DAI_I2S_DATAORDER_POS4,       ///< TDM data order POSITION 4
	DAI_I2S_DATAORDER_POS5,       ///< TDM data order POSITION 5
	DAI_I2S_DATAORDER_POS6,       ///< TDM data order POSITION 6
	DAI_I2S_DATAORDER_POS7,       ///< TDM data order POSITION 7
	DAI_I2S_DATAORDER_POS8,       ///< TDM data order POSITION 8
	DAI_I2S_DATAORDER_POS9,       ///< TDM data order POSITION 9
	DAI_I2S_DATAORDER_POS10,       ///< TDM data order POSITION 10
	DAI_I2S_DATAORDER_POS11,       ///< TDM data order POSITION 11
	DAI_I2S_DATAORDER_POS12,       ///< TDM data order POSITION 12
	DAI_I2S_DATAORDER_POS13,       ///< TDM data order POSITION 13
	DAI_I2S_DATAORDER_POS14,       ///< TDM data order POSITION 14
	DAI_I2S_DATAORDER_POS15,       ///< TDM data order POSITION 15
	DAI_I2S_DATAORDER_POS16,       ///< TDM data order POSITION 16
	DAI_I2S_DATAORDER_POS17,       ///< TDM data order POSITION 17
	DAI_I2S_DATAORDER_POS18,       ///< TDM data order POSITION 18
	DAI_I2S_DATAORDER_POS19,       ///< TDM data order POSITION 19

	ENUM_DUMMY4WORD(DAI_I2S_DATAORDERPOS)
} DAI_I2S_DATAORDERPOS;



/**
    HDMI Audio Channel Select when I2S TDM 4/6/8 CH.

    This is used in dai_set_i2s_config(DAI_I2SCONFIG_ID_HDMICH_SEL)
*/
typedef enum {
	DAI_I2SHDMI_SEL_CH01,           ///< During I2S TDM 4/6/8CH playback, channel 0/1 are transmited to HDMI audio.
	DAI_I2SHDMI_SEL_CH23,           ///< During I2S TDM 4/6/8CH playback, channel 2/3 are transmited to HDMI audio.
	DAI_I2SHDMI_SEL_CH45,           ///< During I2S TDM   6/8CH playback, channel 4/5 are transmited to HDMI audio.
	DAI_I2SHDMI_SEL_CH67,           ///< During I2S TDM     CH playback, channel 6/7 are transmited to HDMI audio.
	DAI_I2SHDMI_SEL_CH89,           ///< During I2S TDM     CH playback, channel 8/9 are transmited to HDMI audio.
	DAI_I2SHDMI_SEL_CH1011,         ///< During I2S TDM     CH playback, channel 10/11 are transmited to HDMI audio.
	DAI_I2SHDMI_SEL_CH1213,         ///< During I2S TDM     CH playback, channel 12/13 are transmited to HDMI audio.
	DAI_I2SHDMI_SEL_CH1415,         ///< During I2S TDM     CH playback, channel 14/15 are transmited to HDMI audio.
	DAI_I2SHDMI_SEL_CH1617,         ///< During I2S TDM     CH playback, channel 16/17 are transmited to HDMI audio.
    DAI_I2SHDMI_SEL_CH1819,         ///< During I2S TDM     CH playback, channel 18/19 are transmited to HDMI audio.

	ENUM_DUMMY4WORD(DAI_I2SHDMI_SEL)
} DAI_I2SHDMI_SEL;


/**
    DAI I2S clock ratio

    @note Used for dai_set_i2s_config(DAI_I2SCONFIG_ID_CLKRATIO).
*/
typedef enum {
	DAI_I2SCLKR_256FS_32BIT,            ///< SystemClk = 256 * FrameSync, FrameSync = 32 BitClk
	DAI_I2SCLKR_256FS_64BIT,            ///< SystemClk = 256 * FrameSync, FrameSync = 64 BitClk
	DAI_I2SCLKR_256FS_128BIT,           ///< SystemClk = 256 * FrameSync, FrameSync = 128 BitClk
	DAI_I2SCLKR_256FS_256BIT,           ///< SystemClk = 256 * FrameSync, FrameSync = 256 BitClk
	DAI_I2SCLKR_320FS_320BIT,           ///< SystemClk = 320 * FrameSync, FrameSync = 320 BitClk

	DAI_I2SCLKR_384FS_32BIT,            ///< SystemClk = 384 * FrameSync, FrameSync = 32 BitClk
	DAI_I2SCLKR_384FS_48BIT,            ///< SystemClk = 384 * FrameSync, FrameSync = 48 BitClk
	DAI_I2SCLKR_384FS_96BIT,            ///< SystemClk = 384 * FrameSync, FrameSync = 96 BitClk

	DAI_I2SCLKR_512FS_32BIT,            ///< SystemClk = 512 * FrameSync, FrameSync = 32 BitClk
	DAI_I2SCLKR_512FS_64BIT,            ///< SystemClk = 512 * FrameSync, FrameSync = 64 BitClk
	DAI_I2SCLKR_512FS_128BIT,           ///< SystemClk = 512 * FrameSync, FrameSync = 128 BitClk

	DAI_I2SCLKR_768FS_32BIT,            ///< SystemClk = 768 * FrameSync, FrameSync = 32 BitClk
	DAI_I2SCLKR_768FS_48BIT,            ///< SystemClk = 768 * FrameSync, FrameSync = 48 BitClk
	DAI_I2SCLKR_768FS_64BIT,            ///< SystemClk = 768 * FrameSync, FrameSync = 64 BitClk
	DAI_I2SCLKR_768FS_192BIT,           ///< SystemClk = 768 * FrameSync, FrameSync = 192 BitClk

	DAI_I2SCLKR_1024FS_32BIT,           ///< SystemClk = 1024 * FrameSync, FrameSync = 32 BitClk
	DAI_I2SCLKR_1024FS_64BIT,           ///< SystemClk = 1024 * FrameSync, FrameSync = 64 BitClk
	DAI_I2SCLKR_1024FS_128BIT,          ///< SystemClk = 1024 * FrameSync, FrameSync = 128 BitClk
	DAI_I2SCLKR_1024FS_256BIT,          ///< SystemClk = 1024 * FrameSync, FrameSync = 256 BitClk

	ENUM_DUMMY4WORD(DAI_I2SCLKR)
} DAI_I2SCLKR;

/**
    DAI I2S format

    @note Use for dai_set_i2s_config(DAI_I2SCONFIG_ID_FORMAT)
*/
typedef enum {
	DAI_I2SFMT_STANDARD,                ///< I2S Standard
	DAI_I2SFMT_DSP,                     ///< I2S DSP mode
	DAI_I2SFMT_LIKE_MSB,                ///< I2S Like, MSB justified
	DAI_I2SFMT_LIKE_LSB,                ///< I2S Like, LSB justified

	ENUM_DUMMY4WORD(DAI_I2SFMT)
} DAI_I2SFMT;

/**
    DAI I2S OP mode

    @note Used for dai_set_i2s_config(DAI_I2SCONFIG_ID_OPMODE)
*/
typedef enum {
	DAI_OP_SLAVE,                       ///< Slave mode
	DAI_OP_MASTER,                      ///< Master mode

	ENUM_DUMMY4WORD(DAI_OPMODE)
} DAI_OPMODE;

/**
    DAI I2S Channel Length

    @note Use for dai_set_i2s_config(DAI_I2SCONFIG_ID_CHANNEL_LEN)
*/
typedef enum {
	DAI_I2SCHLEN_16BITS,                ///< I2S interface is 16 bit-clocks per audio channel
	DAI_I2SCHLEN_32BITS,                ///< I2S interface is 32 bit-clocks per audio channel

	ENUM_DUMMY4WORD(DAI_I2SCHLEN)
} DAI_I2SCHLEN;

/**
    DAI AVSYNC Source Select

    @note Use for dai_set_config(DAI_CONFIG_ID_AVSYNC_SRC)
*/
typedef enum {
	DAI_AVSYNCSRC_SIE,                  ///< AVSYNC Source from SIE.
	DAI_AVSYNCSRC_SIE2,                 ///< AVSYNC Source from SIE2.
	DAI_AVSYNCSRC_SIE3,                 ///< AVSYNC Source from SIE3.
	DAI_AVSYNCSRC_SIE4,                 ///< AVSYNC Source from SIE4.
	DAI_AVSYNCSRC_SIE5,                 ///< AVSYNC Source from SIE5.
	DAI_AVSYNCSRC_SIE6,                 ///< AVSYNC Source from SIE6.
	DAI_AVSYNCSRC_SIE7,                 ///< AVSYNC Source from SIE7.
	DAI_AVSYNCSRC_SIE8,                 ///< AVSYNC Source from SIE8.

	ENUM_DUMMY4WORD(DAI_AVSYNCSRC)
} DAI_AVSYNCSRC;






//
// Export APIs in digital Audio interface
//

/**
    Open digital audio controller driver.

    Open digital auiod controller.

    @param[in] pIsrHdl      Callback function registered for interrupt notification.

    @return void
*/
extern void     dai_open(int id, DRV_CB isr_hdl);
extern void     dai2_open(int id, DRV_CB isr_hdl);
extern void     dai3_open(int id, DRV_CB isr_hdl);
extern void     dai4_open(int id, DRV_CB isr_hdl);
extern void     dai5_open(int id, DRV_CB isr_hdl);



/**
    Close digital audio controller driver.

    Close digital auiod controller.

    @return void
*/
extern void     dai_close(int id);
extern void     dai2_close(int id);
extern void     dai3_close(int id);
extern void     dai4_close(int id);
extern void     dai5_close(int id);

/**
    Set Digital Audio interface(DAI) General Configurations.

    Set Digital Audio interface(DAI) general configuration.
    Use DAI_CONFIG_ID as configuration selection and config_value is the configuration parameter.

    @param[in] config_id     Configuration selection. Please refer to DAI_CONFIG_ID for details.
    @param[in] config_value   configuration parameter. Please refer to DAI_CONFIG_ID for details.

    @return void
*/
extern void     dai_set_config(int id, DAI_CONFIG_ID config_id,  uintptr_t config_value);
extern void     dai2_set_config(int id, DAI_CONFIG_ID config_id, uintptr_t config_value);
extern void     dai3_set_config(int id, DAI_CONFIG_ID config_id, uintptr_t config_value);
extern void     dai4_set_config(int id, DAI_CONFIG_ID config_id, uintptr_t config_value);
extern void     dai5_set_config(int id, DAI_CONFIG_ID config_id, uintptr_t config_value);



/**
    Get Digital Audio interface(DAI) General Configurations.

    Get Digital Audio interface(DAI) general configuration.
    Use DAI_CONFIG_ID as configuration selection and config_value is the configuration parameter.

    @param[in] config_id     Configuration selection. Please refer to DAI_CONFIG_ID for details.

    @return void
*/
extern UINT32   dai_get_config(int id, DAI_CONFIG_ID config_id);
extern UINT32   dai2_get_config(int id, DAI_CONFIG_ID config_id);
extern UINT32   dai3_get_config(int id, DAI_CONFIG_ID config_id);
extern UINT32   dai4_get_config(int id, DAI_CONFIG_ID config_id);
extern UINT32   dai5_get_config(int id, DAI_CONFIG_ID config_id);

/**
    Set Digital Audio I2S interface TDM DATA ORDER

    Use DAI_I2S_DATAORDERPOS as configuration selection position and config_value is the configuration parameter.

    @param[in] config_id     Configuration position. Please refer to DAI_I2S_DATAORDERPOS for details.
    @param[in] config_value   configuration parameter. Please refer to DAI_I2S_DATAORDERPOS for details.

    @return void
*/
extern void dai_set_i2s_order(int id,DAI_I2S_DATAORDERPOS config_id, UINT32 config_value);
extern void dai2_set_i2s_order(int id,DAI_I2S_DATAORDERPOS config_id, UINT32 config_value);
extern void dai3_set_i2s_order(int id,DAI_I2S_DATAORDERPOS config_id, UINT32 config_value);
extern void dai4_set_i2s_order(int id,DAI_I2S_DATAORDERPOS config_id, UINT32 config_value);
extern void dai5_set_i2s_order(int id,DAI_I2S_DATAORDERPOS config_id, UINT32 config_value);

/**
    Get Digital Audio I2S interface configurations

    Use DAI_I2S_DATAORDERPOS as configuration selection position.

    @param[in] config_id     Configuration selection position. Please refer to DAI_I2SCONFIG_ID for details.

    @return void
*/
extern UINT32 dai_get_i2s_order(int id,DAI_I2S_DATAORDERPOS config_id);
extern UINT32 dai2_get_i2s_order(int id,DAI_I2S_DATAORDERPOS config_id);
extern UINT32 dai3_get_i2s_order(int id,DAI_I2S_DATAORDERPOS config_id);
extern UINT32 dai4_get_i2s_order(int id,DAI_I2S_DATAORDERPOS config_id);
extern UINT32 dai5_get_i2s_order(int id,DAI_I2S_DATAORDERPOS config_id);

/**
    Set Digital Audio I2S interface configurations

    Use DAI_I2SCONFIG_ID as configuration selection and config_value is the configuration parameter.

    @param[in] config_id     Configuration selection. Please refer to DAI_I2SCONFIG_ID for details.
    @param[in] config_value   configuration parameter. Please refer to DAI_I2SCONFIG_ID for details.

    @return void
*/
extern void     dai_set_i2s_config(int id, DAI_I2SCONFIG_ID config_id, UINT32 config_value);
extern void     dai2_set_i2s_config(int id, DAI_I2SCONFIG_ID config_id, UINT32 config_value);
extern void     dai3_set_i2s_config(int id, DAI_I2SCONFIG_ID config_id, UINT32 config_value);
extern void     dai4_set_i2s_config(int id, DAI_I2SCONFIG_ID config_id, UINT32 config_value);
extern void     dai5_set_i2s_config(int id, DAI_I2SCONFIG_ID config_id, UINT32 config_value);


/**
    Get Digital Audio I2S interface configurations

    Use DAI_I2SCONFIG_ID as configuration selection and config_value is the configuration parameter.

    @param[in] config_id     Configuration selection. Please refer to DAI_I2SCONFIG_ID for details.

    @return void
*/
extern UINT32   dai_get_i2s_config(int id, DAI_I2SCONFIG_ID config_id);
extern UINT32   dai2_get_i2s_config(int id, DAI_I2SCONFIG_ID config_id);
extern UINT32   dai3_get_i2s_config(int id, DAI_I2SCONFIG_ID config_id);
extern UINT32   dai4_get_i2s_config(int id, DAI_I2SCONFIG_ID config_id);
extern UINT32   dai5_get_i2s_config(int id, DAI_I2SCONFIG_ID config_id);

/**
    Set DAI Playback Chaneels Configurations.

    Set DAI Playback Chaneels(TX1 and TX2) Configurations.

    @param[in]  channel        playback Channel selection
    @param[in]  config_id    playback config ID selection
    @param[in]  config_value  configuration value

    @return void
*/
extern void     dai_set_tx_config(int id, DAI_TXCH channel, DAI_TXCFG_ID config_id, UINT32 config_value);
extern void     dai2_set_tx_config(int id, DAI_TXCH channel, DAI_TXCFG_ID config_id, UINT32 config_value);
extern void     dai3_set_tx_config(int id, DAI_TXCH channel, DAI_TXCFG_ID config_id, UINT32 config_value);
extern void     dai4_set_tx_config(int id, DAI_TXCH channel, DAI_TXCFG_ID config_id, UINT32 config_value);
extern void     dai5_set_tx_config(int id, DAI_TXCH channel, DAI_TXCFG_ID config_id, UINT32 config_value);


/**
    Get DAI Playback Chaneels Configurations.

    Get DAI Playback Chaneels(TX1 and TX2) Configurations.

    @param[in]  channel        playback Channel selection
    @param[in]  config_id    playback config ID selection

    @return configuration value
*/
extern UINT32   dai_get_tx_config(int id, DAI_TXCH channel, DAI_TXCFG_ID config_id);
extern UINT32   dai2_get_tx_config(int id, DAI_TXCH channel, DAI_TXCFG_ID config_id);
extern UINT32   dai3_get_tx_config(int id, DAI_TXCH channel, DAI_TXCFG_ID config_id);
extern UINT32   dai4_get_tx_config(int id, DAI_TXCH channel, DAI_TXCFG_ID config_id);
extern UINT32   dai5_get_tx_config(int id, DAI_TXCH channel, DAI_TXCFG_ID config_id);


/**
    Set DAI Playback Loopback Chaneel Configurations.

    Set DAI Playback Loopback Chaneels(TXLB) Configurations.

    @param[in]  channel        playback Loopback Channel selection
    @param[in]  config_id    playback Loopback config ID selection
    @param[in]  config_value  configuration value

    @return void
*/
extern void     dai_set_txlb_config(int id, DAI_TXLBCFG_ID config_id, UINT32 config_value);
extern void     dai2_set_txlb_config(int id, DAI_TXLBCFG_ID config_id, UINT32 config_value);
extern void     dai3_set_txlb_config(int id, DAI_TXLBCFG_ID config_id, UINT32 config_value);
extern void     dai4_set_txlb_config(int id, DAI_TXLBCFG_ID config_id, UINT32 config_value);
extern void     dai5_set_txlb_config(int id, DAI_TXLBCFG_ID config_id, UINT32 config_value);

/**
    Set DAI Playback Loopback Chaneel Configurations.

    Set DAI Playback Loopback Chaneels(TXLB) Configurations.

    @param[in]  channel        playback Loopback Channel selection
    @param[in]  config_id    playback Loopback config ID selection
    @param[in]  config_value  configuration value

    @return void
*/
extern UINT32   dai_get_txlb_config(int id, DAI_TXLBCFG_ID config_id);
extern UINT32   dai2_get_txlb_config(int id, DAI_TXLBCFG_ID config_id);
extern UINT32   dai3_get_txlb_config(int id, DAI_TXLBCFG_ID config_id);
extern UINT32   dai4_get_txlb_config(int id, DAI_TXLBCFG_ID config_id);
extern UINT32   dai5_get_txlb_config(int id, DAI_TXLBCFG_ID config_id);

/**
    Set DAI Record Channels Configurations.

    Set DAI Record Channels (RX1 & RX2) Configurations.

    @param[in]  config_id    record config ID selection
    @param[in]  config_value  configuration value

    @return void
*/
extern void     dai_set_rx_config(int id, DAI_RXCFG_ID config_id, UINT32 config_value);
extern void     dai2_set_rx_config(int id, DAI_RXCFG_ID config_id, UINT32 config_value);
extern void     dai3_set_rx_config(int id, DAI_RXCFG_ID config_id, UINT32 config_value);
extern void     dai4_set_rx_config(int id, DAI_RXCFG_ID config_id, UINT32 config_value);
extern void     dai5_set_rx_config(int id, DAI_RXCFG_ID config_id, UINT32 config_value);

/**
    Get DAI Record Channels Configurations.

    Get DAI Record Channels (RX1 & RX2) Configurations.

    @param[in]  config_id    record config ID selection

    @return configuration value
*/
extern UINT32   dai_get_rx_config(int id, DAI_RXCFG_ID config_id);
extern UINT32   dai2_get_rx_config(int id, DAI_RXCFG_ID config_id);
extern UINT32   dai3_get_rx_config(int id, DAI_RXCFG_ID config_id);
extern UINT32   dai4_get_rx_config(int id, DAI_RXCFG_ID config_id);
extern UINT32   dai5_get_rx_config(int id, DAI_RXCFG_ID config_id);

/**
    Enable/Disable DAI playback DMA Channel

    Enable/Disable DAI playback DMA Channel

    @param[in]  channel        playback Channel selection
    @param[in]  en         TRUE is Enable. FALSE is Disable.

    @return void
*/
extern void     dai_enable_tx_dma(int id, DAI_TXCH channel, BOOL en);
extern void     dai2_enable_tx_dma(int id, DAI_TXCH channel, BOOL en);
extern void     dai3_enable_tx_dma(int id, DAI_TXCH channel, BOOL en);
extern void     dai4_enable_tx_dma(int id, DAI_TXCH channel, BOOL en);
extern void     dai5_enable_tx_dma(int id, DAI_TXCH channel, BOOL en);

/**
    Enable/Disable DAI record DMA Channel

    Enable/Disable DAI record DMA Channel

    @param[in]  en         TRUE is Enable. FALSE is Disable.

    @return void
*/
extern void     dai_enable_rx_dma(int id, BOOL en);
extern void     dai2_enable_rx_dma(int id, BOOL en);
extern void     dai3_enable_rx_dma(int id, BOOL en);
extern void     dai4_enable_rx_dma(int id, BOOL en);
extern void     dai5_enable_rx_dma(int id, BOOL en);

/**
    Enable/Disable DAI playback loopback DMA Channel

    Enable/Disable DAI playback loopback DMA Channel

    @param[in]  en         TRUE is Enable. FALSE is Disable.

    @return void
*/
extern void     dai_enable_txlb_dma(int id, BOOL en);
extern void     dai2_enable_txlb_dma(int id, BOOL en);
extern void     dai3_enable_txlb_dma(int id, BOOL en);
extern void     dai4_enable_txlb_dma(int id, BOOL en);
extern void     dai5_enable_txlb_dma(int id, BOOL en);

/**
    Start/Stop DAI Playback

    Start/Stop DAI Specified Channel Playback

    @param[in]  channel        playback Channel selection
    @param[in]  en         TRUE is Start. FALSE is Stop.

    @return void
*/
extern void     dai_enable_tx(int id, DAI_TXCH channel, BOOL en);
extern void     dai2_enable_tx(int id, DAI_TXCH channel, BOOL en);
extern void     dai3_enable_tx(int id, DAI_TXCH channel, BOOL en);
extern void     dai4_enable_tx(int id, DAI_TXCH channel, BOOL en);
extern void     dai5_enable_tx(int id, DAI_TXCH channel, BOOL en);

/**
    Start/Stop DAI Record

    Start/Stop DAI Specified Channel Record

    @param[in]  en         TRUE is Start. FALSE is Stop.

    @return void
*/
extern void     dai_enable_rx(int id, BOOL en);
extern void     dai2_enable_rx(int id, BOOL en);
extern void     dai3_enable_rx(int id, BOOL en);
extern void     dai4_enable_rx(int id, BOOL en);
extern void     dai5_enable_rx(int id, BOOL en);

/**
    Start/Stop DAI playback loopback

    Start/Stop DAI Specified Channel Record

    @param[in]  en         TRUE is Start. FALSE is Stop.

    @return void
*/
extern void     dai_enable_txlb(int id, BOOL en);
extern void     dai2_enable_txlb(int id, BOOL en);
extern void     dai3_enable_txlb(int id, BOOL en);
extern void     dai4_enable_txlb(int id, BOOL en);
extern void     dai5_enable_txlb(int id, BOOL en);

/**
    Set DAI module enable/disable

    Set DAI module enable/disable

    @param[in] en      DAI module enable/disable
     - @b TRUE:     module enable
     - @b FALSE:    module disable

    @return void
*/
extern void     dai_enable_dai(int id, BOOL en);
extern void     dai2_enable_dai2(int id, BOOL en);
extern void     dai3_enable_dai3(int id, BOOL en);
extern void     dai4_enable_dai4(int id, BOOL en);
extern void     dai5_enable_dai5(int id, BOOL en);

/**
    Check whether DAI is enabled or not

    If DAI is enabled this function will return TRUE.

    @return
	- @b TRUE:  DAI is enabled
	- @b FALSE: DAI is disabled
*/
extern BOOL     dai_is_dai_enable(int id);
extern BOOL     dai2_is_dai2_enable(int id);
extern BOOL     dai3_is_dai3_enable(int id);
extern BOOL     dai4_is_dai4_enable(int id);
extern BOOL     dai5_is_dai5_enable(int id);

/**
    Check if DAI Playback Channel is enabled

    Check if DAI Playback Channel is enabled

    @param[in]  channel        playback Channel selection

    @return
	- @b TRUE:  Specified Playback channel is enabled
	- @b FALSE: Specified Playback channel is disabled
*/
extern BOOL     dai_is_tx_enable(int id, DAI_TXCH channel);
extern BOOL     dai2_is_tx_enable(int id, DAI_TXCH channel);
extern BOOL     dai3_is_tx_enable(int id, DAI_TXCH channel);
extern BOOL     dai4_is_tx_enable(int id, DAI_TXCH channel);
extern BOOL     dai5_is_tx_enable(int id, DAI_TXCH channel);

/**
    Check if DAI record Channel is enabled

    Check if DAI record Channel is enabled

    @return
	- @b TRUE:  Record is enabled
	- @b FALSE: Record is disabled
*/
extern BOOL     dai_is_rx_enable(int id);
extern BOOL     dai2_is_rx_enable(int id);
extern BOOL     dai3_is_rx_enable(int id);
extern BOOL     dai4_is_rx_enable(int id);
extern BOOL     dai5_is_rx_enable(int id);

/**
    Check if DAI playback loopback Channel is enabled

    Check if DAI playback loopback Channel is enabled

    @return
	- @b TRUE:  playback loopback is enabled
	- @b FALSE: playback loopback is disabled
*/
extern BOOL     dai_is_txlb_enable(int id);
extern BOOL     dai2_is_txlb_enable(int id);
extern BOOL     dai3_is_txlb_enable(int id);
extern BOOL     dai4_is_txlb_enable(int id);
extern BOOL     dai5_is_txlb_enable(int id);

/**
    Check whether DAI is under tx/rx

    If DAI is under tx/rx, this function will return TRUE.

    @return
	- @b TRUE:  DAI is under tx/rx.
	- @b FALSE: DAI is not under tx/rx.

*/
extern BOOL     dai_is_txrx_enable(int id);
extern BOOL     dai2_is_txrx_enable(int id);
extern BOOL     dai3_is_txrx_enable(int id);
extern BOOL     dai4_is_txrx_enable(int id);
extern BOOL     dai5_is_txrx_enable(int id);

/**
    Check if playback DMA Channel is Enabled

    Check if playback DMA Channel is Enabled

    @param[in]  channel        playback Channel selection

    @return
	- @b TRUE:  Specified Playback DMA channel is enabled
	- @b FALSE: Specified Playback DMA channel is disabled
*/
extern BOOL     dai_is_tx_dma_enable(int id, DAI_TXCH channel);
extern BOOL     dai2_is_tx_dma_enable(int id, DAI_TXCH channel);
extern BOOL     dai3_is_tx_dma_enable(int id, DAI_TXCH channel);
extern BOOL     dai4_is_tx_dma_enable(int id, DAI_TXCH channel);
extern BOOL     dai5_is_tx_dma_enable(int id, DAI_TXCH channel);

/**
    Check if Record DMA Channel is Enabled

    Check if Record DMA Channel is Enabled

    @return
	- @b TRUE:  Specified Record DMA channel is enabled
	- @b FALSE: Specified Record DMA channel is disabled
*/
extern BOOL     dai_is_rx_dma_enable(int id);
extern BOOL     dai2_is_rx_dma_enable(int id);
extern BOOL     dai3_is_rx_dma_enable(int id);
extern BOOL     dai4_is_rx_dma_enable(int id);
extern BOOL     dai5_is_rx_dma_enable(int id);

/**
    Check if Playback Loopback DMA Channel is Enabled

    Check if Playback Loopback DMA Channel is Enabled

    @return
	- @b TRUE:  Specified Playback Loopback DMA channel is enabled
	- @b FALSE: Specified Playback Loopback DMA channel is disabled
*/
extern BOOL     dai_is_txlb_dma_enable(int id);
extern BOOL     dai2_is_txlb_dma_enable(int id);
extern BOOL     dai3_is_txlb_dma_enable(int id);
extern BOOL     dai4_is_txlb_dma_enable(int id);
extern BOOL     dai5_is_txlb_dma_enable(int id);

/**
    Check whether Tx or Rx DMA is enabled or not

    If Tx or Rx DMA is enabled this function will return TRUE.

    @return
	- @b TRUE:  DMA is enabled
	- @b FALSE: DMA is disabled
*/
extern BOOL     dai_is_dma_enable(int id);
extern BOOL     dai2_is_dma_enable(int id);
extern BOOL     dai3_is_dma_enable(int id);
extern BOOL     dai4_is_dma_enable(int id);
extern BOOL     dai5_is_dma_enable(int id);
/**
    Set Playback DMA parameter

    Set Playback DMA starting address, buffer size.

    @param[in] channel       Playback DMA Channel Selection. Valid value is 0 or 1 for NT96680.
    @param[in] buf_addr      DMA start address. (unit: byte, should be word-alignment)
    @param[in] buf_size      DMA buffer size (unit: 16 words aligned)

    @return void
*/
extern void     dai_set_tx_dma_para(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai2_set_tx_dma_para(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai3_set_tx_dma_para(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai4_set_tx_dma_para(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai5_set_tx_dma_para(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);

extern void     dai_ep_set_tx_dma_para(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai2_ep_set_tx_dma_para(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai3_ep_set_tx_dma_para(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai4_ep_set_tx_dma_para(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai5_ep_set_tx_dma_para(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);

/**
    Set Playback DMA parameter2

    Set Playback DMA starting address2, buffer size.2

    @param[in] channel       Playback DMA Channel Selection. Valid value is 0 or 1 for NT96680.
    @param[in] buf_addr      DMA start address. (unit: byte, should be word-alignment)
    @param[in] buf_size      DMA buffer size (unit: 16 words aligned)

    @return void
*/
extern void     dai_set_tx_dma_para2(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai2_set_tx_dma_para2(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai3_set_tx_dma_para2(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai4_set_tx_dma_para2(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai5_set_tx_dma_para2(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
/*
extern void     dai_ep_set_tx_dma_para(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai2_ep_set_tx_dma_para(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai3_ep_set_tx_dma_para(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai4_ep_set_tx_dma_para(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai5_ep_set_tx_dma_para(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
*/

/**
    Set Record DMA parameter

    Set Record DMA starting address, buffer size.

    @param[in] channel       Record DMA Channel Selection. Valid value is 0 or 1 for NT96680.
    @param[in] buf_addr      DMA start address. (unit: byte, should be word-alignment)
    @param[in] buf_size      DMA buffer size (unit: 16 words aligned)

    @return void
*/
extern void     dai_set_rx_dma_para(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai2_set_rx_dma_para(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai3_set_rx_dma_para(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai4_set_rx_dma_para(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai5_set_rx_dma_para(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);

extern void     dai_ep_set_rx_dma_para(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai2_ep_set_rx_dma_para(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai3_ep_set_rx_dma_para(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai4_ep_set_rx_dma_para(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai5_ep_set_rx_dma_para(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);

/**
    Set Record DMA parameter2

    Set Record DMA starting address2, buffer size2.

    @param[in] channel       Record DMA Channel Selection. Valid value is 0 or 1 for NT96680.
    @param[in] buf_addr      DMA start address. (unit: byte, should be word-alignment)
    @param[in] buf_size      DMA buffer size (unit: 16 words aligned)

    @return void
*/
extern void     dai_set_rx_dma_para2(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai2_set_rx_dma_para2(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai3_set_rx_dma_para2(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai4_set_rx_dma_para2(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai5_set_rx_dma_para2(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
/*
extern void     dai_ep_set_rx_dma_para(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai2_ep_set_rx_dma_para(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai3_ep_set_rx_dma_para(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai4_ep_set_rx_dma_para(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
extern void     dai5_ep_set_rx_dma_para(int id, int ddr_id,UINT32 channel, uintptr_t buf_addr, UINT32 buf_size);
*/

/**
    Set Playback Loopback DMA parameter

    Set Playback Loopback DMA starting address, buffer size.

    @param[in] buf_addr      DMA start address. (unit: byte, should be word-alignment)
    @param[in] buf_size      DMA buffer size (unit: 16 words aligned)

    @return void
*/
extern void     dai_set_txlb_dma_para(int id, int ddr_id,uintptr_t buf_addr, UINT32 buf_size);
extern void     dai2_set_txlb_dma_para(int id, int ddr_id,uintptr_t buf_addr, UINT32 buf_size);
extern void     dai3_set_txlb_dma_para(int id, int ddr_id,uintptr_t buf_addr, UINT32 buf_size);
extern void     dai4_set_txlb_dma_para(int id, int ddr_id,uintptr_t buf_addr, UINT32 buf_size);
extern void     dai5_set_txlb_dma_para(int id, int ddr_id,uintptr_t buf_addr, UINT32 buf_size);

extern void     dai_ep_set_txlb_dma_para(int id, int ddr_id,uintptr_t buf_addr, UINT32 buf_size);
extern void     dai2_ep_set_txlb_dma_para(int id, int ddr_id,uintptr_t buf_addr, UINT32 buf_size);
extern void     dai3_ep_set_txlb_dma_para(int id, int ddr_id,uintptr_t buf_addr, UINT32 buf_size);
extern void     dai4_ep_set_txlb_dma_para(int id, int ddr_id,uintptr_t buf_addr, UINT32 buf_size);
extern void     dai5_ep_set_txlb_dma_para(int id, int ddr_id,uintptr_t buf_addr, UINT32 buf_size);

/**
    Set Playback Loopback DMA parameter2

    Set Playback Loopback DMA starting address2, buffer size2.

    @param[in] buf_addr      DMA start address. (unit: byte, should be word-alignment)
    @param[in] buf_size      DMA buffer size (unit: 16 words aligned)

    @return void
*/
extern void     dai_set_txlb_dma_para2(int id, int ddr_id,uintptr_t buf_addr, UINT32 buf_size);
extern void     dai2_set_txlb_dma_para2(int id, int ddr_id,uintptr_t buf_addr, UINT32 buf_size);
extern void     dai3_set_txlb_dma_para2(int id, int ddr_id,uintptr_t buf_addr, UINT32 buf_size);
extern void     dai4_set_txlb_dma_para2(int id, int ddr_id,uintptr_t buf_addr, UINT32 buf_size);
extern void     dai5_set_txlb_dma_para2(int id, int ddr_id,uintptr_t buf_addr, UINT32 buf_size);
/*
extern void     dai_ep_set_txlb_dma_para(int id, int ddr_id,uintptr_t buf_addr, UINT32 buf_size);
extern void     dai2_ep_set_txlb_dma_para(int id, int ddr_id,uintptr_t buf_addr, UINT32 buf_size);
extern void     dai3_ep_set_txlb_dma_para(int id, int ddr_id,uintptr_t buf_addr, UINT32 buf_size);
extern void     dai4_ep_set_txlb_dma_para(int id, int ddr_id,uintptr_t buf_addr, UINT32 buf_size);
extern void     dai5_ep_set_txlb_dma_para(int id, int ddr_id,uintptr_t buf_addr, UINT32 buf_size);
*/

/**
    Get Playback DMA parameter

    Get Playback DMA starting address, buffer size.

    @param[in] channel       Playback DMA Channel Selection. Valid value is 0 or 1 for NT96680.
    @param[out] buf_addr     DMA start address. (unit: byte, should be word-alignment)
    @param[out] buf_size     DMA buffer size (unit: word)

    @return void
*/
extern void     dai_get_tx_dma_para(int id, UINT32 channel, UINT32 *p_buf_addr, UINT32 *p_buf_size);
extern void     dai2_get_tx_dma_para(int id, UINT32 channel, UINT32 *p_buf_addr, UINT32 *p_buf_size);
extern void     dai3_get_tx_dma_para(int id, UINT32 channel, UINT32 *p_buf_addr, UINT32 *p_buf_size);
extern void     dai4_get_tx_dma_para(int id, UINT32 channel, UINT32 *p_buf_addr, UINT32 *p_buf_size);
extern void     dai5_get_tx_dma_para(int id, UINT32 channel, UINT32 *p_buf_addr, UINT32 *p_buf_size);

/**
    Get Record DMA parameter

    Get Record DMA starting address, buffer size.

    @param[in] channel       Record DMA Channel Selection. Valid value is 0 or 1 for NT96680.
    @param[out] buf_addr      DMA start address. (unit: byte, should be word-alignment)
    @param[out] buf_size      DMA buffer size (unit: word)

    @return void
*/
extern void     dai_get_rx_dma_para(int id, UINT32 channel, UINT32 *p_buf_addr, UINT32 *p_buf_size);
extern void     dai2_get_rx_dma_para(int id, UINT32 channel, UINT32 *p_buf_addr, UINT32 *p_buf_size);
extern void     dai3_get_rx_dma_para(int id, UINT32 channel, UINT32 *p_buf_addr, UINT32 *p_buf_size);
extern void     dai4_get_rx_dma_para(int id, UINT32 channel, UINT32 *p_buf_addr, UINT32 *p_buf_size);
extern void     dai5_get_rx_dma_para(int id, UINT32 channel, UINT32 *p_buf_addr, UINT32 *p_buf_size);

/**
    Get Playback Loopback DMA parameter

    Get Playback Loopback DMA starting address, buffer size.

    @param[in] buf_addr      DMA start address. (unit: byte, should be word-alignment)
    @param[in] buf_size      DMA buffer size (unit: word)

    @return void
*/
extern void     dai_get_txlb_dma_para(int id, UINT32 *p_buf_addr, UINT32 *p_buf_size);
extern void     dai2_get_txlb_dma_para(int id, UINT32 *p_buf_addr, UINT32 *p_buf_size);
extern void     dai3_get_txlb_dma_para(int id, UINT32 *p_buf_addr, UINT32 *p_buf_size);
extern void     dai4_get_txlb_dma_para(int id, UINT32 *p_buf_addr, UINT32 *p_buf_size);
extern void     dai5_get_txlb_dma_para(int id, UINT32 *p_buf_addr, UINT32 *p_buf_size);

/**
    Get Playback DMA currrent address

    Get Playback DMA currrent address

    @param[in] channel   Playback DMA Channel Selection. Valid value is 0 or 1 for NT96680.

    @return Playback DMA current address
*/
extern UINT32   dai_get_tx_dma_curaddr(int id, UINT32 channel);
extern UINT32   dai2_get_tx_dma_curaddr(int id, UINT32 channel);
extern UINT32   dai3_get_tx_dma_curaddr(int id, UINT32 channel);
extern UINT32   dai4_get_tx_dma_curaddr(int id, UINT32 channel);
extern UINT32   dai5_get_tx_dma_curaddr(int id, UINT32 channel);

/**
    Get Record DMA currrent address

    Get Record DMA currrent address

    @param[in] channel   Record DMA Channel Selection. Valid value is 0 or 1 for NT96680.

    @return Record DMA current address
*/
extern UINT32   dai_get_rx_dma_curaddr(int id, UINT32 channel);
extern UINT32   dai2_get_rx_dma_curaddr(int id, UINT32 channel);
extern UINT32   dai3_get_rx_dma_curaddr(int id, UINT32 channel);
extern UINT32   dai4_get_rx_dma_curaddr(int id, UINT32 channel);
extern UINT32   dai5_get_rx_dma_curaddr(int id, UINT32 channel);

/**
    Get Playback Loopback DMA currrent address

    Get Playback Loopback DMA currrent address

    @return Playback Loopback DMA current address
*/
extern UINT32   dai_get_txlb_dma_curaddr(int id);
extern UINT32   dai2_get_txlb_dma_curaddr(int id);
extern UINT32   dai3_get_txlb_dma_curaddr(int id);
extern UINT32   dai4_get_txlb_dma_curaddr(int id);
extern UINT32   dai5_get_txlb_dma_curaddr(int id);

/**
    Clear DAI flag

    Clear DAI flag

    @param[in] int_flag  interrupt flags to clear.

    @return void
*/
extern void     dai_clr_flg(int id, DAI_INTERRUPT int_flag);
extern void     dai2_clr_flg(int id, DAI_INTERRUPT int_flag);
extern void     dai3_clr_flg(int id, DAI_INTERRUPT int_flag);
extern void     dai4_clr_flg(int id, DAI_INTERRUPT int_flag);
extern void     dai5_clr_flg(int id, DAI_INTERRUPT int_flag);



extern void		dai_set_rx_muti_ch_dma_para(int id, int ddr_id,UINT32 dma_channel,uintptr_t dma_start_addr,UINT32 dma_buffer_size);
extern void		dai2_set_rx_muti_ch_dma_para(int id, int ddr_id,UINT32 dma_channel,uintptr_t dma_start_addr,UINT32 dma_buffer_size);
extern void		dai3_set_rx_muti_ch_dma_para(int id, int ddr_id,UINT32 dma_channel,uintptr_t dma_start_addr,UINT32 dma_buffer_size);
extern void		dai4_set_rx_muti_ch_dma_para(int id, int ddr_id,UINT32 dma_channel,uintptr_t dma_start_addr,UINT32 dma_buffer_size);
extern void		dai5_set_rx_muti_ch_dma_para(int id, int ddr_id,UINT32 dma_channel,uintptr_t dma_start_addr,UINT32 dma_buffer_size);

extern void		dai_ep_set_rx_muti_ch_dma_para(int id, int ddr_id,UINT32 dma_channel,uintptr_t dma_start_addr,UINT32 dma_buffer_size);
extern void		dai2_ep_set_rx_muti_ch_dma_para(int id, int ddr_id,UINT32 dma_channel,uintptr_t dma_start_addr,UINT32 dma_buffer_size);
extern void		dai3_ep_set_rx_muti_ch_dma_para(int id, int ddr_id,UINT32 dma_channel,uintptr_t dma_start_addr,UINT32 dma_buffer_size);
extern void		dai4_ep_set_rx_muti_ch_dma_para(int id, int ddr_id,UINT32 dma_channel,uintptr_t dma_start_addr,UINT32 dma_buffer_size);
extern void		dai5_ep_set_rx_muti_ch_dma_para(int id, int ddr_id,UINT32 dma_channel,uintptr_t dma_start_addr,UINT32 dma_buffer_size);

extern void		dai_set_rx_muti_ch_dma_para2(int id, int ddr_id,UINT32 dma_channel,uintptr_t dma_start_addr,UINT32 dma_buffer_size);
extern void		dai2_set_rx_muti_ch_dma_para2(int id, int ddr_id,UINT32 dma_channel,uintptr_t dma_start_addr,UINT32 dma_buffer_size);

/*
    dai clk releated API
*/
extern void     dai_enableclk(int id);
extern void     dai2_enableclk(int id);
extern void     dai3_enableclk(int id);
extern void     dai4_enableclk(int id);
extern void     dai5_enableclk(int id);

extern void     dai_disableclk(int id);
extern void     dai2_disableclk(int id);
extern void     dai3_disableclk(int id);
extern void     dai4_disableclk(int id);
extern void     dai5_disableclk(int id);

extern void     dai_setclkrate(int id, unsigned long clkrate);
extern void     dai2_setclkrate(int id, unsigned long clkrate);
extern void     dai3_setclkrate(int id, unsigned long clkrate);
extern void     dai4_setclkrate(int id, unsigned long clkrate);
extern void     dai5_setclkrate(int id, unsigned long clkrate);


extern ER dai_lock(int id);
extern ER dai2_lock(int id);
extern ER dai3_lock(int id);
extern ER dai4_lock(int id);
extern ER dai5_lock(int id);

extern ER dai_unlock(int id);
extern ER dai2_unlock(int id);
extern ER dai3_unlock(int id);
extern ER dai4_unlock(int id);
extern ER dai5_unlock(int id);


extern DAI_INTERRUPT dai_wait_interrupt(int id, DAI_INTERRUPT waited_flag);
extern DAI_INTERRUPT dai2_wait_interrupt(int id, DAI_INTERRUPT waited_flag);
extern DAI_INTERRUPT dai3_wait_interrupt(int id, DAI_INTERRUPT waited_flag);
extern DAI_INTERRUPT dai4_wait_interrupt(int id, DAI_INTERRUPT waited_flag);
extern DAI_INTERRUPT dai5_wait_interrupt(int id, DAI_INTERRUPT waited_flag);

extern void dai_select_mclk_pinmux(int id, BOOL b_en);
extern void dai_select_pinmux(int id, BOOL b_en);
extern void dai_debug(BOOL b_en);
extern void dai_debug_eac(BOOL b_en, BOOL b_mode_ad);
extern BOOL dai_get_debug_status(void);
extern void dai_clr_debug_status(void);
extern void dai_set_debug_dma_para(uintptr_t dma_start_addr, uintptr_t dma_buffer_size);

extern void dai2_select_mclk_pinmux(int id, BOOL b_en);
extern void dai2_select_pinmux(int id, BOOL b_en);
extern void dai2_debug(BOOL b_en);
extern void dai2_debug_eac(BOOL b_en, BOOL b_mode_ad);
extern BOOL dai2_get_debug_status(void);
extern void dai2_clr_debug_status(void);
extern void dai2_set_debug_dma_para(uintptr_t dma_start_addr, uintptr_t dma_buffer_size);

extern void dai3_select_mclk_pinmux(int id, BOOL b_en);
extern void dai3_select_pinmux(int id, BOOL b_en);
extern void dai3_debug(BOOL b_en);
extern void dai3_debug_eac(BOOL b_en, BOOL b_mode_ad);
extern BOOL dai3_get_debug_status(void);
extern void dai3_clr_debug_status(void);
extern void dai3_set_debug_dma_para(uintptr_t dma_start_addr, uintptr_t dma_buffer_size);

extern void dai4_select_mclk_pinmux(int id, BOOL b_en);
extern void dai4_select_pinmux(int id, BOOL b_en);
extern void dai4_debug(BOOL b_en);
extern void dai4_debug_eac(BOOL b_en, BOOL b_mode_ad);
extern BOOL dai4_get_debug_status(void);
extern void dai4_clr_debug_status(void);
extern void dai4_set_debug_dma_para(uintptr_t dma_start_addr, uintptr_t dma_buffer_size);

extern void dai5_select_mclk_pinmux(int id, BOOL b_en);
extern void dai5_select_pinmux(int id, BOOL b_en);
extern void dai5_debug(BOOL b_en);
extern void dai5_debug_eac(BOOL b_en, BOOL b_mode_ad);
extern BOOL dai5_get_debug_status(void);
extern void dai5_clr_debug_status(void);
extern void dai5_set_debug_dma_para(uintptr_t dma_start_addr, uintptr_t dma_buffer_size);

#if defined(__FREERTOS)
#else
extern void     dai_isr(int id);
extern void     dai2_isr(int id);
extern void     dai3_isr(int id);
extern void     dai4_isr(int id);
extern void     dai5_isr(int id);

#endif
//@}
#endif
