/**
	@brief Header file of vendor videocapture module.\n
	This file contains the functions which is related to vendor videocapture.

	@file vendor_videocapture.h

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#ifndef _VENDOR_VIDEOCAP_H_
#define _VENDOR_VIDEOCAP_H_

#ifdef __cplusplus
extern "C" {
#endif


/********************************************************************
	INCLUDE FILES
********************************************************************/
#include "hd_type.h"
#include "hd_common.h"
#include "hd_videocapture.h"
/********************************************************************
	MACRO CONSTANT DEFINITIONS
********************************************************************/
#define HD_VIDEO_PIX_Y             0x0500

#define VENDOR_VCAP_TSEN_CFG_NUM   20
/********************************************************************
	MACRO FUNCTION DEFINITIONS
********************************************************************/
// will be merged to hd_videocapture.h in the next HDAL version
#define HD_VIDEOCAP_SEN_EXT_CLK_LANE_2   8  ///< second MIPI for combo device

#define HD_COMMON_VIDEO_IN_TDIO (HD_COMMON_VIDEO_IN_SPI+1)
/********************************************************************
	TYPE DEFINITION
********************************************************************/
typedef struct _VENDOR_VIDEOCAP_AE_PRESET {
	BOOL  enable;
	UINT32 exp_time;
	UINT32 gain_ratio;
} VENDOR_VIDEOCAP_AE_PRESET;

typedef struct _VENDOR_VIDEOCAP_FAST_OPEN_SENSOR {
	HD_VIDEOCAP_SEN_CONFIG  sen_cfg;
	UINT32 sen_mode;                         ///< referring to sensor driver, or set HD_VIDEOCAP_SEN_MODE_AUTO for AUTO selecting
	HD_VIDEO_FRC frc;                        ///< frame rate
	HD_DIM dim;                              ///< dim w,h. only valid when sen_mode is HD_VIDEOCAP_SEN_MODE_AUTO
	HD_VIDEO_PXLFMT pxlfmt;                  ///< pixel format. only valid when sen_mode is HD_VIDEOCAP_SEN_MODE_AUTO
	HD_VIDEOCAP_SEN_FRAME_NUM out_frame_num; ///< sensor output frame number, 1 for linear mode and 2/3/4 for sensor HDR mode.
	UINT32 data_lane;                        ///< set data lane number for selecting sensor mode
	BOOL builtin_hdr;                        ///< set TRUE for enabling built-in HDR mode
	VENDOR_VIDEOCAP_AE_PRESET ae_preset;     ///< ae preset
	BOOL wait_det_ms;                        ///< plugin detection delay interval. ad only
} VENDOR_VIDEOCAP_FAST_OPEN_SENSOR;

#define AD_CHIP(id)    ((((UINT32)(id)) & 0x000000ff)<<24)
#define AD_IN(ch)    ((((UINT32)(ch)) & 0x000000ff)<<16)
#define VENDOR_VIDEOCAP_AD_MAP(ad_chip, ad_in, vdocap_id) (AD_CHIP(ad_chip)|AD_IN(ad_in)|(vdocap_id))

#define VENDOR_VIDEOCAP_ENCODE_RATE(aggressive, enc_rate) ((aggressive << 16)|(enc_rate))

typedef enum {
	VENDOR_VIDEOCAP_FIELD_DISABLE = 0,
	VENDOR_VIDEOCAP_FIELD_EN_0,
	VENDOR_VIDEOCAP_FIELD_EN_1,
	ENUM_DUMMY4WORD(VENDOR_VIDEOCAP_FIELD_SEL)
} VENDOR_VIDEOCAP_FIELD_SEL;

typedef enum {
	VENDOR_VIDEOCAP_CCIR_FMT_SEL_CCIR601 = 0,
	VENDOR_VIDEOCAP_CCIR_FMT_SEL_CCIR656,
	VENDOR_VIDEOCAP_CCIR_FMT_SEL_CCIR709,
	VENDOR_VIDEOCAP_CCIR_FMT_SEL_CCIR1120,
	VENDOR_VIDEOCAP_CCIR_FMT_SEL_MAX_NUM,
	ENUM_DUMMY4WORD(VENDOR_VIDEOCAP_CCIR_FMT_SEL)
} VENDOR_VIDEOCAP_CCIR_FMT_SEL;

