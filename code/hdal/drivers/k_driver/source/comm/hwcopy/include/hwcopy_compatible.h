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
#include "comm/kdrv_hwcopy.h"

/**
    @addtogroup mIDrvIPP_Hwcopy
*/
//@{

/**
    HwCopy channel ID

    HwCopy channel ID
*/
/*
typedef enum {
    HWCOPY_ID_0,
	HWCOPY_ID_1,                          ///< Hwcopy Controller
	HWCOPY_ID_MAX,                          ///< Hwcopy Controller
	ENUM_DUMMY4WORD(HWCOPY_ID)
} HWCOPY_ID;
*/
/**
    HwCopy request structure

    Description of hwcopy request

    @note For hwcopy_request()
*/
typedef struct {
	//DRV_VER_INFO ver_info;           ///< Driver version
	HWCOPY_CMD command;
	PHWCOPY_MEM p_memory_descript;       ///< Descriptions of Memories for command
	PHWCOPY_CTEX p_ctex_descript;

    uintptr_t io_addr;
    BOOL flush;

} HWCOPY_REQUEST, *PHWCOPY_REQUEST;

/*
	Request list element

*/
typedef struct _HWCOPY_REQ_LIST_NODE {
	KDRV_HWCOPY_TRIGGER_PARAM	trig_param;

	HWCOPY_MEM		memory[2];

	KDRV_CALLBACK_FUNC	callback;

	KDRV_HWCOPY_EVENT_CB_INFO cb_info;

    uintptr_t io_addr;

//	struct list_head	list; // just linux
} HWCOPY_REQ_LIST_NODE;

extern void hwcopy_isr_bottom(HWCOPY_ID id, UINT32 events);
extern ER hwcopy_trigger(HWCOPY_ID id);
extern ER hwcopy_enqueue(HWCOPY_ID id, KDRV_HWCOPY_TRIGGER_PARAM *p_param,
                KDRV_CALLBACK_FUNC *p_cb_func);



// Functions exported from HwCopy driver
extern ER       hwcopy_open(HWCOPY_ID id);
extern BOOL     hwcopy_is_opened(HWCOPY_ID id);
extern ER       hwcopy_close(HWCOPY_ID id);

extern ER       hwcopy_request(HWCOPY_ID id, PHWCOPY_REQUEST p_request);

//@}

#endif
