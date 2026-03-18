/*;   Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
 *;
 *;   @file       BinInfo.c
 *;   @ingroup
 *;
 *;   @brief      Binary file information
 *;               Binary file information
 *;
 *;   @version    V1.20.000
 *;   @author     Novatek
 *;   @date       2012/07/19
 */

#include <hd_type.h>
#include <comm/bin_info.h>
#include "prjcfg.h"

#define FW_VERSION 0x01000001 // Major: 31~24, Minor 23~8, Build and sign 7~0
#define FW_RELEASE_BUILD 0    // release:1, debug:0

#define __string(_x) #_x
#define __xstring(_x) __string(_x)

extern char _section_01_addr[];
extern char _section_10_size[];

BININFO bin_info = {
	//HEADINFO
	{
		(UINT32)_section_01_addr, //<- fw CODE entry (4)
		{
			_BOARD_FDT_ADDR_,     //<- fdt entry
			_BOARD_SHMEM_ADDR_,   //<- shmem entry
#if defined(_FW_TYPE_PARTIAL_) || defined(_FW_TYPE_PARTIAL_COMPRESS_)
			BOOT_FLAG_PARTLOAD_EN, //<- partial flag
#else
			0,
#endif
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
		}, ///<- reserved (4*19)
		_BIN_NAME_, //<- CHIP-NAME (8)
		"10000000", //<- version (8)
		__xstring(_BUILD_DATE_), //<- releasedate (8)
		(UINT32)_section_10_size, //<- Bin File Length (4)
		0xffffffff, //<- Check Sum or CRC (4)
		0,///<- Length check for CRC (4)
		0,///<- reserved (4)
		0,///<- Bin flag (4)
		0,///<- Binary Tag for CRC (4)
	},
#if defined(_NVT_DUAL_RTOS_ON_)
	{
		[BININFO_RESV_IDX_TAG] = MAKEFOURCC('D','R','T','S'), // for u-boot indicate to use decide boot which rtos bank,
		[BININFO_RESV_IDX_VER] = FW_VERSION,
		[BININFO_RESV_IDX_RELEASE_BUILD] = FW_RELEASE_BUILD,
		[BININFO_RESV_IDX_BOOT_BANK_FROM] = 0, // reserved an UINT32 field for u-boot notify rtos boot from which bank
		[BININFO_RESV_IDX_DATE_SEC] = _BUILD_DATE_SEC_,
	}, //RESERVED
#else
	{0}, //RESERVED
#endif
};


