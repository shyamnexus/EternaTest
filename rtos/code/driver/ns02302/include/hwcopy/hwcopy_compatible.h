/**
    HwCopy module driver

    @file       hwcopy.h
    @ingroup    mIDrvIPP_Hwcopy
    @note
\n              -hwcopy_open() to start this module.
\n              -hwcopy_close() to close this module.
\n
\n              This module will have to enable and wait the system interrupt.
\n
\n              -hwcopy_request. set parameters of images.
\n
\n
\n              Usage example:
\n
\n              (1)hwcopy_open(HWCOPY_ID_1);
\n              (2)Fill request;
\n                 hwcopy_request(HWCOPY_ID_1, &request);
\n                      :
\n              (3)hwcopy_close(HWCOPY_ID_1);

    Copyright   Novatek Microelectronics Corp. 2016.  All rights reserved.
*/
#ifndef   _HWCOPY_H
#define   _HWCOPY_H

#include "kwrap/type.h"
#include "hwcopy_int.h"
#include "hwcopy_reg.h"

/**
    @addtogroup mIDrvIPP_Hwcopy
*/
//@{
/**
    HwCopy request structure

    Description of hwcopy request

    @note For hwcopy_request()
*/
typedef struct _HWCOPY_REQUEST{
	HWCOPY_CMD command;
	PHWCOPY_MEM p_memory_descript;       ///< Descriptions of Memories for command
	PHWCOPY_CTEX p_ctex_descript;
    uintptr_t io_addr;

    struct _HWCOPY_REQUEST *r_next;

} HWCOPY_REQUEST, *PHWCOPY_REQUEST;

extern void hwcopy_isr_bottom(HWCOPY_CHANNEL ch, UINT32 events);
extern ER hwcopy_trigger(HWCOPY_CHANNEL ch, PHWCOPY_REQUEST p_request);



// Functions exported from HwCopy driver
extern ER       hwcopy_open(void);
extern BOOL     hwcopy_is_opened(HWCOPY_CHANNEL ch);
extern ER       hwcopy_close(void);
extern ER       hwcopy_request(HWCOPY_CHANNEL ch, PHWCOPY_REQUEST p_request);
//@}

#endif
