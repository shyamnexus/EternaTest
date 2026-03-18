/**
 * @file kdrv_hwcopy.h
 * @brief type definition of KDRV API.
 * @author ESW
 * @date in the year 2018
 */

#ifndef __KDRV_HWCOPY_H__
#define __KDRV_HWCOPY_H__

#include <stdint.h>
#include <kwrap/nvt_type.h>
#include <io_address.h>
#include <rcw_macro.h>
#include <sys.h>

#define MAU1_Addr	0x40000000

#define FLGPTN_HWCOPY  0x01
#define ENUM_DUMMY4WORD(name)   E_##name = 0x10000000

/**
     @name hwcopy register access definition
     @note
*/
//-1 means contoller
#define HWCOPY_GETREG(channel,ofs)          INW(IOADDR_HWCP_REG_BASE + (channel+1) * 0x100 + (ofs))                  ///< get hwcopy reg
#define HWCOPY_SETREG(channel,ofs,value)    OUTW(IOADDR_HWCP_REG_BASE + (channel+1) * 0x100 + (ofs), value)      ///< set hwcopy reg

typedef enum
{
    HWCOPY_CHC = -1,
    HWCOPY_CH0,
    HWCOPY_CH1,
    HWCOPY_CH2,
    HWCOPY_CH3,
    HWCOPY_CHMAX = 4,
    ENUM_DUMMY4WORD(HWCOPY_CHANNEL)
} HWCOPY_CHANNEL;

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
	HWCOPY_MEM_ID mem_id;               ///< Memory identifier
	ULONG  address;	                    ///< Memory DRAM address (unit: byte)
	UINT32 lineoffset;                  ///< Memory lineoffset (unit: byte) (Must be 4 byte alignment)
	UINT32 width;                       ///< Memory width (unit: byte)
	UINT32 height;                      ///< Memory height (unit: line)
	UINT32 PA_ADD;						///< if PA_ADD exist use this

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

typedef enum {
	HWCOPY_MODE_SET_LINEAR,
	HWCOPY_MODE_SET_BLOCK,
	HWCOPY_MODE_COPY_LINEAR,
	HWCOPY_MODE_COPY_BLOCK,

	ENUM_DUMMY4WORD(HWCOPY_MODE_ENUM)
} HWCOPY_MODE_ENUM;


typedef enum {
    HWCOPY_DONE,
	HWCOPY_LL_DONE,
	HWCOPY_JOB_DONE,

	ENUM_DUMMY4WORD(HWCOPY_EVENT_TYPE)
} HWCOPY_EVENT_TYPE;



#endif
