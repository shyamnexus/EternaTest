/**
    Public header file for high-resolution hrtimer module.

    This file is the header file that define the API and data type for hrtimer
    module.

    @file       hrhrtimer.h
    @ingroup    miDrvHRTimer_HRTimer
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.
*/

#ifndef _HRTIMER_H
#define _HRTIMER_H

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

typedef enum {
        EXTCNT_0,                        ///< external counter 0
        EXTCNT_1,                        ///< external counter 1
        EXTCNT_2,                        ///< external counter 2
        EXTCNT_3,                        ///< external counter 3
        EXTCNT_4,                        ///< external counter 4
        EXTCNT_5,                        ///< external counter 5
        EXTCNT_6,                        ///< external counter 6
        EXTCNT_7,                        ///< external counter 7

        ENUM_DUMMY4WORD(EXTCNT_ID)
} EXTCNT_ID;

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
extern ER       hrtimer_open(void);
extern ER       hrtimer_close(void);
extern ER       hrtimer_cfg(UINT32 interval_lb, UINT32 interval_ub,HRTIMER_MODE mode, HRTIMER_STATE state);
extern ER       hrtimer_pause_play(HRTIMER_STATE state);
extern ER       hrtimer_wait_timeup(void);
extern ER       hrtimer_check_timeup(BOOL *p_check);
extern ER       hrtimer_reload(UINT32 interval_lb, UINT32 interval_ub);
// HRTimer Utility API
extern void     hrtimer_set_config(UINT32 config);
extern UINT32   hrtimer_get_config(void);
extern UINT64   hrtimer_get_current_count(void);
extern UINT64   hrtimer_get_target_count(void);
extern void     hrtimer_chg_initcnt(UINT32 chg_lb, UINT32 chg_ub);
extern UINT32   hrtimer_get_initcnt_lb(void);
extern UINT32   hrtimer_get_initcnt_ub(void);

extern void hrtimer_enable_extcnt(UINT32 en_mask);
extern void hrtimer_set_extcnt_buf0(UINT32 en_mask);
extern void hrtimer_set_extcnt_buf1(UINT32 en_mask);
extern UINT32 hrtimer_get_extcnt_index(EXTCNT_ID ext_cnt_id);
extern UINT64 hrtimertest_get_extcnt_count(EXTCNT_ID ext_cnt_id, int buf_index);

//@}

#endif