typedef struct _VENDOR_VIDEOCAP_CCIR_INFO {
	VENDOR_VIDEOCAP_FIELD_SEL field_sel;
	VENDOR_VIDEOCAP_CCIR_FMT_SEL fmt;
	BOOL interlace;                         ///< enable interlace or not
	UINT32 mux_data_index;                  ///< mux data index
} VENDOR_VIDEOCAP_CCIR_INFO;

typedef struct {
	BOOL  enable;
	UINT8 positive;
	UINT8 negative;
	UINT8 nochange;
} VENDOR_VIDEOCAP_DVS_INFO;

typedef struct {
	UINT32 max_out_pixel_cnt;	// evs out event data common buf = max_out_pixel_cnt * 2(byte)
	UINT16 padding_val;			// padding value when output size not 4 byte align, 538 not supported, 539A supported
} VENDOR_VIDEOCAP_EVS_INFO;

typedef struct {
	HD_DIM size;        ///< [out], current plug size
	UINT32 fps;         ///< [out], current plug fps
	BOOL   interlace;   ///< [out] TRUE:CTL_SEN_MODE_CCIR_INTERLACE, FALSE:CTL_SEN_MODE_CCIR
	UINT32 param[5];    ///< [out], current plug info
} VENDOR_VIDEOCAP_GET_PLUG_INFO;

typedef enum {
	VENDOR_VIDEOCAP_QUEUE_FLUSH_ALL = 0,   ///< default setting
	VENDOR_VIDEOCAP_QUEUE_FLUSH_OLDEST,
	ENUM_DUMMY4WORD(VENDOR_VIDEOCAP_QUEUE_FLUSH_SCHEME)
} VENDOR_VIDEOCAP_QUEUE_FLUSH_SCHEME;

typedef enum {
	VENDOR_VCAP_SEN_MODE_TYPE_UNKNOWN = 0,
	VENDOR_VCAP_SEN_MODE_LINEAR,               ///< 1 frame, linear raw
	VENDOR_VCAP_SEN_MODE_BUILTIN_HDR,          ///< 1 frame, bulid in HDR
	VENDOR_VCAP_SEN_MODE_CCIR,                 ///< 1 frame, ccir progressive
	VENDOR_VCAP_SEN_MODE_CCIR_INTERLACE,       ///< 1 frame, ccir interlace
	VENDOR_VCAP_SEN_MODE_RAW_PDAF,             ///< 1 frame, get pdaf from raw plane 3
	VENDOR_VCAP_SEN_MODE_BUILTIN_DCG_HDR,      ///< 1 frame, pwl & dcg strategy
	VENDOR_VCAP_SEN_MODE_STAGGER_HDR,          ///< 2 frame, stagger HDR
	VENDOR_VCAP_SEN_MODE_PDAF,                 ///< 2 frame, linear raw + pdaf
	VENDOR_VCAP_SEN_MODE_BUILTIN_DCG_SHDR,     ///< 2 frame, path 1: pwl, path 2: linear
	VENDOR_VCAP_SEN_MODE_STAGGER_PDAF,         ///< 2 frame, get pdaf from raw plane 3
	VENDOR_VCAP_SEN_MODE_DCG_HDR,              ///< 2 frame, dcg strategy
	VENDOR_VCAP_SEN_MODE_DCG_SHDR,             ///< 3 frame, path 1/2 dcg strategy
	VENDOR_VCAP_SEN_MODE_STAGGER3_HDR,         ///< 3 frame
	ENUM_DUMMY4WORD(VENDOR_VCAP_SEN_MODE_TYPE)
} VENDOR_VCAP_SEN_MODE_TYPE;

typedef enum {
	VENDOR_VCAP_PDAF_TYPE_UNKNOWN = 0,
	VENDOR_VCAP_PDAF_TYPE_INTERLEAVE,         ///< 1-framed PDAF mode
	VENDOR_VCAP_PDAF_TYPE_EMBEDDED,           ///< 1-framed PDAF mode
	VENDOR_VCAP_PDAF_TYPE_DATA_TYPE,          ///< 2-framed PDAF mode
	VENDOR_VCAP_PDAF_TYPE_VIRTUAL_CHANNEL,    ///< 2-framed PDAF mode
	ENUM_DUMMY4WORD(VENDOR_VCAP_PDAF_TYPE)
} VENDOR_VCAP_PDAF_TYPE;

typedef struct {
	BOOL   en;          ///< enable gyro function
	UINT32 data_num;    ///< data number per axis
} VENDOR_VIDEOCAP_GYRO_INFO;

