#ifndef _AD_TP2830_H_
#define _AD_TP2830_H_

#include "ad_drv.h"

/************************ Common information ************************/

// maximum number of chips which is supported by the external decoder driver
#define AD_TP2830_CHIP_MAX 	4

// number of input channels in one chip
#define AD_TP2830_VIN_MAX 	4

// number of output ports in one chip
#define AD_TP2830_VOUT_MAX 	2

// i2c (max) address bytes & data bytes
#define AD_TP2830_I2C_ADDR_LEN 1
#define AD_TP2830_I2C_DATA_LEN 1

// i2c data order. DISABLE: LSB, ENABLE: MSB
#define AD_TP2830_I2C_DATA_MSB ENABLE


/************************** Ext information **************************/

#define AD_TP2830_VIDEO_EQ_LOCKED 4 	// extension of VIDEO_LOCKED

typedef struct {
	UINT32 mode;
	UINT32 eq;
} AD_TP2830_DET_RAW_MODE;

typedef struct {
	UINT32 width;
	UINT32 height;
	UINT32 fps;
	BOOL prog;
	UINT32 std;
} AD_TP2830_DET_VIN_MODE;

typedef struct {
	AD_TP2830_DET_RAW_MODE raw_mode;
	AD_TP2830_DET_VIN_MODE vin_mode;
} AD_TP2830_DET_MAP;

typedef struct {
	UINT32 width;
	UINT32 height;
	UINT32 fps;
	BOOL prog;
	UINT32 std;
} AD_TP2830_VIN_MODE;

typedef struct {
	UINT32 mode;
	UINT32 std;
	BOOL half;
} AD_TP2830_CH_MODE;

typedef struct {
	AD_TP2830_VIN_MODE vin_mode;
	AD_TP2830_CH_MODE ch_mode;
} AD_TP2830_VIN_MAP;

typedef struct {
} AD_TP2830_VOUT_MODE;

typedef struct {
} AD_TP2830_PORT_MODE;

typedef struct {
	AD_TP2830_VOUT_MODE vout_mode;
	AD_TP2830_PORT_MODE port_mode;
} AD_TP2830_VOUT_MAP;

typedef struct {
	BOOL vloss;
	UINT32 width;
	UINT32 height;
	UINT32 fps;
	BOOL prog;
	UINT32 std;
} AD_TP2830_DET_VIDEO_INFO;

typedef enum {
	AD_TP2830_IMAGE_PARAM_TYPE_MAX,
} AD_TP2830_IMAGE_PARAM_TYPE;

/*******************************************************************************
* vin_id: video input id                                                       *
* vout_id: video output id                                                     *
* val: parameter value                                                         *
*******************************************************************************/
typedef struct {
	UINT32 vin_id;					///< [in]
	AD_TP2830_IMAGE_PARAM_TYPE type;	///< [in]

	UINT32 val;						///< [in/out]
} AD_TP2830_IMAGE_PARAM_INFO;

typedef enum {
	AD_TP2830_AUDIO_PARAM_TYPE_MAX,
} AD_TP2830_AUDIO_PARAM_TYPE;

/*******************************************************************************
* val: parameter value                                                         *
*******************************************************************************/
typedef struct {
	AD_TP2830_AUDIO_PARAM_TYPE type;	///< [in]

	UINT32 val;						///< [in/out]
} AD_TP2830_AUDIO_PARAM_INFO;

/*******************************************************************************
* vin_id: video input id                                                       *
* addr: read/write register address                                            *
* val: parameter value                                                         *
*******************************************************************************/
typedef struct {
	UINT32 vin_id;					///< [in]
	UINT32 addr;					///< [in]

	UINT32 val;						///< [in/out]
} AD_TP2830_PTZ_REG_INFO;

typedef enum {
	AD_TP2830_PATTERN_GEN_MODE_DISABLE,
	AD_TP2830_PATTERN_GEN_MODE_AUTO,
	AD_TP2830_PATTERN_GEN_MODE_FORCE,
} AD_TP2830_PATTERN_GEN_MODE;

#define AD_TP2830_WATCHDOG_GAIN_NUM 4
typedef struct {
	UINT32 count;
	UINT32 mode;
	UINT32 scan;
	UINT32 gain[AD_TP2830_WATCHDOG_GAIN_NUM];
	UINT32 std;
	UINT32 state;
	UINT32 force;
	UINT32 eq_locked;
} AD_TP2830_WATCHDOG_INFO;

typedef struct {
	BOOL is_inited;
	AD_TP2830_DET_RAW_MODE raw_mode[AD_TP2830_VIN_MAX];
	AD_TP2830_DET_VIN_MODE det_vin_mode[AD_TP2830_VIN_MAX];
	AD_TP2830_PATTERN_GEN_MODE pat_gen_mode[AD_TP2830_VIN_MAX];
	AD_TP2830_WATCHDOG_INFO wdi[AD_TP2830_VIN_MAX];
	AD_DET_TYPE det_type[AD_TP2830_VIN_MAX];
	AD_MUX_NUM mux; // mux num for detection
	AD_DATA_RATE_MODE dr_mode; // dr_mode for detection
} AD_TP2830_INFO;


// external function
extern AD_DEV* ad_get_obj_tp2830(void);
#endif

