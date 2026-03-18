/**
 * @file kdrv_hwcopy.h
 * @brief type definition of KDRV API.
 * @author ESW
 * @date in the year 2018
 */

#ifndef __KDRV_HWCOPY_H__
#define __KDRV_HWCOPY_H__

#include "kdrv_type.h"

typedef enum {
	HWCOPY_ID_0,				///< Hwcopy RC Controller
	HWCOPY_ID_1,				///< Hwcopy EP Controller
	HWCOPY_ID_MAX,                          ///< Hwcopy Controller MAX number
	ENUM_DUMMY4WORD(HWCOPY_ID)
} HWCOPY_ID;

/*
 * structured passed to callback's 2nd parameter "user_data"
 */
typedef struct _KDRV_HWCOPY_EVENT_CB_INFO {
	UINT32       handle;     ///< handle feed by kdrv_hwcopy_trigger()
	UINT32       timestamp;  ///< timestamp when job complete
} KDRV_HWCOPY_EVENT_CB_INFO;

/*
 * Hwcopy operation command
 *
 * Select which operation for hwcopy engine operation.
 *
 * @note Used for KDRV_HWCOPY_TRIGGER_PARAM.
 */
typedef enum _HWCOPY_CMD {
	HWCOPY_LINEAR_SET,                  ///< linear set constant to addressed memory
	HWCOPY_BLOCK_SET,                   ///< block set constant to addressed memory
	HWCOPY_LINEAR_COPY,                 ///< linear copy to addressed memory
	HWCOPY_BLOCK_COPY,                  ///< block copy to addressed memory

	HWCOPY_CMD_MAX,
	ENUM_DUMMY4WORD(HWCOPY_CMD)
} HWCOPY_CMD;

/*
 * Hwcopy memory identifier
 *
 * Select Memory A/B.
 *
 * @note For HWCOPY_MEM
 */
typedef enum _HWCOPY_MEM_ID {
	HWCOPY_MEM_ID_A,                          ///< Memory A
	HWCOPY_MEM_ID_B,                          ///< Memory B

	ENUM_DUMMY4WORD(HWCOPY_MEM_ID)
} HWCOPY_MEM_ID;

/* struct: _HWCOPY_MEM: structure to describe a block memory
 * Hwcopy memory structure
 *
 * Description of hwcopy memory
 *
 * @note For KDRV_HWCOPY_TRIGGER_PARAM
 */
typedef struct _HWCOPY_MEM {
	HWCOPY_MEM_ID mem_id;            ///< Memory identifier
	uintptr_t address;               ///< Memory DRAM address (unit: byte)
	uintptr_t p_address;               ///< Memory DRAM address (unit: byte)
	UINT32 lineoffset;            ///< Memory lineoffset (unit: byte) (Must be 4 byte alignment)
	UINT32 width;                 ///< Memory width (unit: byte)
	UINT32 height;                ///< Memory height (unit: line)

	struct _HWCOPY_MEM *p_next;         ///< Link to next Memory. (Fill NULL if this is last item)
} HWCOPY_MEM, *PHWCOPY_MEM;


/* struct: _HWCOPY_CTEX: structure to describe a memory set with constant text
 * Hwcopy memory structure
 *
 * Description of hwcopy memory to set with constant text
 *
 * @note For KDRV_HWCOPY_TRIGGER_PARAM
 */
typedef struct _HWCOPY_CTEX {
	UINT32 ctex;                    ///<  context
	UINT32 datalength;               ///< Data length of linear set and copy (unit: byte)

} HWCOPY_CTEX, *PHWCOPY_CTEX;


/* struct  _KDRV_HWCOPY_TRIGGER_PARAM: structure to describe a job
 *
 * @note For kdrv_hwcopy_trigger()
 */
typedef struct _KDRV_HWCOPY_TRIGGER_PARAM {
	HWCOPY_CMD         command;        ///< Hwcopy Command
	HWCOPY_MEM         *p_memory;      ///< Description of memory for this command
	HWCOPY_CTEX        *p_ctex_descript;
    uintptr_t io_addr;
    BOOL flush;
} KDRV_HWCOPY_TRIGGER_PARAM;

/*!
 * @fn INT32 kdrv_hwcopy_open(UINT32 chip, UINT32 engine)
 * @brief open hardware engine
 * @param chip      the chip id of hardware
 * @param engine    the engine id of hardware
 *          	    - @b KDRV_GFX2D_HWCOPY: hwcopy engine
 * @return return 0 on success, -1 on error
 */
extern INT32 kdrv_hwcopy_open(UINT32 chip, UINT32 engine);

/*!
 * @fn INT32 kdrv_hwcopy_close(UINT32 chip, UINT32 engine)
 * @brief close hardware engine
 * @param chip      the chip id of hardware
 * @param engine    the engine id of hardware
 *          	    - @b KDRV_GFX2D_HWCOPY: hwcopy engine
 * @return return 0 on success, -1 on error
 */
extern INT32 kdrv_hwcopy_close(UINT32 chip, UINT32 engine);

/*!
 * @fn INT32 kdrv_hwcopy_trigger(INT32 handler, KDRV_HWCOPY_TRIGGER_PARAM *p_param,
                                 KDRV_CALLBACK_FUNC *p_cb_func,
                                 VOID *p_user_data);
 * @brief trigger hardware engine
 * @param id           		the id of hardware
 * @param p_param               the parameter for trigger
 * @param p_cb_func             the callback function
 * @param p_user_data           the private user data
 * @return return 0 on success, -1 on error
 */
extern INT32 kdrv_hwcopy_trigger(UINT32 id ,KDRV_HWCOPY_TRIGGER_PARAM *p_param,
						KDRV_CALLBACK_FUNC *p_cb_func,
						VOID *p_user_data);

#endif
