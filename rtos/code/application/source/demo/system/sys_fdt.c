#include <libfdt.h>
#include <compiler.h>
#include <strg_def.h>
#include <fdtfast.h>
#include <rtosfdt.h>
#include "sys_storage_partition.h"
#include "sys_mempool.h"
#include "sys_fdt.h"
#include "prjcfg.h"
#include <kwrap/debug.h>

static BOOL inited = FALSE;

/**
 * @brief only for debug on T.bin
 */
int fdt_set_app(void *p_fdt_app)
{
	int er;

	if (inited == TRUE) {
		return 0;
	}

	if ((er = fdt_check_header((void*)p_fdt_app)) != 0) {
		DBG_ERR("invalid fdt-app header, addr=0x%08lX er = %d \n", (unsigned long)p_fdt_app, er);
		return -1;
	}

	int fdtapp_size = fdt_totalsize((void*)p_fdt_app);
	if (fdtapp_size > POOL_SIZE_FDTAPP) {
		DBG_ERR("POOL_SIZE_FDTAPP is too small, require:0x%08X\n", fdtapp_size);
		return -1;
	}

	memcpy((void *)mempool_fdtapp, p_fdt_app, fdtapp_size);
	inited = TRUE;
	return 0;
}


void *fdt_get_app(void)
{
	ER er;
	if (inited == TRUE) {
		return (void *)mempool_fdtapp;
	}

	STORAGE_OBJ* pStrg = EMB_GETSTRGOBJ(STRG_OBJ_FW_APP);
	if (pStrg == NULL) {
		DBG_ERR("pStrg is NULL.\n");
		return NULL;
	}

	ULONG blksize = 0;
	pStrg->GetParam(STRG_GET_BEST_ACCESS_SIZE, (ULONG)&blksize, 0);
	if (blksize == 0) {
		DBG_ERR("blksize is 0.\n");
		return NULL;
	}

	pStrg->Open();
	pStrg->Lock();
	er = pStrg->RdSectors((INT8*)mempool_fdtapp, 0, 1);
	pStrg->Unlock();
	if (er != 0) {
		return NULL;
	}

	if ((er = fdt_check_header((void*)mempool_fdtapp)) != 0) {
		DBG_ERR("invalid fdt-app header, addr=0x%08X er = %d \n", (unsigned int)mempool_fdtapp, er);
		pStrg->Close();
		return NULL;
	}

	int fdtapp_size = fdt_totalsize((void*)mempool_fdtapp);
	if (fdtapp_size > POOL_SIZE_FDTAPP) {
		DBG_ERR("POOL_SIZE_FDTAPP is too small, require:0x%08X\n", fdtapp_size);
		return NULL;
	}

	if (fdtapp_size > (int)blksize) {
		int remain_blocks = ALIGN_CEIL(fdtapp_size - blksize, blksize) / blksize;
		pStrg->Lock();
		er = pStrg->RdSectors((INT8*)(mempool_fdtapp + blksize), 1, remain_blocks);
		pStrg->Unlock();
		if (er != 0) {
			pStrg->Close();
			return NULL;
		}
	}

	pStrg->Close();

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
		DBG_ERR("invalid fdt-sensor header, addr=0x%08lX er = %d \n", (unsigned long)_fdt_sensor, er);
		return NULL;
	}

	return (void *)_fdt_sensor;
}

SHMINFO *fdt_get_shminfo(void)
{
	static SHMINFO *p_shm = NULL;

	if (p_shm) {
		return p_shm;
	}

	unsigned char *p_fdt = (unsigned char *)fdt_get_base();

	if (p_fdt == NULL) {
	    DBG_ERR("p_fdt is NULL.\n");
	    return NULL;
	}

	int nodeoffset = fdt_path_offset(p_fdt, SHMEM_PATH);
	if (nodeoffset < 0) {
	    DBG_ERR("failed to offset for  %s = %d \n", SHMEM_PATH, nodeoffset);
	} else {
	    DBG_DUMP("offset for  %s = %d \n", SHMEM_PATH, nodeoffset);
	}

	int len;
	const void *nodep = fdt_getprop(p_fdt, nodeoffset, "reg", &len);
	if (len == 0 || nodep == NULL) {
	    DBG_ERR("failed to access reg.\n");
	    return NULL;
	}

	unsigned int *p_data = (unsigned int *)nodep;
	p_shm = (SHMINFO *)(uintptr_t)be32_to_cpu(p_data[0]);

	return p_shm;
}

int is_fastboot(void)
{
	static int b_fastboot = -1;

	//speed up accessing fdt
	if (b_fastboot != -1) {
		return b_fastboot;
	}

#if defined(FASTBOOT20)
	unsigned char *p_fdt = (unsigned char *)fdtfast_get_base();
#else
	unsigned char *p_fdt = (unsigned char *)fdt_get_base();
#endif

	if (p_fdt == NULL) {
		DBG_ERR("fdtfast is NULL. rtos run in non-fastboot.\n");
		b_fastboot = 0;
		return 0;
	}

	int len;
	int nodeoffset;
	const void *nodep;  /* property node pointer */

	// get linux space
	nodeoffset = fdt_path_offset(p_fdt, "/fastboot");
	if (nodeoffset < 0) {
		DBG_ERR("failed to offset for  %s = %d \n", "/fastboot", nodeoffset);
		return 0;
	}

	nodep = fdt_getprop(p_fdt, nodeoffset, "enable", &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("failed to access enable.\n");
		return 0;
	}

	unsigned int *p_data = (unsigned int *)nodep;
	b_fastboot = be32_to_cpu(p_data[0]);

	return b_fastboot;
}
