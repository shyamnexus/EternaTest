/**
    HwCopy module internal header

    HwCopy module internal header

    @file       hwcopy_int.h
    @ingroup    mIDrvIPP_Hwcopy
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2016.  All rights reserved.
*/
#ifndef  _HWCOPY_INT_H
#define  _HWCOPY_INT_H

#include "hwcopy_platform.h"

/**
     @name hwcopy register access definition
     @note
*/
//@{

//@}

/**
     Hwcopy engine internal status.

     Hwcopy engine internal status.
     @note
*/
typedef enum {
	HWCOPY_ENGINE_IDLE,     ///< engine idle
	HWCOPY_ENGINE_READY,    ///< engine ready
	HWCOPY_ENGINE_RUN,      ///< engine running

	ENUM_DUMMY4WORD(HWCOPY_ENGINE_STATUS)
} HWCOPY_ENGINE_STATUS;

#endif
