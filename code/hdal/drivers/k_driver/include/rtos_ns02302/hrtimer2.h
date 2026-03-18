/**
    Public header file for high-resolution hrtimer module.

    This file is the header file that define the API and data type for hrtimer
    module.

    @file       hrhrtimer.h
    @ingroup    miDrvHRTimer_HRTimer
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.
*/

#ifndef _HRTIMER2_H
#define _HRTIMER2_H

#include "comm/driver.h"
#include <kwrap/nvt_type.h>

/**
    @addtogroup miDrvHRTimer_HRTimer
*/
//@{

/**
    HRTimer mode.

    HRTimer mode.
*/
typedef enum {
	HRTIMER_MODE_ONE_SHOT     = 0x00000010,   ///< HRTimer operation mode is "one shot"
	HRTIMER_MODE_FREE_RUN     = 0x00000020,   ///< HRTimer operation mode is "free run"
	HRTIMER_MODE_ENABLE_INT   = 0x00000040,   ///< Enable timeout interrupt

	ENUM_DUMMY4WORD(HRTIMER_MODE)
} HRTIMER_MODE, *PHRTIMER_MODE;

/**
    HRTimer state.

    HRTimer state.
*/
typedef enum {
	HRTIMER_STATE_PAUSE,              ///< Stop hrtimer
	HRTIMER_STATE_PLAY,               ///< Start hrtimer

	ENUM_DUMMY4WORD(HRTIMER_STATE)
} HRTIMER_STATE, *PHRTIMER_STATE;

// HRTimer Driver API
extern ER       hrtimer2_open(void);
extern ER       hrtimer2_close(void);
extern ER       hrtimer2_cfg(UINT32 interval_lb, UINT32 interval_ub,HRTIMER_MODE mode, HRTIMER_STATE state);
extern ER       hrtimer2_pause_play(HRTIMER_STATE state);
extern ER       hrtimer2_wait_timeup(void);
extern ER       hrtimer2_check_timeup(BOOL *p_check);
extern ER       hrtimer2_reload(UINT32 interval_lb, UINT32 interval_ub);
// HRTimer Utility API
extern void     hrtimer2_set_config(UINT32 config);
extern UINT32   hrtimer2_get_config(void);
extern UINT64   hrtimer2_get_current_count(void);
extern UINT64   hrtimer2_get_target_count(void);
extern void     hrtimer2_chg_initcnt(UINT32 chg_lb, UINT32 chg_ub);
extern UINT32   hrtimer2_get_initcnt_lb(void);
extern UINT32   hrtimer2_get_initcnt_ub(void);

//@}

#endif
