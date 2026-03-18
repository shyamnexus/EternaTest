/*
    System Storage Callback

    System Callback for Storage Module.

    @file       sys_strg_exe.c
    @ingroup    mIPRJSYS

    @note       PStore sysem functions.


    Copyright   Novatek Microelectronics Corp. 2010.  All rights reserved.
*/
	
#include <kwrap/debug.h>
#include <hdal.h>
#include <vendor_common.h>
#include "sys_mempool.h"
#include "PStore.h"
#include "sys_storage_partition.h"
#include "prjcfg.h"


#if defined(_EMBMEM_SPI_NOR_)
#define MAX_BLK_PER_SEC         128
#if !defined(PSTORE_MAX_SECTION_NUM)
#ifdef _MODEL_UVCCAM5_517L_DUAL_EVB_
#define PSTORE_MAX_SECTION_NUM  8
#else
#define PSTORE_MAX_SECTION_NUM  4
#endif
#endif
#else
#define MAX_BLK_PER_SEC         512
#if !defined(PSTORE_MAX_SECTION_NUM)
#define PSTORE_MAX_SECTION_NUM  4
#endif
#endif


///////////////////////////////////////////////////////////////////////////////
//
//  PSTORE
//
///////////////////////////////////////////////////////////////////////////////
void System_PS_Format(void)
{
	PSFMT gFmtStruct = {MAX_BLK_PER_SEC, PSTORE_MAX_SECTION_NUM};
	PStore_Format(&gFmtStruct);
}
void System_OnStrgInit_PS(void)
{
	PStore_InstallID();
#ifdef _MODEL_UVCCAM5_517L_DUAL_EVB_
	DisableNORWriteProtect();
#endif
	//PHASE-2 : Init & Open Lib or LibExt
	// Open PStore
	PSFMT gFmtStruct = {MAX_BLK_PER_SEC, PSTORE_MAX_SECTION_NUM};
	PSTORE_INIT_PARAM gPStoreParam;
	UINT32 result = 0;
	UINT8 *pBuf;
#if defined(_CPU2_LINUX_) && defined(_EMBMEM_EMMC_)
	PStore_Init(PS_TYPE_FILESYS, PST_FS_DRIVE[0]);
	UINT32 paramNum;
	UINT32 paramArray[MAX_MESSAGE_PARAM_NUM];
	do {
		UserWaitEvent(NVTEVT_STRG_ATTACH, &paramNum, paramArray);
	} while(paramArray[0] != PST_DEV_ID); //PStore will mount first before dev[0],dev[1]
#else
	PStore_Init(PS_TYPE_EMBEDED, 0);
#endif
	pBuf = (UINT8 *)mempool_pstore;
	if(!pBuf){
		DBG_ERR("fail to allocate pstore buffer of %d bytes\n", POOL_SIZE_PS_BUFFER);
		return;
	}

	gPStoreParam.pBuf = pBuf;
	gPStoreParam.uiBufSize = POOL_SIZE_PS_BUFFER;
	DBG_DUMP("PStore uiBufSize=%d\r\n", gPStoreParam.uiBufSize);
	result = PStore_Open(EMB_GETSTRGOBJ(STRG_OBJ_PSTORE1), &gPStoreParam);

	if (result != E_PS_OK) {
		DBG_ERR("PStore Open fail %d format \r\n", result);
		PStore_Format(&gFmtStruct);
	DBG_DUMP("PStore_Format done\r\n");
	}
#ifdef _MODEL_UVCCAM5_517L_DUAL_EVB_
	if (PStore_GetInfo(PS_INFO_MAX_SEC) != PSTORE_MAX_SECTION_NUM){
		DBG_ERR("PStore SECTION update %d to %d, format\r\n", PStore_GetInfo(PS_INFO_MAX_SEC), PSTORE_MAX_SECTION_NUM);
		PStore_Format(&gFmtStruct);
	}
#endif

#if defined(_CPU2_LINUX_)
	{
		PSTOREIPC_OPEN   PsopenObj = {0};
		// open pstore ipc
		PstoreIpc_Open(&PsopenObj);
	}
#endif
//#if (POWERON_FAST_BOOT == ENABLE)
//	INIT_SETFLAG(FLGINIT_MOUNTPS);
//#endif
#ifdef _MODEL_UVCCAM5_517L_DUAL_EVB_
	EnableNORWriteProtect();
#endif

}
void System_OnStrgExit_PS(void)
{
#if defined(_CPU2_LINUX_)
	PstoreIpc_Close();
#endif
	PStore_Close();
}


typedef enum {
	NAND_SPI_NOR_STS_RDSR_1 = 0,
	NAND_SPI_NOR_STS_RDSR_2,

	NAND_SPI_NOR_STS_WRSR_1,
	NAND_SPI_NOR_STS_WRSR_2,

	NAND_SPI_NOR_STS_RDCR,
	NAND_SPI_NOR_STS_WRSR_3,

	ENUM_DUMMY4WORD(NAND_SPI_NOR_STATUS_SET)
} NAND_SPI_NOR_STATUS_SET;

///!! to use this function spi must opened and locked
#if 0//defined(_EMBMEM_SPI_NOR_) && !defined(_RESCUE_UITRON_NONE_)
static int spi_nor_write_protect(int en)
{
	UINT8 status = 0;
	PNAND_SPI_OBJ nand_obj = nand_ctrl_spi_nor_getObject();
	if (nand_obj){
		DBG_DUMP("Remove NOR flash write protect...\r\n");
		nand_obj->getStatus(NAND_SPI_NOR_STS_RDSR_1, &status);
		DBG_DUMP("current NOR flash status = 0x%02x\r\n", status);
		if (en) {
			status |= ~0x3C;
		} else {
			status &= ~0x3C;
		}
		nand_obj->setStatus(NAND_SPI_NOR_STS_WRSR_1, status);
		DBG_DUMP("Checking write protect...\r\n");
		nand_obj->getStatus(NAND_SPI_NOR_STS_RDSR_1, &status);
		DBG_DUMP("current NOR flash status = 0x%02x\r\n", status);
	}
	return 0;
}
#endif
