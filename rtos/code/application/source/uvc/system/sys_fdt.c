#include <libfdt.h>
#include <strg_def.h>
#include "sys_storage_partition.h"
#include "sys_mempool.h"
#include "sys_fdt.h"
#include "prjcfg.h"
#include <kwrap/debug.h>


void *fdt_get_app(void)
{
	ER er;
	static BOOL inited = FALSE;
	if (inited == TRUE) {
		return (void *)mempool_fdtapp;
	}

	STORAGE_OBJ* pStrg = EMB_GETSTRGOBJ(STRG_OBJ_FW_APP);
	if (pStrg == NULL) {
		DBG_ERR("pStrg is NULL.\n");
		return NULL;
	}

	UINT32 blksize = 0;
	pStrg->GetParam(STRG_GET_BEST_ACCESS_SIZE, (UINT32)&blksize, 0);
	if (blksize == 0) {
		DBG_ERR("blksize is 0.\n");
		return NULL;
	}


	pStrg->Lock();
	pStrg->Open();
	er = pStrg->RdSectors((INT8*)mempool_fdtapp, 0, 1);
	if (er != 0) {
		pStrg->Unlock();
		return NULL;
	}

	if ((er = fdt_check_header((void*)mempool_fdtapp)) != 0) {
		DBG_ERR("invalid fdt-app header, addr=0x%08X er = %d \n", (unsigned int)mempool_fdtapp, er);
		pStrg->Close();
		pStrg->Unlock();
		return NULL;
	}

	int fdtapp_size = fdt_totalsize((void*)mempool_fdtapp);
	if (fdtapp_size > POOL_SIZE_FDTAPP) {
		DBG_ERR("POOL_SIZE_FDTAPP is too small, require:0x%08X\n", fdtapp_size);
		return NULL;
	}

	if (fdtapp_size > (int)blksize) {
		int remain_blocks = ALIGN_CEIL(fdtapp_size - blksize, blksize) / blksize;
		er = pStrg->RdSectors((INT8*)(mempool_fdtapp + blksize), 1, remain_blocks);
		if (er != 0) {
			pStrg->Close();
			pStrg->Unlock();
			return NULL;
		}
	}

	pStrg->Close();
	pStrg->Unlock();

	if ((er = fdt_check_full((void*)mempool_fdtapp, fdtapp_size)) != 0) {
		DBG_ERR("invalid fdt_check_full, addr=0x%08X er = %d \n", (unsigned int)mempool_fdtapp, er);
		pStrg->Close();
		return NULL;
	}

	inited = TRUE;
	return (void *)mempool_fdtapp;
}

void *fdt_get_sensor(void)
{
	int er;
	extern unsigned char _fdt_sensor[];

	if ((er = fdt_check_header((void*)_fdt_sensor)) != 0) {
		DBG_ERR("invalid fdt-sensor header, addr=0x%08X er = %d \n", (unsigned int)_fdt_sensor, er);
		return NULL;
	}

	return (void *)_fdt_sensor;
}