typedef enum {
	VENDOR_VIDEOCAP_TGE_RESET = 0,               ///< reset TGE signal
	ENUM_DUMMY4WORD(VENDOR_VIDEOCAP_TGE_CTRL)
} VENDOR_VIDEOCAP_TGE_CTRL;


typedef enum {
    VENDOR_VCAP_OOC_PACKBUS_6 = 0,     ///< 6 bits packing
    VENDOR_VCAP_OOC_PACKBUS_7,         ///< 7 bits packing
    VENDOR_VCAP_OOC_PACKBUS_8,         ///< 8 bits packing
    VENDOR_VCAP_OOC_PACKBUS_MAX,
    ENUM_DUMMY4WORD(VENDOR_VCAP_OOC_PACKBUS_SEL)
} VENDOR_VCAP_OOC_PACKBUS_SEL;

typedef struct {
	UINT32 len;                              ///< from 4 ~ 512 clock cycle
	UINT32 tx_data[VENDOR_VCAP_TSEN_CFG_NUM];///< config data
	UINT32 id;                               ///< config id for next RAW frame
} VENDOR_VCAP_TSEN_CFG;

typedef struct {
	ULONG  buf_pa;                            ///< ooc buffer address
	VENDOR_VCAP_OOC_PACKBUS_SEL pack_bus_sel; ///< referring to VENDOR_VCAP_OOC_PACKBUS_SEL
	UINT32 lofs;                              ///< lineoffset
	UINT32 id;                                ///< OOC id for next RAW frame
} VENDOR_VCAP_TSEN_OOC;

