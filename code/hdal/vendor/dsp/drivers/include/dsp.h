/**
    DSP module driver.

    This file is the driver of DSP module.

    @file       DSP.h
    @ingroup    mIDrvSys_DSP
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2011.  All rights reserved.
*/

#ifndef _DSP_H
#define _DSP_H

#ifdef __KERNEL__
#include "kwrap/type.h"
//#include "kdrv_type.h"
#else
#include <kwrap/nvt_type.h>
#endif
/**
    DSP ID
*/
//@{
typedef enum {
	DSP_ID_1,                          ///< DSP Controller
	DSP_ID_MAX,                        ///< DSP ID MAX.
	DSP_ID_2,                          ///< DSP2 Controller
	ENUM_DUMMY4WORD(DSP_ID)
} DSP_ID;
//@}

/**
    DSP BOOT mode
*/
//@{
typedef enum {
	DSP_BOOTMODE_RESET_SEQ,           ///< DSP BOOT mode: reset sequence
	DSP_BOOTMODE_BOOT_SEQ,            ///< DSP BOOT mode: boot sequence

	ENUM_DUMMY4WORD(DSP_BOOT_MODE)
} DSP_BOOT_MODE;
//@}

/**
    DSP CONFIG ID
*/
//@{
typedef enum {
	DSP_CONFIG_ID_BOOTVECTOR,         ///< Configure boot vector
	DSP_CONFIG_ID_BOOTMODE,           ///< Configure boot mode

	DSP_CONFIG_STS_REG,               ///< Configure get current dsp status
	DSP_CONFIG_ID_DERESET,            ///< Configure de-reset (core, sys, ocem, global)


	DSP_CONFIG_ID_DEBUGPORTEN,        ///< Coinfig debug port enable
	DSP_CONFIG_ID_DEBUGPORTSEL,       ///< Coinfig debug port selection

	DSP_CONFIG_ID_EPP_POSTD,          ///< Config EPP POSTD
	DSP_CONFIG_ID_EPP_PAWBA_MODE,     ///< Config EPP PAWBA mode
	DSP_CONFIG_ID_EPP_AWBA_SET,       ///< Config EPP AWBA
	DSP_CONFIG_ID_EPP_PPRE_RD_OFF,    ///< Config EPP PPRE_RD_OFF

	DSP_CONFIG_ID_EDP_POSTD,          ///< Config EDP POSTD
	DSP_CONFIG_ID_EDP_PAWBA_MODE,     ///< Config EDP PAWBA mode
	DSP_CONFIG_ID_EDP_AWBA_SET,       ///< Config EDP AWBA
	DSP_CONFIG_ID_EDP_PPRE_RD_OFF,    ///< Config EDP PPRE_RD_OFF

	DSP_CONFIG_ID_IOPVECTOR,          ///< Config IOP memory
	DSP_CONFIG_ID_EDPREMAP,           ///< Config EDP remap
	DSP_CONFIG_ID_ERR_CALLBACK,       ///< Config ERR CALLBACK
	DSP_CONFIG_ID_TIMEREN,            ///< Config Timer Enable
	DSP_CONFIG_ID_TIMERRST,           ///< Config Timer reset

	ENUM_DUMMY4WORD(DSP_CONFIG_ID)
} DSP_CONFIG_ID;
//@}

/**
    DSP INTERRUPT ERROR CALLBACK ID
*/
//@{
typedef enum {
	DSP_INT_ERR_WDOG_EPP = 0x1,   ///< DSP interrupt error EPP watchdog.
	DSP_INT_ERR_WDOG_EDP = 0x2,   ///< DSP interrupt error EDP watchdog.
	DSP_INT_ERR_WDOG_IOP = 0x8,   ///< DSP interrupt error IOP watchdog.
	DSP_INT_ERR_UOP = 0x10,       ///< DSP interrupt error UOP (undefined opcode).
	DSP_INT_ERR_GVI = 0x20,       ///< DSP interrupt error GVI (global violation indicator).

	ENUM_DUMMY4WORD(DSP_INT_ERR_ID)
} DSP_INT_ERR_ID;
//@}

#ifdef __KERNEL__
extern void     dsp_isr(void);
extern void     dsp_create_resource(void);
extern void     dsp_release_resource(void);
extern void     dsp_set_base_addr(ULONG addr, ULONG edap_addr);

extern void     dsp2_isr(void);
extern void     dsp2_create_resource(void);
extern void     dsp2_release_resource(void);
extern void     dsp2_set_base_addr(ULONG addr, ULONG edap_addr);
#endif

extern ER 		dsp_open(DSP_ID id);
extern ER 		dsp_close(DSP_ID id);
extern BOOL 	dsp_is_opened(DSP_ID id);
extern ER 		dsp_set_config(DSP_ID id, DSP_CONFIG_ID config_id, ULONG cfg_value);
extern UINT32	dsp_get_config(DSP_ID id, DSP_CONFIG_ID config_id);
//extern UINT32 	dsp_pll_get_dsp_freq(void);




#endif
