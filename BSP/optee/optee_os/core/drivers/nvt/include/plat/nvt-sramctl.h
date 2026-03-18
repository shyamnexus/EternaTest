/**
    SRAM Control header file
    This file will handle core communications.
    @file       nvt-sramctl.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#ifndef _NA51055_NVT_SRAMCTL_H
#define _NA51055_NVT_SRAMCTL_H


#include <kwrap/nvt_type.h>

//#define ENUM_DUMMY4WORD(m)

/*
    SRAM ShutDown ID

    This is for nvt_disable_sram_shutdown() and nvt_enable_sram_shutdown().
*/
typedef enum {

#if defined(_BSP_NS02302_)
    RSA_SD = 14,           //< Shut Down RSA SRAM
#elif defined(_BSP_NS02301_)
    RSA_SD = 10,
#else
	RSA_SD = 25,           //< Shut Down RSA SRAM
#endif

	//ENUM_DUMMY4WORD(SRAM_SD)
} SRAM_SD;

typedef enum {
	CHIP_NA51055 = 0x4821,
	CHIP_NA51084 = 0x5021,
    CHIP_NS02201 = 0xF221,
	CHIP_NS02301 = 0x7721,
	CHIP_NS02302 = 0x5A21,
	CHIP_NS02401 = 0xBB21,
	CHIP_NS02402 = 0x5B21,
} CHIP_ID;

extern void nvt_disable_sram_shutdown(SRAM_SD id);
extern void nvt_enable_sram_shutdown(SRAM_SD id);
extern UINT32 nvt_get_chip_id(void);


#endif /* _NT96680_NVT_SRAMCTL_H */
