/**
    Header file for GPENC module.

    This file is the header file that define the API and data type
    for GPENC module.

    @file       wdt.h
    @ingroup    miDrvDisplay_GPENC
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _GPENC_H
#define _GPENC_H

//#include "Type.h"
#include <kwrap/nvt_type.h>

#if (defined __FREERTOS)
#define GPENCDATA_TYPE	UINT32
#else
#define GPENCDATA_TYPE	UINT64
#endif

/**
    GPENC Module ID

    This is used by the GPENC Get Object API gpenc_get_drv_object() to get the specific GPENC module control object.
*/
typedef enum {
	GPENC_ID_0,     ///< GPENC  Controller ID
	GPENC_ID_1, 	///< GPENC2	Controller ID
	GPENC_ID_MAX,
	ENUM_DUMMY4WORD(GPENC_ID)
} GPENC_ID;


/**
    GPENC configuration ID

    GPENC configuration ID for gpenc_set_config and gpenc_get_config
*/
typedef enum {
	GPENC_CONFIG_ID_IDX_WIDTH,    ///< config/get GPENC index width
	GPENC_CONFIG_ID_SRC_WIDTH,    ///< config/get GPENC source width
	GPENC_CONFIG_ID_WAIT_TIME,    ///< config/get waiting time
	GPENC_CONFIG_ID_READ_ADDR,    ///< config/get read address
	GPENC_CONFIG_ID_READ_SIZE,    ///< config/get read size
	GPENC_CONFIG_ID_WRITE_ADDR,   ///< config/get write address
	GPENC_CONFIG_ID_WRITE_LIMIT,  ///< config/get write limit  
	GPENC_CONFIG_ID_IMG_W,        ///< config/get image width
	GPENC_CONFIG_ID_IMG_H,        ///< config/get image height
	GPENC_CONFIG_ID_IMG_OFS,      ///< config/get image lineoffset
	GPENC_CONFIG_ID_SW_RST,       ///< config gpenc sw reset

	ENUM_DUMMY4WORD(GPENC_CONFIG_ID)
} GPENC_CONFIG_ID;

typedef enum {
	GPENC_SOURCE_WIDTH_1BYTE  = 0,    ///< source width 1B 
	GPENC_SOURCE_WIDTH_2BYTES = 1,    ///< source width 2B 
	GPENC_SOURCE_WIDTH_4BYTES = 2,    ///< source width 4B

	ENUM_DUMMY4WORD(GPENC_SOURCE_WIDTH)
} GPENC_SOURCE_WIDTH;

typedef enum {
	GPENC_IDX_WIDTH_3BITS = 0,     ///< config index width = 3 bits
	GPENC_IDX_WIDTH_4BITS = 1,     ///< config index width = 4 bits
	GPENC_IDX_WIDTH_5BITS = 2,     ///< config index width = 5 bits
	GPENC_IDX_WIDTH_6BITS = 3,     ///< config index width = 6 bits
	GPENC_IDX_WIDTH_7BITS = 4,     ///< config index width = 7 bits
	
	ENUM_DUMMY4WORD(GPENC_IDX_WIDTH)
} GPENC_IDX_WIDTH;


/**
    GPENC Driver Object Definitions

*/
typedef struct {
	ER(*open)(void);///< module driver open
	ER(*close)(void);///< module driver close
	BOOL (*is_opened)(void); ///< check module driver is opened

	void (*set_config)(GPENC_CONFIG_ID config_id, UINT32 cfg_value);///< set module configurations
	UINT32(*get_config)(GPENC_CONFIG_ID config_id);///< get module configurations

	void (*set_trigger)(void);///< set module trigger
	void (*wait_done)(void);///< module wait done

	UINT32(*get_enc_size)(void);///< get module encode size

} GPENCOBJ, *PGPENCOBJ;

#ifdef __KERNEL__
extern void 	gpenc_create_resource(void);
extern void 	gpenc_release_resource(void);
extern void 	gpenc_set_base_addr(GPENCDATA_TYPE addr);
extern void		gpenc_isr(void);
extern void 	gpenc2_create_resource(void);
extern void 	gpenc2_release_resource(void);
extern void 	gpenc2_set_base_addr(GPENCDATA_TYPE addr);
extern void		gpenc2_isr(void);
#endif

extern ER       gpenc_open(void);
extern ER       gpenc_close(void);
extern BOOL     gpenc_is_opened(void);
extern void     gpenc_set_config(GPENC_CONFIG_ID config_id, UINT32 ui_config);
extern UINT32   gpenc_get_config(GPENC_CONFIG_ID config_id);
extern void     gpenc_set_trigger(void);
extern void     gpenc_wait_done(void);
extern UINT32   gpenc_get_enc_size(void);

extern ER       gpenc2_open(void);
extern ER       gpenc2_close(void);
extern BOOL     gpenc2_is_opened(void);
extern void     gpenc2_set_config(GPENC_CONFIG_ID config_id, UINT32 ui_config);
extern UINT32   gpenc2_get_config(GPENC_CONFIG_ID config_id);
extern void     gpenc2_set_trigger(void);
extern void     gpenc2_wait_done(void);
extern UINT32   gpenc2_get_enc_size(void);


extern PGPENCOBJ gpenc_get_drv_object(GPENC_ID gpenc_id);


#endif

