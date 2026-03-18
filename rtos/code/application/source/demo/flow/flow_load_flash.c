#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <kwrap/perf.h>
#include <kwrap/debug.h>
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h>
#include <hdal.h>
#include <plat/strg_def.h>
#include "prjcfg.h"
#include "sys_storage_partition.h"
#include "flow_load_flash.h"

// sample to read whole 16MB spi-flash
#define CFG_READ_SIZE ALIGN_CEIL(0x1000000, _EMBMEM_BLK_SIZE_)

static  HD_COMMON_MEM_VB_BLK m_mem_blk;

static unsigned char *prepare_memory(void)
{
	ULONG pa; // physical address
	ULONG va; // logicial address

	//--- Get memory ---
	m_mem_blk = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, CFG_READ_SIZE, DDR_ID0); // Get block from mem pool
	if (m_mem_blk == HD_COMMON_MEM_VB_INVALID_BLK) {
		printf("get block fail (0x%x)\r\n", (int)m_mem_blk);
		return NULL;
	}

	pa = hd_common_mem_blk2pa(m_mem_blk); // Get physical addr
	if (pa == 0) {
		printf("blk2pa fail, blk = 0x%x\r\n", (int)m_mem_blk);
		goto rel_blk;
	}

	va = (ULONG)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa, CFG_READ_SIZE);
	if (va == 0) {
		printf("Error: mmap fail !! blk = 0x%x\r\n", (int)m_mem_blk);
		goto rel_blk;
	}

	return (unsigned char *)va;

rel_blk:
	//--- Release memory ---
	if (hd_common_mem_release_block(m_mem_blk) != HD_OK) {
		printf("failed to hd_common_mem_release_block.\n");
	}
	return NULL;
}

static int free_memory(void)
{
	if (hd_common_mem_release_block(m_mem_blk) != HD_OK) {
		printf("failed to hd_common_mem_release_block.\n");
		return -1;
	}
	return 0;
}

int flow_load_flash(void)
{
	vos_perf_list_mark("lflash", __LINE__, 0);

	// create temp buffer
	unsigned char *p = prepare_memory();
	if (p == NULL) {
		return -1;
	}

	STORAGE_OBJ *pStrg = EMB_GETSTRGOBJ(STRG_OBJ_FW_RSV7);
	if (pStrg == NULL) {
		DBG_ERR("pStrg is NULL.\n");
		free_memory();
		return -1;
	}

	UINT32 blksize = 0;
	pStrg->GetParam(STRG_GET_BEST_ACCESS_SIZE, (ULONG)&blksize, 0);
	if (blksize == 0) {
		DBG_ERR("blksize is 0.\n");
		free_memory();
		return -1;
	}
	pStrg->SetParam(STRG_SET_PARTITION_SECTORS, 0, (ULONG)(CFG_READ_SIZE / blksize));

	pStrg->Lock();
	pStrg->Open();
	pStrg->RdSectors((INT8 *)p, 0, (ULONG)(CFG_READ_SIZE / blksize));
	pStrg->Close();
	pStrg->Unlock();

	free_memory();

	vos_perf_list_mark("lflash", __LINE__, 1);

	return 0;
}
