#include <hdal.h>
#include <vendor_common.h>
#include "sys_mempool.h"

ULONG mempool_storage_sdio = 0;
ULONG mempool_filesys = 0;
ULONG mempool_fdtapp = 0;

void mempool_init(void)
{
	void *va;
	ULONG pa;
#if (_PACKAGE_SDCARD_)
	if (vendor_common_mem_alloc_fixed_pool("sdio", &pa, (void **)&va, POOL_SIZE_STORAGE_SDIO, DDR_ID0) == HD_OK) {
		mempool_storage_sdio = (ULONG)va;
	}
#endif
#if (_PACKAGE_FILESYS_)
	if (vendor_common_mem_alloc_fixed_pool("filesys", &pa, (void **)&va, POOL_SIZE_FILESYS, DDR_ID0) == HD_OK) {
		mempool_filesys = (ULONG)va;
	}
#endif
	if (vendor_common_mem_alloc_fixed_pool("fdapp", &pa, (void **)&va, POOL_SIZE_FDTAPP, DDR_ID0) == HD_OK) {
		mempool_fdtapp = (ULONG)va;
	}
}

void mempool_init2(void)
{
#if (_PACKAGE_SDCARD_)
	mempool_storage_sdio = (ULONG)malloc(POOL_SIZE_STORAGE_SDIO);
#endif
#if (_PACKAGE_FILESYS_)
	mempool_filesys = (ULONG)malloc(POOL_SIZE_FILESYS);
#endif
	mempool_fdtapp = (ULONG)malloc(POOL_SIZE_FDTAPP);
}