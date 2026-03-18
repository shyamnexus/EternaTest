/**
	@brief Header file of vendor dsp module.\n

	@file vendor_dsp.h

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#ifndef _VENDOR_DSP_H_
#define _VENDOR_DSP_H_

/********************************************************************
	INCLUDE FILES
********************************************************************/
#include "hd_type.h"

/********************************************************************
	MACRO CONSTANT DEFINITIONS
********************************************************************/
#define DSP_MAX_FILE_PATH_LEN 64
/********************************************************************
	MACRO FUNCTION DEFINITIONS
********************************************************************/

/********************************************************************
	TYPE DEFINITION
********************************************************************/
/**
    DSP core ID.
*/
typedef enum _DSP_CORE_ID {
	DSP_CORE_ID_1 = 0,                       ///< DSP core 1
	DSP_CORE_ID_2,                           ///< DSP core 2
	DSP_CORE_ID_MAX_NUM,
	ENUM_DUMMY4WORD(DSP_CORE_ID)
} DSP_CORE_ID;

typedef struct {
	UINT32 core_id;                          ///< DSP core id
	UINT32 reset_addr;                       ///< DSP reset address
	UINT32 prog_int_addr;                    ///< DSP internal program address
	UINT32 data_int_addr;                    ///< DSP internal data address
} VENDOR_DSP_OPEN, *PVENDOR_DSP_OPEN;

/**
    DSP dram range.
*/
typedef struct {
	ULONG start_addr;   ///< DSP starting address
	ULONG size;         ///< DSP size
} VENDOR_DSP_DRAM_RANGE, *PVENDOR_DSP_DRAM_RANGE;

typedef struct {
	CHAR *path_name;    ///< path name
	UINT32 path_len;    ///< path length, should include '\0'
} VENDOR_DSP_FILE_PATH;

typedef enum {
	VENDOR_DSP_PARAM_FILE_PATH,           ///< specify the file path of DSP bin, referring to VENDOR_DSP_FILE_PATH
	ENUM_DUMMY4WORD(VENDOR_DSP_PARAM_ID)
} VENDOR_DSP_PARAM_ID;

extern HD_RESULT vendor_dsp_init(void);
extern HD_RESULT vendor_dsp_get_dram_range(DSP_CORE_ID dsp_core_id, VENDOR_DSP_DRAM_RANGE *p_range);
extern HD_RESULT vendor_dsp_open(VENDOR_DSP_OPEN *p_dsp_open);
extern HD_RESULT vendor_dsp_close(DSP_CORE_ID dsp_core_id);
extern HD_RESULT vendor_dsp_is_running(DSP_CORE_ID dsp_core_id, unsigned long *is_running);
extern HD_RESULT vendor_dsp_uninit(void);
extern HD_RESULT vendor_dsp_disable_clock(DSP_CORE_ID dsp_core_id);
extern HD_RESULT vendor_dsp_enable_clock(DSP_CORE_ID dsp_core_id);
extern HD_RESULT vendor_dsp_set(DSP_CORE_ID dsp_core_id, VENDOR_DSP_PARAM_ID param_id, VOID *p_param);


#endif