typedef enum _VENDOR_VIDEOCAP_PARAM_ID {
	VENDOR_VIDEOCAP_PARAM_FAST_OPEN_SENSOR,      ///< using device id, refer to VENDOR_VIDEOCAP_FAST_OPEN_SENSOR struct
	VENDOR_VIDEOCAP_PARAM_FAST_CHANGE_MODE,      ///< using device id, refer to VENDOR_VIDEOCAP_FAST_OPEN_SENSOR struct
	VENDOR_VIDEOCAP_PARAM_FAST_START,            ///< using bitwise id to START VCAP for 2A in fast boot mode
	VENDOR_VIDEOCAP_PARAM_MCLK_SRC_SYNC_SET,     ///< for ctrl path, indicate which MCLKs of VCAP use the same clock source, bitwise setting for VIDEOCAPx, e.g. set 0x3 to sync VIDEOCAP0 and VIDEOCAP1
	VENDOR_VIDEOCAP_PARAM_BUILTIN_HDR,           ///< for i/o path, set TRUE for enabling built-in HDR mode
	VENDOR_VIDEOCAP_PARAM_DATA_LANE,             ///< for i/o path, set data lane number for selecting sensor mode
	VENDOR_VIDEOCAP_PARAM_CCIR_INFO,             ///< for i/o path, referring to VENDOR_VIDEOCAP_CCIR_INFO
	VENDOR_VIDEOCAP_PARAM_GET_PLUG,              ///< for ctrl path, get sensor plugged or unplugged
	VENDOR_VIDEOCAP_PARAM_GET_PLUG_INFO,         ///< for ctrl path, get sensor plugged info, referring to VENDOR_VIDEOCAP_GET_PLUG_INFO
	VENDOR_VIDEOCAP_PARAM_SET_FPS,               ///< for ctrl path, set sensor frame rate in running time and the fps should NOT greater HD_VIDEOCAP_IN.frc
	VENDOR_VIDEOCAP_PARAM_AD_MAP,                ///< for ctrl path, set Analog Decoder mapping, e.g. VENDOR_VIDEOCAP_AD_MAP(0, 0, HD_VIDEOCAP_0|HD_VIDEOCAP_1)
	VENDOR_VIDEOCAP_PARAM_AD_TYPE,               ///< for ctrl path, set Analog Decoder type
	VENDOR_VIDEOCAP_PARAM_PDAF_MAP,              ///< for ctrl path, set PDAF sensor mapping, e.g. (HD_VIDEOCAP_0|HD_VIDEOCAP_1)
	VENDOR_VIDEOCAP_PARAM_DVS_INFO,              ///< for i/o path, referring to VENDOR_VIDEOCAP_DVS_INFO
	VENDOR_VIDEOCAP_PARAM_ABORT,                 ///< for ctrl path, abort DMA
	VENDOR_VIDEOCAP_PARAM_SIE_MAP,               ///< for ctrl path, re-map VCAP to other SIE, e.g. 0 for SIE1, 1 for SIE2 and so on.
	VENDOR_VIDEOCAP_PARAM_ENC_RATE,              ///< for i/o path, set RAW compression ratio and only support 50, 58 and 66
	VENDOR_VIDEOCAP_PARAM_AE_PRESET,             ///< for i/o path, set AE data, referring to VENDOR_VIDEOCAP_AE_PRESET.
	VENDOR_VIDEOCAP_PARAM_BP3_RATIO,             ///< for i/o path, set SIE act_start to bp ratio(1~100)
	VENDOR_VIDEOCAP_PARAM_QUEUE_FLUSH_SCHEME,    ///< for i/o path, set the scheme for flushing queue in the condition of queue full, referring to VENDOR_VIDEOCAP_QUEUE_FLUSH_SCHEME
	VENDOR_VIDEOCAP_PARAM_CSI_ERR_CNT,           ///< for ctrl path, get CSI error count(UINT32)
	VENDOR_VIDEOCAP_PARAM_SW_VD_SYNC,            ///< for ctrl path, indicate which VCAPs use software VD sync mechanism, bitwise setting for VIDEOCAPx, e.g. set 0x3 to sync VIDEOCAP0 and VIDEOCAP1
	VENDOR_VIDEOCAP_PARAM_PROBE_SENSOR,          ///< using device id, probe sensor refer to HD_VIDEOCAP_SENSOR_DEVICE struct
	VENDOR_VIDEOCAP_PARAM_MODE_TYPE,             ///< for i/o path, set sensor mode type, referring to VENDOR_VCAP_SEN_MODE_TYPE
	VENDOR_VIDEOCAP_PARAM_PDAF_TYPE,             ///< for i/o path, set sensor mode type, referring to VENDOR_VCAP_PDAF_TYPE
	VENDOR_VIDEOCAP_PARAM_RESET_FC,              ///< for i/o path, reset the frame count of VCAPs
	VENDOR_VIDEOCAP_PARAM_GYRO_INFO,             ///< for i/o path, referring to VENDOR_VIDEOCAP_GYRO_INFO
	VENDOR_VIDEOCAP_PARAM_USER_PINMUX,           ///< for ctrl path, specify sensor_pinmux without referring to dtsi, referring to HD_VIDEOCAP_SEN_CONFIG_EXT
	VENDOR_VIDEOCAP_PARAM_DEFAULT_CCIR_FMT,      ///< for ctrl path, set default format for AD driver, referring to VENDOR_VIDEOCAP_CCIR_FMT_SEL
	VENDOR_VIDEOCAP_PARAM_SSENIF_FORCE_DIS,      ///< for i/o path, enable the function of forcing to disable sensor interface
	VENDOR_VIDEOCAP_PARAM_KEEP_PINMUX_SETTING,   ///< for i/o path, keep pinmux setting for I2C
	VENDOR_VIDEOCAP_PARAM_PDAF_PXLFMT,           ///< for i/o path, set pixel format for PDAF sensor which only use one VCAP
	VENDOR_VIDEOCAP_PARAM_EVS_INFO,              ///< for i/o path, referring to VENDOR_VIDEOCAP_EVS_INFO
	VENDOR_VIDEOCAP_PARAM_ADVANCE_PUSH,          ///< for i/o path, set TRUE to enable AdvancePush
	VENDOR_VIDEOCAP_PARAM_TGE_CTRL,              ///< for i/o path, control TGE engine in run time, referring to VENDOR_VIDEOCAP_TGE_CTRL
	VENDOR_VIDEOCAP_PARAM_TSEN_CFG,              ///< for i/o path, set config for thermal sensor, referring to VENDOR_VCAP_TSEN_CFG
	VENDOR_VIDEOCAP_PARAM_TSEN_OOC,              ///< for i/o path, set OOC for thermal sensor, referring to VENDOR_VCAP_TSEN_OOC
	ENUM_DUMMY4WORD(VENDOR_VIDEOCAP_PARAM_ID)
} VENDOR_VIDEOCAP_PARAM_ID;
/********************************************************************
	EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
HD_RESULT vendor_videocap_set(UINT32 id, VENDOR_VIDEOCAP_PARAM_ID param_id, VOID *p_param);
HD_RESULT vendor_videocap_get(UINT32 id, VENDOR_VIDEOCAP_PARAM_ID param_id, VOID *p_param);
//only for fast boot in RTOS
HD_RESULT vendor_videocap_set_lite(UINT32 id, VENDOR_VIDEOCAP_PARAM_ID param_id, VOID *p_param);

#ifdef __cplusplus
}
#endif

#endif

