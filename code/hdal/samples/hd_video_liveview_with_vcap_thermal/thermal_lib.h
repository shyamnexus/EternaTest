/**
	@brief Header file of thermal lib module.\n

	@file thermal_lib.h

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2024.  All rights reserved.
*/

#ifndef _THERMAL_LIB_H_
#define _THERMAL_LIB_H_

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "hdal.h"
#include "hd_debug.h"

#define TSEN_RESTART_CALIBRATION       0xFFFFFFFF

#define TSEN_GET_8BitsValue(addr)            (*(volatile UINT8*)((uintptr_t)addr))
#define TSEN_SET_8BitsValue(addr, value)     (*(volatile UINT8*)((uintptr_t)addr) = (UINT8)(value))
#define TSEN_GET_32BitsValue(addr)           (*(volatile UINT32*)((uintptr_t)addr))
#define TSEN_SET_32BitsValue(addr, value)    (*(volatile UINT32*)((uintptr_t)addr) = (UINT32)(value))


#define TSEN_CFG_NUM                   16

typedef struct {
	UINT32  sz_x;      ///< horizontal window size
	UINT32  sz_y;      ///< vertical window size
	UINT32  lofs;      ///< lineoffset, unit: byte
	UINT32  depth;     ///< pixel depth, unit: byte
} TSEN_RAW_INFO;


typedef enum {
	IRAY_6122C_25HZ,
	IRAY_6122C_50HZ,
	IRAY_1212W,
	GST_612W,
	ENUM_DUMMY4WORD(TSEN_SENSOR_ID)
} TSEN_SENSOR_ID;

typedef enum {
	UPDATE_NONE         = 0,
	UPDATE_CONFIG       = 0x00000001,
	UPDATE_OOC          = 0x00000002,
	UPDATE_DONE         = 0x00000004,
	ENUM_DUMMY4WORD(TSEN_UPDATE_RESULT)
} TSEN_UPDATE_RESULT;

typedef enum {
    TSEN_OOC_PACKBUS_6 = 0,     ///< 6 bits packing
    TSEN_OOC_PACKBUS_7,         ///< 7 bits packing
    TSEN_OOC_PACKBUS_8,         ///< 8 bits packing
    TSEN_OOC_PACKBUS_MAX,
    ENUM_DUMMY4WORD(TSEN_OOC_PACKBUS_SEL)
} TSEN_OOC_PACKBUS_SEL;

typedef struct {
	UINT32 len;                   ///< from 4 ~ 512 clock cycle
	UINT32 tx_data[TSEN_CFG_NUM]; ///< config data
	UINT32 id;                    ///< [in] config id from current RAW frame, [out] config id for next RAW frame
} TSEN_CONFIG;

typedef struct {
	ULONG addr;                        // [in] ooc buffer address
	TSEN_OOC_PACKBUS_SEL pack_bus_sel; // [in] referring to TSEN_OOC_PACKBUS_SEL
	UINT32 id;                         // [in] OOC id from current RAW frame, [out] OOC id for next RAW frame
	UINT32 width;                      // [in] OOC width
	UINT32 height;                     // [in] OOC height
	UINT32 lofs;                       // [in] lineoffset
} TSEN_OOC_INFO;

extern UINT32 tsen_get_working_buf_size(TSEN_SENSOR_ID sensor_id, TSEN_RAW_INFO *p_raw_info);
extern void tsen_init(ULONG work_buf, UINT32 work_buf_size);
extern void tsen_get_init_config(TSEN_CONFIG *p_config, UINT32 serial_id);
extern void tsen_get_init_ooc(TSEN_OOC_INFO *p_ooc_info);
extern TSEN_UPDATE_RESULT tsen_update_params(ULONG raw_buf, TSEN_OOC_INFO *p_ooc_info, TSEN_CONFIG *p_config);
extern void tsen_dump_int_status(void);

#endif
